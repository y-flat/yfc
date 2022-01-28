#include "find-files.h"

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Scan through all files in a folder, adding them to the project data.
 */
int yfd_folder_scan(
    struct yf_project_compilation_data * data,
    const char * folder_name
);

/**
 * Add a file to either the "recompile" list or the "compiled" list.
 */
int yfd_add_file(
    struct yf_project_compilation_data * data,
    const char * file_name
);

/**
 * Whether or not recompilation happens.
 */
int yfd_should_recompile(const char * src_file, const char * sym_file);

/**
 * Get symbol file name.
 */
int yfd_get_sym_file_name(
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

int yfd_folder_scan(
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

    strcpy(sub_entry_buf, folder_name);
    strcat(sub_entry_buf, "/");

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

            strcat(sub_entry_buf, "/");
            strcat(sub_entry_buf, entry->d_name);
            
            if ( (ret = yfd_add_file(data, entry->d_name))) {
                goto out;
            }
        }

    }

out:
    closedir(dir);
    return ret;

}

int yfd_add_file(
    struct yf_project_compilation_data * data,
    const char * file_name
) {

    struct yf_file_compilation_data * file;

    if (data->num_files++ >= 1000) {
        YF_PRINT_ERROR("Too many files");
        return 4;
    }

    file = yf_malloc(sizeof(struct yf_file_compilation_data));
    file->file_name = yf_malloc(sizeof (char) * strlen(file_name));
    strcpy(file->file_name, file_name);
    file->error = 0;

    /* 16 is not really specific, just some extra padding */
    file->sym_file = yf_malloc(sizeof (char) * strlen(file_name) + 16);
    yfd_get_sym_file_name(data, file->file_name, file->sym_file);

    if (yfd_should_recompile(file->file_name, file->sym_file) > 0) {
        file->parse_anew = 1;
    } else {
        file->parse_anew = 0;
    }

    data->files[data->num_files - 1] = file;

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

    /* file name: src/file.yf
     * sym file name: bin/sym/file.yf.yfsym
     */

    file_name += strlen("src/");
    strcpy(sym_file_name, "bin/sym/");
    strcat(sym_file_name, file_name);
    strcat(sym_file_name, ".yfsym");

    return 0;

}
