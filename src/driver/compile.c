#include "compile.h"

#include <stdio.h> /* fopen, etc. */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <sys/stat.h>
#include <sys/time.h> /* struct timeval */
#include <unistd.h> /* getcwd */

#include <api/compilation-data.h>
#include <api/cst-dump.h>
#include <api/lexer-input.h>
#include <driver/compiler-backend.h>
#include <driver/find-files.h>
#include <parser/parser.h>
#include <semantics/symtab.h>
#include <semantics/validate/validate.h>
#include <util/allocator.h>
#include <util/list.h>
#include <util/hashmap.h>
#include <util/yfc-out.h>

/* Forward decls for whole file */
static int yf_compile_project(struct yf_args *, struct yf_compilation_data *);
static int yf_compile_files(struct yf_args *, struct yf_compilation_data *);
static int yfc_run_frontend_build_symtable(
    struct yf_compilation_data *,
    struct yf_compile_analyse_job *
);
static int yfc_validate_compile(
    struct yf_compilation_data *,
    struct yf_compile_compile_job *
);
static int yf_find_project_files(struct yf_project_compilation_data *);
static int dump_tokens(struct yf_lexer *);
static int yf_build_symtab(struct yf_compile_analyse_job *);
static int yf_validate_ast(
    struct yf_compilation_data * pdata,
    struct yf_compile_analyse_job * adata
);
static int yf_do_cst_dump(struct yf_parse_node * tree);
static int yf_cleanup(struct yf_compilation_data *);

static inline const char * str_or_null(const char * s) {
    return s ? s : "(null)";
}

static void yf_dump_compile_job(struct yf_compile_analyse_job * job, const char * label) {
    fputs(label, YF_OUTPUT_STREAM);
    switch (job->stage) {
        case YF_COMPILE_LEXONLY:
            fputs(" (DUMP_TOKENS) ", YF_OUTPUT_STREAM);
            break;
        case YF_COMPILE_PARSEONLY:
            fputs(" (DUMP_CST) ", YF_OUTPUT_STREAM);
            break;
        case YF_COMPILE_ANALYSEONLY:
            fputs(" (ANALYSE) ", YF_OUTPUT_STREAM);
            break;
        case YF_COMPILE_CODEGENONLY:
            fputs(" (OUTPUT) ", YF_OUTPUT_STREAM);
            break;
        case YF_COMPILE_FULL:
            fputs(" (COMPILE) ", YF_OUTPUT_STREAM);
            break;
    }
    if (!job->unit_info->parse_anew) {
        fputs("(cached) ", YF_OUTPUT_STREAM);
    }
    fprintf(YF_OUTPUT_STREAM, "file=%s prefix=%s syms=%s out=%s\n",
        job->unit_info->file_name,
        str_or_null(job->unit_info->file_prefix),
        str_or_null(job->unit_info->sym_file),
        str_or_null(job->unit_info->output_file)
    );
}

/**
 * Create a list of jobs to be done to compile with the given arguments.
 */
static int yf_create_compilation_data(struct yf_args * args, struct yf_compilation_data * compilation) {
    if (args->project) {
        return yf_compile_project(args, compilation);
    } else {
        return yf_compile_files(args, compilation);
    }
}

/**
 * This is it. This is the actual compile function for a set of arguments. It
 * just defers compilation to one of two functions, depending on whether
 * --project is enabled or not.
 */
int yf_run_compiler(struct yf_args * args) {

    struct yf_compilation_data compilation;
    struct yf_compilation_job * job;
    int res = 0;

    res = yf_create_compilation_data(args, &compilation);

    if (res)
        return res;

    /* Execute jobs */
    YF_LIST_FOREACH(compilation.jobs, job) {
        switch (job->type) {
            case YF_COMPILATION_ANALYSE:
                if (args->dump_commands) {
                    yf_dump_compile_job((struct yf_compile_analyse_job *)job, "ANALYSE");
                }
                if (!args->simulate_run) {
                    res = yfc_run_frontend_build_symtable(&compilation, (struct yf_compile_analyse_job *)job);
                }
                break;

            case YF_COMPILATION_COMPILE:
                if (args->dump_commands) {
                    yf_dump_compile_job(((struct yf_compile_compile_job *)job)->unit, "COMPILE");
                }
                if (!args->simulate_run) {
                    res = yfc_validate_compile(&compilation, (struct yf_compile_compile_job *)job);
                }
                break;

            case YF_COMPILATION_EXEC:
                if (args->dump_commands) {
                    yf_print_command((struct yf_compile_exec_job *)job);
                }
                if (!args->simulate_run) {
                    res = yf_exec_command((struct yf_compile_exec_job *)job);
                }
                break;

        }

        if (res)
            break;
    }

    yf_cleanup(&compilation);
    yf_free((void *)args->selected_compiler);

    return res;

}

