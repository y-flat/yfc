#include "operator.h"

#include <stdlib.h> /* NULL */
#include <string.h>

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
        { "=",   YFO_EQ          },
        { "=",   YFO_EQ          },
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
