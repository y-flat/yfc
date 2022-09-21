# The driver job system

## Intro
The compilation driver is responsible for processing arguments from the user of the driver, then
executing steps to fully compile a program. Apart from the compiler's builtin tasks, like lexing
tokens, parsing, and target program generation, it needs to call into external programs, in order
to compile resulting C code and linking the object files into an executable, and finally do cleanup.

For this, I've designed a job system that splits all these tasks into singular units,
called **jobs**, that are then executed in order in order to complete the compilation.

The main compilation structure consists of the following:
```c
struct yf_compilation_data {
	/** @item_type yf_compilation_job */
    struct yf_list jobs;

    char * project_name;

	/** @item_type yfs_symtab */
    struct yf_hashmap symtables;

	/** @item_type ? */
    struct yf_list garbage;
};
```

- `jobs` is a list of jobs to execute.
- `project_name` is the name of the compiled project, if any.
- `symtables` stores references to symbol tables for each translation unit included in the compilation.
- `garbage` collects references to allocated objects that need to be freed after compilation finishes.

The compilation data are collected by `yf_create_compilation_data` and are executed inside `yf_run_compiler`.
`yf_create_compilation_data` delegates to either `yf_compile_project` or `yf_compile_files`, depending on
whether the driver is operating in project mode. (If driver is not in project mode, `project_name` is null.)

# Collecting translation unit data

Both drivers use an auxiliary structure `yf_project_compilation_data` that stores the project name
(project directory) and a map from file names to structures describing the translation unit
(in `yf_compilation_unit_info` structures). This structure stores the path of the unit's source file,
its identifier prefix used to refer to this unit's symbols, and paths to its symbol and output files.

This structure also stores an information about whether the symbol file needs to be regenerated.

In each case, path of the output file is filled by the backend, not unit collecting functions.

## File driver mode
`yf_compile_files` simply iterates through all input files provided on the command line and adds
them to the translation unit map, filling only the file name field of each unit.

## Project driver mode
`yf_compile_project` delegates the task of finding project translation units to `yfd_find_projfiles`,
which looks into project's root's `src` folder and recursively searches for source files.
`yfd_add_file` function handles a single source file.

Each found source file adds a translation unit filled with source file path, symbol file path, and
identifier prefix.
The symbol file, if it exists, is examined to determine whether recompilation is necessary,
which is also stored in the translation unit structure.

# Creating jobs
After either driver mode is done collecting files to compile, it calls `yf_create_compiler_jobs`
to generate jobs that perform actual compilation. It takes a `yf_compilation_data` structure to fill
as well as the `yf_project_compilation_data` produced by previous function.

First, it tries to find a target C compiler. In a next version, the check should be employed only
when a compilation to object format or past it is requested. Phases before that do not rely on
a C compiler.

Then, project name is copied from project compilation data, and collected translation units are iterated over
in order to create an **analysis job** for each of them, which stores all information gathered for this
unit. It also stores the phase of compilation that needs to be reached for that unit.

```c
enum yf_compilation_stage {
    YF_COMPILE_LEXONLY,     /** Only do lexical analysis */
    YF_COMPILE_PARSEONLY,   /** Do lexical analysis and syntatic analysis. Construct a CST */
    YF_COMPILE_ANALYSEONLY, /** Do lexical and syntatic analysis, then do semantic analysis in order to obtain an AST */
    YF_COMPILE_CODEGEN,     /** Do all of the above, then produce a generated C file */
};
```

Units are processed in order. However, in order to perform semantic analysis and above,
symbol information of the full program is required. For these units, another pass is done
that adds a second **compile job**, after all **analysis jobs**, which performs
semantic analysis and code generation.

Seperately, for units that need _codegen_, driver backend prepares another job for invoking the compiler.
The **analysis** and **compile jobs** are seperate from this job, because the former run in the frontend,
while this job invokes an external program.

The resulting _object files_ of units that need _codegen_ are collected into a final link list.
If at the end this list is not empty and an executable needs to be produced, one final job
is created by the backend, which invokes the linker (through the found C compiler) that links
all generated objects into a final executable.

# Executing jobs
After all the jobs had been created, the main driver chooses what to do with them. In all cases, they're
processed in order they were added.

The fact that the job creation system is seperate from doing actual work provides an advantage.
A driver can be asked to print the jobs as they're executed,
or to _only_ print the jobs __without__ executing them.

The former can be achieved by passing `--dump-commands` command line flag, and the
latter by passing `--simulate-run` instead.

After all jobs are processed, cleanup is done.

## Analysis job
The analysis job handles lexing and parsing and is executed by `yfc_run_frontend_build_symtable`.
It opens the source file **or** the symbol file, then runs the lexer.

If the target phase was `YF_COMPILE_LEXONLY`, scanned tokens are printed.
Otherwise, analysis continues with the parser.

If the target phase was `YF_COMPILE_PARSEONLY`, the CST is dumped. Otherwise,
a symbol table for this unit is built and added to the compilation
data.

## Compile job
After all symbol tables had been collected, `yfc_validate_compile` handles the compile jobs.
On units with phase of `YF_COMPILE_ANALYSEONLY` and above, semantic analysis is performed.

Finally, units having `YF_COMPILE_CODEGEN` phase are passed to the C code generator,
resulting in a C file.

## Object compilation and linking
The remaining jobs are just program invocations.

The first kind is invoking the C compiler to translate generated C files into object files.

The second kind is invoking the linker that links the resulting object files into a full program.