static int yf_create_compiler_jobs(
    struct yf_compilation_data * compilation,
    struct yf_project_compilation_data * data,
    struct yf_args * args
) {

    struct yf_compilation_unit_info * fdata;
    struct yf_compile_analyse_job * ujob;
    struct yf_compile_compile_job * cjob;
    bool has_compiled_files = false;

    yf_backend_find_compiler(args);

    struct yf_list link_objs;
    yf_list_init(&link_objs);

    /* Fill project info */
    compilation->project_name = data->project_name;
    yf_list_init(&compilation->jobs);
    yfh_init(&compilation->symtables);
    yf_list_init(&compilation->garbage);

    struct yfh_cursor cursor;
    for (yfh_cursor_init(&cursor, &data->files); !yfh_cursor_next(&cursor); ) {
        yfh_cursor_get(&cursor, NULL, (void **)&fdata);

        ujob = malloc(sizeof(struct yf_compile_analyse_job));
        memset(ujob, 0, sizeof(struct yf_compile_analyse_job));

        ujob->job.type = YF_COMPILATION_ANALYSE;
        ujob->unit_info = fdata;

        ujob->stage =
            args->tdump          ? YF_COMPILE_LEXONLY     :
            args->cstdump        ? YF_COMPILE_PARSEONLY   :
            args->just_semantics ? YF_COMPILE_ANALYSEONLY :
           !args->run_c_comp     ? YF_COMPILE_CODEGENONLY :
                                   YF_COMPILE_FULL;

        yfh_cursor_set(&cursor, ujob); // Set the job for further stages
        yf_list_add(&compilation->jobs, ujob);
    }

    for (yfh_cursor_init(&cursor, &data->files); !yfh_cursor_next(&cursor); ) {
        yfh_cursor_get(&cursor, NULL, (void **)&ujob);
        if (ujob->stage < YF_COMPILE_ANALYSEONLY)
            continue;

        cjob = malloc(sizeof(struct yf_compile_compile_job));
        cjob->job.type = YF_COMPILATION_COMPILE;
        cjob->unit = ujob;
        yf_list_add(&compilation->jobs, cjob);

        if (ujob->stage >= YF_COMPILE_CODEGENONLY) {
            char * object_file = yf_backend_add_compile_job(compilation, args, ujob->unit_info);
            yf_list_add(&link_objs, object_file);
            has_compiled_files = true;
        }
    }

    if (has_compiled_files && ujob->stage >= YF_COMPILE_FULL) {
        yf_backend_add_link_job(compilation, args, &link_objs);
    }

    yf_list_merge(&compilation->garbage, &link_objs);
    yfh_destroy(&data->files, NULL);

    return 0;

}

#if 0
static int yf_run_compiler_on_data(
    struct yf_project_compilation_data * data,
    struct yf_args * args
) {

/* Local defines for profiling */
#define BEGIN_PROFILE() \
    gettimeofday(&step_begin, NULL)

#define END_PROFILE(step_name) do { \
    gettimeofday(&step_end, NULL); \
    if (args->profile) { \
        time_for_step = (step_end.tv_sec - step_begin.tv_sec) * 1000000 + \
            (step_end.tv_usec - step_begin.tv_usec); \
        YF_PRINT_DEFAULT( \
            "Time for %s: %f seconds", \
            step_name, \
            ( (double) time_for_step ) / 1000000.0 \
        ); \
    } \
} while (0)

    int i, err = 0;
    struct yf_file_compilation_data * fdata;

    /* For profiling purposes only */
    double time_for_step, time_total;
    struct timeval total_begin, step_begin, step_end, total_end;

    /* Parse the frontend for all and create symtabs */
    BEGIN_PROFILE();
    total_begin = step_begin;
    for (i = 0; i < YFH_BUCKETS; ++i) {
        fdata = data->files->buckets[i].value;
        if (!fdata) continue;
        fdata->error = 0; /* Starting off clean */
        if (yf_run_frontend(fdata, args)) {
            fdata->error = 1;
        }
        if (args->cstdump || args->tdump) {
            return fdata->error;
        }
    }
    END_PROFILE("parsing");

    BEGIN_PROFILE();
    for (i = 0; i < YFH_BUCKETS; ++i) {
        fdata = data->files->buckets[i].value;
        if (!fdata) continue;
        if (!fdata->error) {
            if (yf_build_symtab(fdata)) {
                fdata->error = 1;
                err = 1;
            } else {
                fdata->error = 0;
            }
        }
    }
    END_PROFILE("building symtabs");

    /* TODO - set flags so we can efficiently avoid compiling the bad ones */

    /* Now validate everything. */
    BEGIN_PROFILE();
    for (i = 0; i < YFH_BUCKETS; ++i) {
        fdata = data->files->buckets[i].value;
        if (!fdata) continue;
        if (!fdata->error
            && yf_validate_ast(
                data, fdata, args
            )
        ) {
            fdata->error = 1;
            err = 1;
        }
    }
    END_PROFILE("validating code");

    /* Finally, generate code. */
    if (!args->just_semantics) {
        BEGIN_PROFILE();
        err = yf_run_backend(data, args);
        gettimeofday(&step_end, NULL);
        END_PROFILE("generating code");
    }
    total_end = step_end;
    if (args->profile) {
        time_total = (total_end.tv_sec - total_begin.tv_sec) * 1000000 +
            (total_end.tv_usec - total_begin.tv_usec);
        YF_PRINT_DEFAULT(
            "Total time: %f seconds",
            ( (double) time_total ) / 1000000.0
        );
    }

