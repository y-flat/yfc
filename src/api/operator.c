#include "operator.h"

#include <stdlib.h> /* NULL */
#include <string.h>

/**
 * Get the type of operator from its string representation.
 */
enum yf_operator yf_get_operator(const char * str) {

    static struct opdef {
        const char * str;
        enum yf_operator op;
    } operator_defs[] = {
        { "+",   YFO_ADD         },
        { "-",   YFO_SUB         },
        { "*",   YFO_MUL         },
        { "/",   YFO_DIV         },
        { "%",   YFO_MOD         },
        { "=",   YFO_ASSIGN      },
        { "==",  YFO_EQ          },
        { "!=",  YFO_NEQ         },
        { "<",   YFO_LT          },
        { "<=",  YFO_LTE         },
        { ">",   YFO_GT          },
        { ">=",  YFO_GTE         },
        { "&",   YFO_AND         },
        { "|",   YFO_OR          },
        { "^",   YFO_XOR         },
        { "+=",  YFO_AADD        },
        { "-=",  YFO_ASUB        },
        { "*=",  YFO_AMUL        },
        { "/=",  YFO_ADIV        },
        { "%=",  YFO_AMOD        },
        { "&=",  YFO_AAND        },
        { "|=",  YFO_AOR         },
        { "^=",  YFO_AXOR        },
        { NULL,  YFO_INVALID     },
    };

    struct opdef * opdef;

    for (opdef = operator_defs; opdef->str != NULL; ++opdef) {
        if (strcmp(opdef->str, str) == 0) {
            return opdef->op;
        }
    }

    return YFO_INVALID;

}

/**
 * Get the association of an operator. If @ is the operator:
 * Left associative: a @ b @ c -> (a @ b) @ c
 * Right associative: a @ b @ c -> a @ (b @ c)
 */
enum yfo_assoc yf_get_operator_assoc(enum yf_operator op) {
    
        switch (op) {
            case YFO_ADD:
            case YFO_SUB:
            case YFO_MUL:
            case YFO_DIV:
            case YFO_MOD:
            case YFO_EQ:
            case YFO_NEQ:
            case YFO_LT:
            case YFO_LTE:
            case YFO_GT:
            case YFO_GTE:
            case YFO_AND:
            case YFO_OR:
            case YFO_XOR:
                return YFOA_LEFT;
            case YFO_AADD:
            case YFO_ASUB:
            case YFO_AMUL:
            case YFO_ADIV:
            case YFO_AMOD:
            case YFO_AAND:
            case YFO_AOR:
            case YFO_AXOR:
            case YFO_ASSIGN:
                return YFOA_RIGHT;
            default:
                return YFOA_INVAL;
        }
    
}

/**
 * There are multiple precedence tiers - higher levels are more tightly binding
 * than lower levels. As an example, assume @1 and @2 are operators, with @2
 * having a higher precedence.
 * x @1 y @2 z is ALWAYS parsed as x @1 (y @2 z).
 */
static int get_precedence_tier(enum yf_operator op) {

    static int precedences[] = {
        0, /* YFO_INVALID */
        4, /* YFO_ADD */
        4, /* YFO_SUB */
        5, /* YFO_MUL */
        5, /* YFO_DIV */
        4, /* YFO_MOD */
        1, /* YFO_ASSIGN */
        3, /* YFO_EQ */
        3, /* YFO_NEQ */
        3, /* YFO_LT */
        3, /* YFO_LTE */
        3, /* YFO_GT */
        3, /* YFO_GTE */
        2, /* YFO_AND */
        2, /* YFO_OR */
        2, /* YFO_XOR */
        1, /* YFO_AADD */
        1, /* YFO_ASUB */
        1, /* YFO_AMUL */
        1, /* YFO_ADIV */
        1, /* YFO_AMOD */
        1, /* YFO_AAND */
        1, /* YFO_AOR */
        1, /* YFO_AXOR */
    };

    return precedences[op];

}

enum yfo_precedence yfo_prec(enum yf_operator op1, enum yf_operator op2) {
    int x1, x2;
    x1 = get_precedence_tier(op1);
    x2 = get_precedence_tier(op2);
    if (x1 > x2) {
        return GREATER;
    } else if (x1 < x2) {
        return LESS;
    } else {
        return EQUAL;
    }
}

/**
 * Convert operator back to string.
 * TODO - separate into "get Y-flat operator" and "get C operator".
 */
char * get_op_string(enum yf_operator op) {

    static char * op_strings[] = {
        "INVALID",
        "+",
        "-",
        "*",
        "/",
        "%",
        "=",
        "==",
        "!=",
        "<",
        "<=",
        ">",
        ">=",
        "&",
        "|",
        "^",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
        "&=",
        "|=",
        "^=",
    };

    return op_strings[op];

}

/**
 * Does this operator require a variable as a left-hand side?
 */
bool yfo_is_assign(enum yf_operator op) {
    switch (op) {
    case YFO_AADD:
    case YFO_ASUB:
    case YFO_AMUL:
    case YFO_ADIV:
    case YFO_AMOD:
    case YFO_AAND:
    case YFO_AOR:
    case YFO_AXOR:
    case YFO_ASSIGN:
        return true;
    default:
        return false;
    }
}

/**
 * Is the value of this operator a boolean? Other operators (for now) are valued
 * as the larger-sized type of their operands. Once user-defined operators are a
 * thing, this will change.
 */
bool yfo_is_bool(enum yf_operator op) {
    switch (op) {
    case YFO_EQ:
    case YFO_NEQ:
    case YFO_LT:
    case YFO_LTE:
    case YFO_GT:
    case YFO_GTE:
        return true;
    default:
        return false;
    }
}
