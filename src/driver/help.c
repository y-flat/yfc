#include "help.h"

const char
    * VERSION_MSG = "Version: 0.0.1\n",
    * USAGE_MSG = "Flags:\n"
      "-h, --help: Display this message.\n"
      "-v, --version: Display version.\n"
      "-native-compiler <compiler>: specify the native C compiler to use.\n",
    * HELP_HINT_MSG = "Invalid command. "
      "Use \"-h\" or \"--help\" for a list of possible commands.\n",
    * NO_ARGS_MSG = "yfc: Use \"-h\" or \"--help\" "
      "for a list of possible commands.\n"
;