#undef BEGIN_PROFILE
#undef END_PROFILE

    return err;

}
#endif

static int yf_compile_project(struct yf_args * args, struct yf_compilation_data * compilation) {

    struct yf_project_compilation_data data;
    struct yf_compilation_unit_info * fdata;

    /**
     * Project name is the current directory, PLUS its own name. So the project
     * in ~/yflat/project/ compiles to ~/yflat/project/project .
     * TODO -- give user control over the name
     * TODO -- make less OS-specific
     */
    data.project_name = yf_malloc(256);
    if (!getcwd(data.project_name, 256)) {
        YF_PRINT_ERROR("Couldn't start compilation: path name is too long");
        return 1;
    }
    /* TODO -- consider that getcwd might return a name ending with a slash? */
    char * copyloc = strrchr(data.project_name, '/');
    if (copyloc == NULL) {
        YF_PRINT_ERROR("The current working directory is invalid");
        return 1;
    }
    /* TODO -- overflow check */
    int end = strlen(data.project_name);
    memmove(data.project_name + end, copyloc, strlen(copyloc) + 1);

    yfh_init(&data.files);
    if (yf_find_project_files(&data)) {
        return 1;
    }

    if (args->dump_projfiles) {
        YF_PRINT_DEFAULT("Project files: (green = needs to be recompiled):");    
        struct yfh_cursor cursor;
        for (yfh_cursor_init(&cursor, &data.files); !yfh_cursor_next(&cursor); ) {
            yfh_cursor_get(&cursor, NULL, (void **)&fdata);
            if (fdata->parse_anew) {
                YF_PRINT_WITH_COLOR(
                    YF_CODE_YELLOW,
                    "%s %s\n",
                    fdata->file_name,
                    fdata->file_prefix
                );
            } else {
                YF_PRINT_WITH_COLOR(
                    YF_CODE_GREEN,
                    "%s %s\n",
                    fdata->file_name,
                    fdata->file_prefix
                );
            }
        }
        return 0;
    }

    return yf_create_compiler_jobs(compilation, &data, args);

}

static int yf_compile_files(struct yf_args * args, struct yf_compilation_data * compilation) {
    
    struct yf_project_compilation_data data;
    struct yf_compilation_unit_info * fdata;
    int i;

    /* No project name */
    data.project_name = NULL;

    yfh_init(&data.files);

    for (i = 0; i < args->num_files; ++i) {
        fdata = malloc(sizeof(struct yf_compilation_unit_info));
        memset(fdata, 0, sizeof (struct yf_compilation_unit_info));
        fdata->file_name = yf_strdup(args->files[i]);
        fdata->parse_anew = 1;
        /* TODO - more data */
        yfh_set(&data.files, fdata->file_name, fdata);
    }

    return yf_create_compiler_jobs(compilation, &data, args);

}

static int yfc_validate_compile(
    struct yf_compilation_data * pdata,
    struct yf_compile_compile_job * udata
) {

    struct yf_compile_analyse_job * adata = udata->unit;
    int retval;

    retval = yf_validate_ast(pdata, adata);
    if (retval)
        return retval;

    if (adata->stage >= YF_COMPILE_CODEGENONLY) {
        /* This check must go inside and return, or else we get errors about it
        once for every file. */
        if (yf_ensure_entry_point(pdata)) {
            return 1;
        }
        retval = yf_backend_generate_code(adata);
    }

    return retval;

}

/**
 * Run the lexing and parsing on one file and build a symtable of the file.
 */
