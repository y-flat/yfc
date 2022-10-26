#include "help.h"

const char
    * VERSION_MSG = "Version: 0.0.1\n",
    * USAGE_MSG = "Flags:\n"
      "-h, --help: Display this message.\n"
      "-v, --version: Display version.\n"
      "-native-compiler <compiler>: specify the native C compiler to use.\n"
      "-compiler-type gcc|msvc: specify the class of a native C compiler. (default: gcc)\n"
      "yfc <file1> <file2> ...: compile and link the given files, up to 16.\n"
      "--project: Compile all files in src/ that need to be compiled. "
      "Read documentation for more specifics on this flag.\n"
      "--dump-tokens: Test lexer by printing out all tokens.\n"
      "--dump-cst: Test parser by printing out the CST.\n"
      "--just-semantics: Only verify the code, do not generate it.\n"
      "--just-gen: Generate the code but don't compile the C.\n"
      "--benchmark: Print out time taken for each step.\n"
      "--dump-projfiles: Print out all files in a project.\n"
      "--dump-commands: Show all compiler invocations.\n"
      "--simulate-run: Like --dump-commands, show all compiler invocations, but don't actually execute any of them.\n"
      ,
    * HELP_HINT_MSG = "Invalid command. "
      "Use \"-h\" or \"--help\" for a list of possible commands.\n",
    * NO_ARGS_MSG = "yfc: Use \"-h\" or \"--help\" "
      "for a list of possible commands.\n"
;
