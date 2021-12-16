/**
 * Output utils.
 */

#ifndef UTIL_OUT_H
#define UTIL_OUT_H

#include <stdio.h>

#define YF_CODE_RED 31
#define YF_CODE_YELLOW 33
#define YF_CODE_GREEN 32
#define YF_CODE_BLUE 34
#define YF_CODE_MAGENTA 35
#define YF_CODE_CYAN 36
#define YF_CODE_WHITE 37

/**
 * Output stream is stderr.
 */

#define YF_OUTPUT_STREAM stderr

#define YF_SET_COLOR(color) fprintf(YF_OUTPUT_STREAM, "\033[%dm", color)

#define YF_RESET_COLOR(color) fprintf(YF_OUTPUT_STREAM, "\033[0m")

#define YF_PRINT_WITH_COLOR(color, ...) do { \
    YF_SET_COLOR(color); \
    fprintf(YF_OUTPUT_STREAM, __VA_ARGS__); \
    YF_RESET_COLOR(color); \
} while (0)

#define YF_PRINT_ERROR(msg, ...) do { \
    YF_PRINT_WITH_COLOR(YF_CODE_RED, "Error: yfc: " msg "\n" ,##__VA_ARGS__); \
} while (0)

#define YF_PRINT_WARNING(msg, ...) do { \
    YF_PRINT_WITH_COLOR(YF_CODE_YELLOW, "Warning: yfc: " msg "\n" ,##__VA_ARGS__); \
} while (0)

#endif /* UTIL_OUT_H */
