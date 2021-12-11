/**
 * External references to strings the compiler prints out, in all processes
 * that do NOT involve actual compilation, like: "version: 1.2.3" or
 * "--my-flag: my behavior".
 */

#ifndef DRIVER_HELP_H
#define DRIVER_HELP_H

extern const char
    * VERSION_MSG,
    * USAGE_MSG,
    * HELP_HINT_MSG,
    * NO_ARGS_MSG
;

#endif /* DRIVER_HELP_H */
