/**
 * Main file for the yfc compiler.
 */

#include <driver/args.h>
#include <driver/compile.h>

int main(int argc, char ** argv) {
    
    struct yf_args args;
    yf_parse_args(argc, argv, &args);

    if (yf_should_compile(&args)) {
        return yf_run_compiler(&args);
    } else {
        return yf_output_info(&args);
    }

}
