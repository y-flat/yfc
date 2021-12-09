/**
 * Main file for the yfc compiler.
 */

#include <driver/args.h>
#include <driver/compile.h>

int main(int argc, char ** argv) {
    
    struct yf_args args;
    yf_parse_args(argc, argv, &args);

    yf_compile(&args);

}
