#include "validate.h"

/**
 * Forwards
 */
static int validate_program(
    struct yfcs_program * prog, struct yfa_program * root
);

int yfs_validate(struct yf_file_compilation_data * data) {
    return validate_program(
        &data->parse_tree.as.program, &data->ast_tree.program
    );
}

static int validate_program(
    struct yfcs_program * cprog, struct yfa_program * aprog
) {
    /* TODO */
    return 0;
}
