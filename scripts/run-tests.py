"""Run tests. Each test file is paired with a value of
whether it should pass or not."""

import os
import sys

tests = []

def add_test(test, flags, pass_):
    tests.append((f"{test} {flags}", pass_))

def add_lexer_test(test, pass_):
    add_test(f"{test}", "--dump-tokens", pass_)

def add_parser_test(test, pass_):
    add_test(f"{test}", "--dump-cst", pass_)

def add_semantic_test(test, pass_):
    add_test(f"{test}", "--just-semantics", pass_)

def run_tests():
    total = passed = failed = 0
    failed_files = []
    # Cursed. TODO: get system success value in a better way
    for test, pass_ in tests:
        ret = os.system(f"./cmake/yfc {test} &>/dev/null")
        ret = (ret == 0) # ret = True is a pass of the file, False is a fail
        if (ret != pass_):
            print(f"\033[91mFAIL: {test.split()[0]}\033[0m")
            failed += 1
            failed_files.append(test)
        else:
            print(f"\033[92mPass: {test.split()[0]}\033[0m")
            passed += 1
        total += 1
    print(f"{passed}/{total} passed")
    if failed > 0:
        print(f"Failed tests:")
        for file in failed_files:
            print(f"\t{file.split()[0]}")
        return 1
    return 0

# TODO - more detailed result checking than just "pass/fail"
def main():

    add_lexer_test("tests/lexer/broken-comment.yf",          False)
    add_lexer_test("tests/lexer/keyword-test.yf",            True )
    add_lexer_test("tests/lexer/large-token.yf",             False)
    add_lexer_test("tests/lexer/lexer-test.yf",              True )

    add_parser_test("tests/parser/funccall-parsing.yf",      True )
    add_parser_test("tests/parser/funcdecl-parsing.yf",      True )
    add_parser_test("tests/parser/op-parsing.yf",            True )
    add_parser_test("tests/parser/return-stmt.yf",           True )
    add_parser_test("tests/parser/vardecl-neq.yf",           False)
    add_parser_test("tests/parser/vardecl-test.yf",          True )

    add_semantic_test("tests/sem/duplicate-decl-global.yf",  False)
    add_semantic_test("tests/sem/duplicate-decl-local.yf",   False)
    add_semantic_test("tests/sem/funccall-bad.yf",           False)
    add_semantic_test("tests/sem/funccall-good.yf",          True )
    add_semantic_test("tests/sem/funccall-types.yf",         False)
    add_semantic_test("tests/sem/funcs-fail.yf",             False)
    add_semantic_test("tests/sem/funcs-pass.yf",             True )
    add_semantic_test("tests/sem/if-condition-not-bool.yf",  False)
    add_semantic_test("tests/sem/if-good.yf",                True )
    add_semantic_test("tests/sem/invalid-int-literal.yf",    False)
    add_semantic_test("tests/sem/no-value-assign.yf",        False)
    add_semantic_test("tests/sem/return-bad.yf",             False)
    add_semantic_test("tests/sem/return-block.yf",           True )
    add_semantic_test("tests/sem/return-good.yf",            True )
    add_semantic_test("tests/sem/return-warn.yf",            True )
    add_semantic_test("tests/sem/shadow.yf",                 True )
    add_semantic_test("tests/sem/type-val-pass.yf",          True )
    add_semantic_test("tests/sem/unknown-type.yf",           False)

    return run_tests()

if __name__ == "__main__":
    sys.exit(main())
