/**
 * Find all of the project files. If they need to be recompiled, add them to the
 * list - if not, load their symbol tables.
 */

#ifndef DRIVER_FIND_FILES_H
#define DRIVER_FIND_FILES_H

#include <api/compilation-data.h>

struct yf_project_compilation_data {

    /** To start - also for error messages */
    char * project_name;

    /**
     * There are indeed a lot of pointers here, but this is so that a project
     * with 5 files does not take up as much space as a project with 1000.
     * @item_type yf_compilation_unit_info
     */
    struct yf_hashmap files;

};

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
