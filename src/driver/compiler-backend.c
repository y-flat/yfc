#include "compiler-backend.h"
#include "driver/os.h"
#include "util/list.h"

#include <string.h>

#include <driver/c-compiler.h>
#include <gen/gen.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

static void dump_command(const char * const cmd[]) {
    if (*cmd) {
        fputc('"', YF_OUTPUT_STREAM);
        fputs(*cmd, YF_OUTPUT_STREAM);
        fputc('"', YF_OUTPUT_STREAM);
        for (++cmd; *cmd; ++cmd) {
            fputc(' ', YF_OUTPUT_STREAM);
            fputc('"', YF_OUTPUT_STREAM);
            fputs(*cmd, YF_OUTPUT_STREAM);
            fputc('"', YF_OUTPUT_STREAM);
        }
    }
    fputc('\n', YF_OUTPUT_STREAM);
}

int create_output_file_name(
    struct yf_file_compilation_data * data, struct yf_args * args
) {

    const char * namebuf_copyloc;

    data->output_file = yf_malloc(sizeof (char) * 256);
    if (!data->output_file)
        return 1;
    memset(data->output_file, 0, sizeof (char) * 256);

    /* Normal: foo/bar/baz.yf -> foo/bar/baz.c */
    /* with --project: src/foo/bar/baz.yf -> bin/c/foo/bar/baz.c */
    if (args->project) {
        strcpy(data->output_file, "bin/c/");
        namebuf_copyloc = data->file_name + strlen("src/");
        strcat(data->output_file, namebuf_copyloc);
        /* Change .yf to .c */
        /* FIRST, check for .yf ending */
        if (strlen(data->output_file) > 3 &&
            strcmp(data->output_file + strlen(data->output_file) - 3, ".yf") == 0) {
            /* Simply tack on .c */
            /* But no .yf is bad */
            strcat(data->output_file, ".c");
            YF_PRINT_WARNING(
                "file %s does not end with .yf", data->output_file
            );
        } else {
            strcpy(data->output_file + strlen(namebuf_copyloc) - 2, "c");
        }
    } else {
        /* Replace .yf with .c */
        /* TODO - reduce code copying from above */
        namebuf_copyloc = data->file_name;
        strcat(data->output_file, namebuf_copyloc);
        if (strlen(data->output_file) > 3 &&
            strcmp(data->output_file + strlen(data->output_file) - 3, ".yf")) {
            /* Simply tack on .c */
            /* But no .yf is bad */
            YF_PRINT_WARNING(
                "file %s does not end with .yf", data->output_file
            );
            strcat(data->output_file, ".c");
        } else {
            strcpy(data->output_file + strlen(namebuf_copyloc) - 2, "c");
        }
    }

    return 0;

}

/**
 * Generate C code
 */
int yf_gen_c(struct yf_file_compilation_data * fdata) {
    return yfg_gen(fdata);
}

/**
 * Run the C compiler and link the binary.
 */
int yf_run_c_backend(
    struct yf_project_compilation_data * data, struct yf_args * args
) {

    int i;
    struct yf_file_compilation_data * file;

    switch (yf_determine_c_compiler(args)) {
        case YF_COMPILER_OK:
            break;
        case YF_SPECIFIED_COMPILER_NOT_FOUND:
            YF_PRINT_ERROR("specified C compiler not found");
            return 1;
        case YF_NO_COMPILER_FOUND:
            YF_PRINT_ERROR("could not determine C compiler");
            return 1;
        case YF_COMPILER_NO_CLASS:
            YF_PRINT_ERROR("cannot set compiler type without providing compiler path");
            return 1;
    }

    if (args->compiler_class != YF_COMPILER_GCC) {
        YF_PRINT_ERROR("only gcc-like compilers are supported");
        return 1;
    }

    static const file_open_descriptor descs[] = {
        { 0, YF_OS_FILE_DEVNULL },
        { 1, YF_OS_FILE_DEVNULL },
        { 2, YF_OS_FILE_DEVNULL },
        { -1, -1 },
    };

    /* Where gcc -c foo.c -o foo.o is stored */
    const char * compile_cmd[] = { args->selected_compiler, "-c", NULL, "-o", NULL, NULL };
    /* Where gcc foo1.o foo2.o -o foo is stored */
    const char ** link_cmd;
    struct yf_list link_objs;
    yf_list_init(&link_objs);

    size_t num_objs = 0;

    for (i = 0; i < YFH_BUCKETS; ++i) {
        file = data->files->buckets[i].value;
        if (!file) continue;

        /* Rewite file name foo.c to have foo.o */
        size_t fname_len = strlen(file->output_file);
        char * object_file = yf_malloc(fname_len + 1);
        memcpy(object_file, file->output_file, fname_len + 1);
        object_file[fname_len - 1] = 'o';

        /* Run compiler */
        /* <file-name> */ compile_cmd[2] = file->output_file;
        /* -o <output-name> */ compile_cmd[4] = object_file;

        if (args->dump_commands) {
            fputs("Compile command: ", YF_OUTPUT_STREAM);
            dump_command(compile_cmd);
        }
        if (proc_exec(compile_cmd, descs, 0) != 0) {
            YF_PRINT_ERROR("Compilation of generated C of file %s failed", file->file_name);
            return 2;
        }

        /*
        Also append name to linker command.
        File name will be automatically freed on list destroy.
        */
        yf_list_add(&link_objs, object_file);
        ++num_objs;
    }

    /* <compiler> <objects...> -o <executable> */
    link_cmd = yf_malloc((4 + num_objs) * sizeof(const char *));

    link_cmd[0] = args->selected_compiler;
    const char ** it = link_cmd + 1;
    size_t obj_it;
    for (obj_it = 0; obj_it < num_objs; ++obj_it) {
        yf_list_get(&link_objs, (void **)it);
        yf_list_next(&link_objs);
        ++it;
    }

    /* Finally, add -o output_file */
    *it++ = "-o";
    if (args->project) {
        *it++ = data->project_name;
    } else {
        *it++ = "a.out";
    }

    /* Finish argument list */
    *it = NULL;

    if (args->dump_commands) {
        fputs("Link command: ", YF_OUTPUT_STREAM);
        dump_command(link_cmd);
    }
    if (proc_exec(link_cmd, descs, 0) != 0) {
        YF_PRINT_ERROR("Linking of project failed");
        goto err2;
    }

    int ret = 0;
    goto out;

err2:
    ret = 2;
    yf_free(link_cmd);
    yf_list_destroy(&link_objs, true);

out:
    return ret;

}

/**
 * Generate C code, run the C compiler, and link the resulting binary.
 */
int yf_run_backend(
    struct yf_project_compilation_data * data, struct yf_args * args
) {
    
    int i, err = 0;
    struct yf_file_compilation_data * file;

    for (i = 0; i < YFH_BUCKETS; ++i) {
        file = data->files->buckets[i].value;
        if (!file) continue;
        if (file->error) {
            err = 1;
            continue;
        }
        create_output_file_name(file, args);
        yf_gen_c(file);
    }

    if (!err)
        err = yf_run_c_backend(data, args);

    return err;

}
