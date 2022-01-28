/**
 * Find all of the project files. If they need to be recompiled, add them to the
 * list - if not, load their symbol tables.
 */

#ifndef DRIVER_FIND_FILES_H
#define DRIVER_FIND_FILES_H

#include <api/compilation-data.h>

/**
 * Returns:
 * 0 - OK
 * 1 - no src/ folder
 * 2 - folder name too long
 * 3 - too many files
 * 4 - internal error
 */
int yfd_find_projfiles(struct yf_project_compilation_data * data);

#endif /* DRIVER_FIND_FILES_H */
