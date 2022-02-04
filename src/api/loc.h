/**
 * The location of a particular token or node.
 */

#ifndef API_LOC_H
#define API_LOC_H

struct yf_location {
    int    line;
    int    column;
    char * file;
};

#endif /* API_LOC_H */
