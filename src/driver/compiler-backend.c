#include "compiler-backend.h"

#include <string.h>

#include <driver/c-compiler.h>
#include <gen/gen.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

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
    
    /* Where gcc -c foo.c -o foo.o is stored */
    char * compile_buf = yf_malloc(256 * sizeof (char));
    /* Where gcc foo1.o foo2.o -o foo is stored */
    char * link_buf = yf_malloc(16384 * sizeof (char));
    if (!compile_buf || !link_buf)
        return 1;

    if (yf_determine_c_compiler(args) != YF_COMPILER_OK) {
        YF_PRINT_ERROR("could not determine C compiler");
        return 1;
    }

    strcpy(link_buf, YF_C_COMPILER);
    strcat(link_buf, " ");

    for (i = 0; i < YFH_BUCKETS; ++i) {
        file = data->files->buckets[i].value;
        if (!file) continue;
        /* Run compiler */
        sprintf(
            compile_buf,
            "%s -c %s -o %s",
            YF_C_COMPILER, file->output_file, file->output_file
        );
        /* Now, rewrite gcc -c x.c -o x.c  to have x.o */
        strcpy(compile_buf + strlen(compile_buf) - 2, ".o");
        //system(compile_buf);
        /* Also append name to linker command */
        strcat(link_buf, file->output_file);
        strcpy(link_buf + strlen(link_buf) - 2, ".o");
        strcat(link_buf, " ");
    }

    /* Finally, add -o output_file */
    if (args->project) {
        strcat(link_buf, "-o ");
        strcat(link_buf, data->project_name);
    } else {
        strcat(link_buf, "-o a.out");
    }

    //system(link_buf);

    free(compile_buf);
    free(link_buf);

    return 0;

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
        yf_run_c_backend(data, args);

    return 0;

}
