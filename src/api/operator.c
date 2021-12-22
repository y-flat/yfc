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
