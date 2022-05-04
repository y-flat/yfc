#include "find-files.h"

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <api/compilation-data.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Scan through all files in a folder, adding them to the project data.
 */
static int yfd_folder_scan(
    struct yf_project_compilation_data * data,
    const char * folder_name
);

/**
 * Add a file to either the "recompile" list or the "compiled" list.
 */
static int yfd_add_file(
    struct yf_project_compilation_data * data,
    const char * file_name
);

/**
 * Whether or not recompilation happens.
 */
static int yfd_should_recompile(const char * src_file, const char * sym_file);

/**
 * Get symbol file name.
 */
static int yfd_get_sym_file_name(
    struct yf_project_compilation_data * data,
    const char * file_name,
    char * sym_file_name
);

int yfd_find_projfiles(struct yf_project_compilation_data * data) {
    
    if (access("src", R_OK) != 0) {
        YF_PRINT_ERROR("Source folder 'src' not found");
        return 1;
    }

    return yfd_folder_scan(data, "src");

}

static int yfd_folder_scan(
    struct yf_project_compilation_data * data,
    const char * folder_name
) {

    DIR * dir;
    struct dirent * entry;
    char sub_entry_buf[256];
    int ret = 0;

    dir = opendir(folder_name);
    if (!dir) {
        YF_PRINT_ERROR("Internal error: could not open folder %s", folder_name);
        return 4;
    }

    /**
     * Loop through folder, recursing if necessary, to add all files.
     */
    for (;;) {
        entry = readdir(dir);
        if (!entry) {
            break;
        }
        if (entry->d_type == DT_DIR) {

            /* Skip . and .. */
            if (
                strcmp(entry->d_name, ".") == 0
                || strcmp(entry->d_name, "..") == 0
            ) {
                continue;
            }

            snprintf(
                sub_entry_buf, sizeof(sub_entry_buf),
                "%s/%s",
                folder_name, entry->d_name
            );
            if (strlen(folder_name) + strlen(entry->d_name) + 2 > 256) {
                YF_PRINT_ERROR(
                    "Internal error: path too long: %s", sub_entry_buf
                );
                ret = 4;
                goto out;
            }

            if ( (ret = yfd_folder_scan(data, sub_entry_buf))) {
                goto out;
            }

        } else if (entry->d_type == DT_REG) {

            snprintf(
                sub_entry_buf, sizeof(sub_entry_buf),
                "%s/%s",
                folder_name, entry->d_name
            );
            
            if ( (ret = yfd_add_file(data, sub_entry_buf))) {
                goto out;
            }
        }

    }

out:
    closedir(dir);
    return ret;

}

/**
 * Find the identifier prefix for a file.
 */
int yfd_get_identifier_prefix(
    struct yf_project_compilation_data * data,
    const char * file_name,
    char * prefix
);

int yfd_add_file(
    struct yf_project_compilation_data * data,
    const char * file_name
) {

    struct yf_compilation_unit_info * file;

    file = yf_malloc(sizeof(struct yf_compilation_unit_info));
    // Don't forget the NUL char, char implicitly guarranteed 1 byte
    file->file_name   = yf_strdup(file_name);
    file->file_prefix = yf_malloc(strlen(file_name) + 1);
    yfd_get_identifier_prefix(data, file_name, file->file_prefix);

    /* 16 is not really specific, but enough to hold the transformed name */
    file->sym_file = yf_malloc(strlen(file_name) + 16);
    yfd_get_sym_file_name(data, file->file_name, file->sym_file);

    file->output_file = NULL;

    if (yfd_should_recompile(file->file_name, file->sym_file) != 0) {
        file->parse_anew = 1;
    } else {
        file->parse_anew = 0;
    }

    if (yfh_set(&data->files, file_name, file)) {
        YF_PRINT_ERROR("Internal error: could not add file %s", file_name);
        return 4;
    }

    return 0;

}

int yfd_should_recompile(const char * src_file, const char * sym_file) {

    /* Should recompile on one of two conditions:
     * - The symbol file does not exist
     * - The source file is newer than the symbol file
     */

    struct stat srcstat, symstat;

    if (stat(src_file, &srcstat) == -1) {
        /* Source file does not exist */
        return -1; /* Error, not handled right now */
    }

    if (stat(sym_file, &symstat) == -1) {
        /* Symbol file does not exist */
        return 1;
    }

    /* If they both exist, see if the source is newer than the symbols */
    if (srcstat.st_mtime > symstat.st_mtime) {
        return 1;
    }

    return 0;

}

int yfd_get_sym_file_name(
    struct yf_project_compilation_data * data,
    const char * file_name,
    char * sym_file_name
) {

    /*
    file name: src/file.yf
    sym file name: bin/sym/file.yfsym
    */

    file_name = strchr(file_name, '/') + 1;

    sym_file_name = yf_strcpy(sym_file_name, "bin/sym/");
    sym_file_name = yf_strcpy(sym_file_name, file_name);
    sym_file_name = yf_strcpy(sym_file_name, "sym");

    return 0;

}

int yfd_get_identifier_prefix(
    struct yf_project_compilation_data * data,
    const char * file_name,
    char * prefix
) {

    char * original_prefix_loc = prefix;
    
    /*
    file name: src/path/to/file.yf
    prefix: path.to.file
    */

    file_name = strchr(file_name, '/') + 1;
    
    /**
     * Copy in characters, replacing '/' with '.'
     */
    while (*file_name) {
        if (*file_name == '/') {
            *prefix = '.';
        } else {
            *prefix = *file_name;
        }
        prefix++;
        file_name++;
    }

    /**
     * Remove trailing .yf
     * We search original_prefix_loc
     */
    prefix = original_prefix_loc;
    strrchr(prefix, '.')[0] = '\0';

    return 0;
    
}
