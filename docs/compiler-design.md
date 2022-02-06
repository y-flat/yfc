# YFC design

## Overview

The `yfc` compiler primarily translates Y-flat source code into C code, while
doing full semantic analysis.

This document aims to explain how the internals of `yfc` work, for anyone who is
interested in diving in.

## Modules

`yfc` is made up of multiple modules, each taking up one folder, which each
function more or less independently of one another. Two of the modules, `api`
and `util`, contain little compiler-central logic themselves, while the other
modules are more integral to the compiler function.

## Control flow path

The main file of `yfc` is located in `src/driver/main.c` inside of the `driver`
module. After parsing the command-line arguments, the main compiler routines are
entered. At core, for each file, the source code is parsed into a concrete
syntax tree, then validated and transformed into an abstract syntax tree, and
finally written out in C format.

## driver

The `driver` module is the module which calls all of the other ones and collects
the disparate routines into a fully-fledged compiler. In essence, the compiler
is designed to that any module can be used independently of any other and
connected in any way the user wants. `driver` is just such a connection which
produces a full compiler.

Command-line arguments are parsed in `args.c`, and then the bulk of the code
interfacing with the other modules is entered in `compile.c`.

## lexer

The `lexer` module produces tokens from a stream of input. The stream of input
is easily configurable - it can be from a file, a string, or anything. For
example, this is the lexer input given in Y-flat:
```c
input = (struct yf_lexer_input) {
    .input = file_src,
    .getc = (int (*)(void*)) getc,
    .ungetc = (int (*)(int, void*)) ungetc,
    .input_name = file_name,
    .close = (int (*) (void*))fclose
};
```
A lexer could just as easily read from a string:
```c
struct string_input {
    char * string;
    int loc;
}

void read_char(struct string_input * in) {
    if (in->str[in->loc] == 0)
        return -1;
    return in->str[in->loc++];
}

void putback_char(struct string_input * in, char c) {
    --in->loc;
}

void close_stream() {

}

sinput = (struct string_input) {
    "~~ Y-flat code ~~\nsum(x: int, y: int): int { return x + y; }\n", 0
};

input = (struct yf_lexer_input) {
    .input = sinput,
    .getc = read_char,
    .ungetc = putback_char,
    .input_name = "<string>",
    .close = close_stream
};
```
The lexer operates according to the following algorithm: all whitespace and
comments are skipped, and then according to the type of character (alphanumeric,
integer, etc.), certain conditions are set for the end of said token. Characters
are then added to a buffer until the condition is found, at which point the
entire buffer is scanned to determine its token type and the buffer is returned.
There are also "unlex" capabilities, where a token is put back into a token
buffer contained by the lexer. At each `yfl_lex` call, this buffer is first
scanned before entering the actual lexing routine.

## parser

The `parser` module produces a concrete syntax tree from a lexer. In the future,
this may be modified to accept any source of token input (much like the lexer
accepts any source of character input). The structure of the concrete syntax
tree is outlined in `api/concrete-tree.h`, and each type of concrete node has a
corresponding parse routine which accepts a node of generic `yf_parse_node`
type and a lexer input, and constructs the node.

The parser has an interesting operator precedence algorithm to produce a correct
tree of expressions.
- First, a list of expressions and their operators is produced.
`(2 + 3) * 4 - 5 -> [(2 + 3), 4, 5], [*, -]`
- Then, the lowest-precedence operator is found, scanning from the right if
left-associative and from the left if not. Then, the list is split along that
operator and each half of the tree is recursed on.
`[(2 + 3), 4, 5], [*, -] -> {[(2 + 3), 4], [*]} - {[5]}`

## semantics

The `semantics` module accepts a concrete syntax tree and transforms it into an
abstract syntax tree, while validating the entire thing. Validation is by far
the slowest step in the compilation process, consuming ~95% of the computing
time. Just like `parser` has a routine for every node type, `semantics` has a
routine for every concrete node type to convert it into the corresponding 
abstract node type. Each of these routines also receives a pointer to the
relevant compilation data, for global identifier lookup. To facilitate global
identifier lookup, the first step of validation is to produce a global symbol
table, and then enter into the recursive validation process.

## gen

Finally, `gen` writes an AST to a file in C form. This is pretty straightforward
but will likely be more complex in the future as Y-flat gains more and more
features unknown to C.

## api and util
Thesse two modules provide more peripheral services - `api` contains all of the
data formats used to communicate between modules, and some utility routines such
as dumping CST code for debugging purposes. `util` provides wrappers for utility
structs in C, like lists, hashmaps, and allocators.
