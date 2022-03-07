#include "compiler-backend.h"

#include <string.h>

#include <api/compilation-data.h>
#include <driver/c-compiler.h>
#include <driver/os.h>
#include <gen/gen.h>
#include <util/allocator.h>
#include <util/list.h>
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

void yf_print_command(
    struct yf_compile_exec_job * job
) {
    dump_command(job->command);
}

int yf_exec_command(
    struct yf_compile_exec_job * job
) {

    static const file_open_descriptor descs[] = {
        { 0, YF_OS_FILE_DEVNULL },
        { 1, YF_OS_FILE_DEVNULL },
        { 2, YF_OS_FILE_DEVNULL },
        { -1, -1 },
    };

    /*if (args->dump_commands) {
        fputs("Compile command: ", YF_OUTPUT_STREAM);
        dump_command(compile_cmd);
    }*/
    if (proc_exec(job->command, descs, 0) != 0) {
        YF_PRINT_ERROR("Compilation command failed");
        return 2;
    }

    return 0;
}

static int create_output_file_name(
    struct yf_compilation_unit_info * data, struct yf_args * args
) {

    const char * namebuf_copyloc;

    data->output_file = yf_malloc(256);
    if (!data->output_file)
        return 1;
    memset(data->output_file, 0, 256);

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
static int yf_gen_c(struct yf_compile_analyse_job * fdata) {
    return yfg_gen(fdata);
}

/**
 * Run the C compiler and link the binary.
 */
int yf_backend_find_compiler(
    struct yf_args * args
) {

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

    return 0;

}

char * yf_backend_add_compile_job(
    struct yf_compilation_data * compilation,
    struct yf_args * args,
    struct yf_compilation_unit_info * unit
) {

    create_output_file_name(unit, args);

    struct yf_compile_exec_job * cjob;

    /* Rewite file name foo.c to have foo.o */
    size_t fname_len = strlen(unit->output_file);
    char * object_file = yf_malloc(fname_len + 1);
    memcpy(object_file, unit->output_file, fname_len + 1);
    object_file[fname_len - 1] = 'o';

    cjob = malloc(sizeof(struct yf_compile_exec_job));
    cjob->job.type = YF_COMPILATION_EXEC;

    /* Where gcc -c foo.c -o foo.o is stored */
    cjob->command = malloc(sizeof(const char *) * 6);
    cjob->command[0] = args->selected_compiler;
    cjob->command[1] = "-c";
    cjob->command[2] = unit->output_file;
    cjob->command[3] = "-o";
    cjob->command[4] = object_file;
    cjob->command[5] = NULL;

    yf_list_add(&compilation->jobs, cjob);

    return object_file;

}

int yf_backend_add_link_job(
    struct yf_compilation_data * compilation,
    struct yf_args * args,
    struct yf_list * link_objs
) {
    /* Where gcc foo1.o foo2.o -o foo is stored */
    const char ** link_cmd;
    struct yf_compile_exec_job * ljob;

    size_t num_objs;
    const char * object_file;

    size_t obj_it;
    const char ** it;

    num_objs = 0;
    YF_LIST_FOREACH(*link_objs, object_file) {
        ++num_objs;
    }

    /* <compiler> <objects...> -o <executable> */
    link_cmd = yf_malloc((4 + num_objs) * sizeof(const char *));
    link_cmd[0] = args->selected_compiler;

    it = link_cmd + 1;
    yf_list_reset(link_objs);
    for (obj_it = 0; obj_it < num_objs; ++obj_it) {
        yf_list_get(link_objs, (void **)it);
        yf_list_next(link_objs);
        ++it;
    }

    /* Finally, add -o output_file */
    *it++ = "-o";
    if (compilation->project_name) {
        *it++ = compilation->project_name;
    } else {
        *it++ = "a.out";
    }

    /* Finish argument list */
    *it = NULL;

    ljob = yf_malloc(sizeof(struct yf_compile_exec_job));
    ljob->job.type = YF_COMPILATION_EXEC;
    ljob->command = link_cmd;

    yf_list_add(&compilation->jobs, ljob);

    return 0;

}

int yf_backend_generate_code(
    struct yf_compile_analyse_job * data
) {
    return yf_gen_c(data);
}
