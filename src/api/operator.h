/**
 * An enum of all binary operators. Possibly more later.
 */

#ifndef API_OPERATOR_H
#define API_OPERATOR_H

enum yf_operator {
    YFO_INVALID,
    YFO_ADD,            /* +  */
    YFO_SUB,            /* -  */
    YFO_MUL,            /* *  */
    YFO_DIV,            /* /  */
    YFO_MOD,            /* %  */
    YFO_ASSIGN,         /* =  */
    YFO_EQ,             /* == */
    YFO_NEQ,            /* != */
    YFO_LT,             /* <  */
    YFO_LTE,            /* <= */
    YFO_GT,             /* >  */
    YFO_GTE,            /* >= */
    YFO_AND,            /* &  */
    YFO_OR,             /* |  */
    YFO_XOR,            /* ^  */
    YFO_AADD,           /* += */
    YFO_ASUB,           /* -= */
    YFO_AMUL,           /* *= */
    YFO_ADIV,           /* /= */
    YFO_AMOD,           /* %= */
    YFO_AAND,           /* &= */
    YFO_AOR,            /* |= */
    YFO_AXOR,           /* ^= */
};

/**
 * Get the operator from a string. Returns YFO_INVALID if the operator is not
 * recognized. Ex: yf_get_operator("+") == YFO_ADD
 */
enum yf_operator yf_get_operator(const char * str);

/**
 * Which direction the operator is associative.
 * Left: a @ b @ c -> a @ (b @ c)
 * Right: a @ b @ c -> (a @ b) @ c
 */
enum yfo_assoc {
    YFOA_INVAL,
    YFOA_LEFT,
    YFOA_RIGHT,
};

enum yfo_assoc yf_get_operator_assoc(enum yf_operator op);

#endif /* API_OPERATOR_H */
