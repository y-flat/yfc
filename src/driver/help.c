#include "help.h"

const char
    * VERSION_MSG = "Version: 0.0.1\n",
    * USAGE_MSG =
      "yfc [options...] <file1> <file2> ...: compile and link the given files.\n"
      "Flags (long options can be specified with prefix - or --):\n"
      "-h, -?, --help: Display this message.\n"
      "-v, --version: Display version.\n"
      "--native-compiler <compiler>: specify the native C compiler to use.\n"
      "--compiler-type gcc|msvc: specify the flavor of the native C compiler. (default: gcc)\n"
      "--project: Compile project. Read documentation for more specifics on this flag.\n"
      "--dump-tokens: Print out all tokens and exit.\n"
      "--dump-cst: Print out the CST and exit.\n"
      "--just-semantics: Only verify the program, do not run generation.\n"
      "--benchmark: Print out time taken for each step.\n"
      "--dump-projfiles: Print out all files in a project.\n"
      "--dump-commands: Print all compiler invocations.\n"
      "--simulate-run: Like --dump-commands, print all compiler invocations, but don't actually do anything.\n"
      ,
    * HELP_HINT_MSG = "Invalid command. "
      "Use \"-h\" or \"--help\" for a list of possible commands.\n",
    * NO_ARGS_MSG = "yfc: Use \"-h\" or \"--help\" for a list of possible commands.\n"
;
