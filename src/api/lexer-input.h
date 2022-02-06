/**
 * Anything that can provide input to the lexer.
 * This may seem overly complex, but it's a convenient wrapper around a file and
 * will also allow others to tap into the lexer and parser without having to
 * shunt their data to a file.
 * 
 * Example usage (with a file):
 * struct yf_lexer_input input {
 *   .input = fopen("file.txt", "r"),
 *   .getc = getc,
 *   .ungetc = ungetc
 * };
 * 
 * Example usage (with a string):
 * int str_getc(char * str) {
 *  int ret;
 *  if (ret = *str++) {
 *    return ret;
 *  } else {
 *    return EOF;
 *  }
 * }
 * struct yf_lexer_input input {
 *   .input = str,
 *   .getc = str_getc,
 *   // Make up your own ungetc function.
 * };
 */

#ifndef API_LEXER_INPUT_H
#define API_LEXER_INPUT_H

struct yf_lexer_input {

    /* A pointer to whatever the input structure is. We don't care what it is -
     * as long as the getc and ungetc functions work.
     */
    void * input;

    char * input_name;

    /**
     * This function needs to provide the next character of input data.
     */
    int (*getc)(void * input);

    /**
     * This function must "unget" the last character of input data. This does
     * NOT need to hold more than 16 chars.
     */
    int (*ungetc)(int c, void * input);

    int (*close)(void * input);

};

#endif /* API_LEXER_INPUT_H */