static int yfc_run_frontend_build_symtable(
    struct yf_compilation_data * compilation,
    struct yf_compile_analyse_job * data
) {

    struct yf_lexer_input input;
    struct yf_lexer lexer;
    FILE * file_src;
    char * file_name;
    struct stat file_stat;
    struct yf_compilation_unit_info * file = data->unit_info;

    int retval;

    file_name = file->parse_anew ? file->file_name : file->sym_file;
    file_src = fopen(
        file_name,
        "r"
    );
    if (!file_src) {
        YF_PRINT_ERROR("Could not open file %s", file_name);
        return 1;
    }
    stat(file_name, &file_stat);
    if (!S_ISREG(file_stat.st_mode)) {
        YF_PRINT_ERROR("%s is not a regular file", file_name);
        return 1;
    }
    
    input = (struct yf_lexer_input) {
        .input = file_src,
        .getc = (int (*)(void*)) getc,
        .ungetc = (int (*)(int, void*)) ungetc,
        .input_name = file_name,
        .close = (int (*) (void*)) fclose,
        .identifier_prefix = file->file_prefix ? file->file_prefix : "" /** TODO: Let user chose file prefix */
    };

    yfl_init(&lexer, &input);

    if (data->stage == YF_COMPILE_LEXONLY) {
        return dump_tokens(&lexer);
    } else {
        if ( (retval = yf_parse(&lexer, &data->parse_tree)) ) {
            YF_PRINT_ERROR("Error parsing file %s", file->file_name);
            return retval;
        }
        if (data->stage == YF_COMPILE_PARSEONLY) {
            retval = yf_do_cst_dump(&data->parse_tree);
        } else {
            retval = yf_build_symtab(data);
            if (!retval && data->unit_info->file_prefix)
                yfh_set(&compilation->symtables, data->unit_info->file_prefix, &data->symtab);
        }
        return retval;
    }

}

/**
 * Stuff the project compilation data with all files that need to be compiled.
 * See yfd_find_projfiles for return code.
 */
static int yf_find_project_files(struct yf_project_compilation_data * data) {
    return yfd_find_projfiles(data);
}

/**
 * Dump all file tokens.
 */
static int dump_tokens(struct yf_lexer * lexer) {

    struct yf_token token;
    for (;;) {
        if (yfl_lex(lexer, &token)) {
            YF_PRINT_ERROR("Invalid token");
            return 1;
        }
        if (token.type == YFT_EOF) {
            break;
        }
        printf(
            "%20s, line: %3d, col: %3d, type: %20s\n",
            token.data,
            token.loc.line, token.loc.column, 
            yf_get_toktype(token.type)
        );
    }

    return 0; /* Indicates that nothing should else should be done (meaning no
    semantic analysis, etc. */

}

/**
 * Dump the CST.
 */
static int yf_do_cst_dump(struct yf_parse_node * tree) {
    yf_dump_cst(tree, stderr);
    return 0;
}

/**
 * Build a table of all externally visible symbols.
 */
static int yf_build_symtab(struct yf_compile_analyse_job * data) {
    return yfs_build_symtab(data);
}

/**
 * Create an AST from the concrete syntax tree, and validate it against the
 * combined symbol tables loaded so far.
 */
static int yf_validate_ast(
    struct yf_compilation_data * pdata,
    struct yf_compile_analyse_job * adata
) {

    return yfs_validate(adata, pdata);

}

/**
 * Destroy all objects and whatnot.
 */
static int yf_cleanup(struct yf_compilation_data * data) {

    struct yf_compilation_job * job;

    YF_LIST_FOREACH(data->jobs, job) {
        switch (job->type) {
            case YF_COMPILATION_ANALYSE: {
                struct yf_compile_analyse_job * adata = (struct yf_compile_analyse_job *)job;
                struct yf_compilation_unit_info * fdata = adata->unit_info;

                if (adata->types.table.buckets)
                    yfh_destroy(&adata->types.table, (void (*)(void *)) yfs_cleanup_type);

                if (adata->symtab.table.buckets)
                    yfh_destroy(&adata->symtab.table, (void (*)(void *)) yfs_cleanup_sym);

                // Will be EMPTY if unset
                yf_cleanup_cst(&adata->parse_tree);
                yf_cleanup_ast(&adata->ast_tree);

                yf_free(fdata->file_name);
                yf_free(fdata->file_prefix);
                yf_free(fdata->sym_file);
                yf_free(fdata->output_file);
                yf_free(fdata);
                break;
            }

            case YF_COMPILATION_EXEC:
                yf_free(((struct yf_compile_exec_job *)job)->command);
                break;

            case YF_COMPILATION_COMPILE:
                break;
        }
    }

    yf_free(data->project_name);
    yf_list_destroy(&data->jobs, true);
    yfh_destroy(&data->symtables, NULL);
    yf_list_destroy(&data->garbage, true);

    return 0;

}
