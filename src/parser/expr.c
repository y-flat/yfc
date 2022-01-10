/**
 * Expression parsing. This algorithm is complicated enough that it deserves its
 * own file. Yikes!
 */

#include <stdbool.h>
#include <string.h>

#include <api/operator.h>
#include <parser/parser-internals.h>

/**
 * So that there's ZERO confusion for future me or others, here's what this does
 * - it parses an expression that cannot EXPLICITLY be divided into two parts.
 * For example, if we're parsing:
 *  (a + b) * c - d + e
 * The "atomic expressions" are (a + b), c, d, and e.
 * The reason parenthesized expressions are atomic is because the expression
 * tree is made by organizing expressions by operator precedence, and we can't
 * split up expressions in parentheses.
 */
int yfp_atomic_expr(struct yf_parse_node * node, struct yf_lexer * lexer) {

    int lex_err;
    
    struct yf_token tok;

    node->type = YFCS_EXPR;
    
    P_LEX(lexer, &tok);
    switch (tok.type) {
    case YFT_IDENTIFIER:
        yfl_unlex(lexer, &tok);
        if (yfp_ident(&node->expr.value.identifier, lexer))
            return 1;
        break;
    case YFT_LITERAL:
    node->expr.type = YFCS_VALUE;
        strcpy(
            node->expr.value.literal.value, tok.data
        );
        node->expr.value.type = YFCS_LITERAL;
        break;
    case YFT_OPAREN:
        if (yfp_expr(node, lexer, 0, NULL))
            return 1;
        P_LEX(lexer, &tok);
        if (tok.type != YFT_CPAREN) {
            YF_TOKERR(tok, "closing parenthesis");
        }
        break;
    default:
        YF_TOKERR(tok, "identifier or literal");
        break;
    }

    return 0;

}

static int yfp_sort_expr_tree(
    struct yf_parse_node * nodes, int num_nodes,
    enum yf_operator * operators, /* num_operators = num_nodes - 1 */
    struct yfcs_expr * node
);

/**
 * Return values:
 * 0 - all OK
 * 1 - too many subbranches
 * 2 - invalid operator
 * first is whether the first atomic expr has already been parsed. If so, supply
 * the first node. This first node passed in will be copied - free it after
 * calling.
 */
int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer,
    bool first, struct yf_parse_node * first_node) {

    struct yf_parse_node atomics[64];
    enum yf_operator operators[63];

    struct yf_token tok;

    int i, lex_err;

    /**
     * Explanation for this stage of the parsing algorithm - get all atomic
     * expressions and operators. Like so:
     * (a + b) * c(d) - e
     * atomics, (a + b), c(d), e
     * operators, *, -
     */

    /**
     * We parse one atomic expr first, and then we parse [op], [atomic expr]
     * until done.
     */
    if (!first) {
        if (yfp_atomic_expr(&atomics[0], lexer)) {
            return 4;
        }
    } else {
        atomics[0] = *first_node;
    }

    P_GETCT(node, atomics[0]);

    for (i = 0; i < 64; i++) {
        P_LEX(lexer, &tok);
        if (tok.type == YFT_OP) {
            operators[i] = yf_get_operator(tok.data);
            if (operators[i] == YFO_INVALID) {
                /* TODO - error message */
                YF_TOKERR(tok, "valid operator");
                return 2;
            }
            if (yfp_atomic_expr(&atomics[i + 1], lexer)) {
                return 4;
            }
        } else {
            /* Not an error - we've simply reached the end. */
            yfl_unlex(lexer, &tok);
            break;
        }
    }

    /* Now we have all operators and atomics. */
    return yfp_sort_expr_tree(atomics, i + 1, operators, &node->expr);

}

/**
 * Sorts the child <nodes> into a tree, where <node> is the pointer to that
 * tree.
 */
static int yfp_sort_expr_tree(
    struct yf_parse_node * nodes, int num_nodes,
    enum yf_operator * operators,
    struct yfcs_expr * node
) {

    int i, index;
    enum yfo_precedence prec;

    /* First, trivial cases. */
    if (num_nodes == 1) {
        *node = nodes[0].expr;
        return 0;
    }
    if (num_nodes == 2) {
        node->type = YFCS_BINARY;
        node->binary.op = operators[0];
        node->binary.left = malloc(sizeof(struct yf_parse_node));
        node->binary.right = malloc(sizeof(struct yf_parse_node));
        if (!node->binary.left || !node->binary.right) {
            return 1;
        }
        *node->binary.left = nodes[0];
        *node->binary.right = nodes[1];
        return 0;
    }

    /**
     * We find the location of the lowest-precedence operator, and then split
     * the expression tree based on it. If there are multiple tied operators,
     * we choose the first one if right-associative, and the last one if left.
     */
    for (i = 0, index = -1; i < num_nodes - 1; i++) {
        prec = yfo_prec(operators[i], operators[index]);
        if (prec == LESS || index == -1) {
            index = i;
        }
        /* If left-associative and tied, we want the last. */
        if (prec == EQUAL && 
            yf_get_operator_assoc(operators[i]) == YFOA_LEFT
        ) {
            index = i;
        }
    }

    /* Now that we have our splitting location, we recurse. */
    node->type = YFCS_BINARY;
    node->binary.left = malloc(sizeof(struct yf_parse_node));
    node->binary.right = malloc(sizeof(struct yf_parse_node));
    if (!node->binary.left || !node->binary.right) {
        return 1;
    }
    node->binary.op = operators[index];
    yfp_sort_expr_tree(
        nodes,
        index + 1,
        operators,
        &node->binary.left->expr
    );
    yfp_sort_expr_tree(
        nodes + index + 1,
        num_nodes - index - 1,
        operators + index + 1,
        &node->binary.right->expr
    );
    return 0;

}
