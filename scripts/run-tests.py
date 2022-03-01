#!/bin/env python3

"""
Run tests.
Each test file is paired with a value of whether it should pass or not.
"""

import json
import os, os.path
import subprocess
import sys

tests = []

def add_test(test, flags, pass_):
    tests.append( (test, flags, pass_) )

yfc_path = "./cmake/yfc"
def run_tests():

    total = passed = failed = 0
    failed_files = []

    for test, flags, pass_ in tests:
        retcode = subprocess.call( (yfc_path, *flags, test),
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL)
        success = retcode == 0
        if (success != pass_):
            print(f"\033[91mFAIL: {test}\033[0m")
            failed += 1
            failed_files.append(test)
        else:
            print(f"\033[92mPass: {test}\033[0m")
            passed += 1
        total += 1

    print(f"{passed}/{total} passed")
    if failed > 0:
        print(f"Failed tests:")
        for file in failed_files:
            print(f"\t{file}")
        return 1
    return 0

# TODO - more detailed result checking than just "pass/fail"
test_dir = "tests"
def main():

    if not os.path.exists(yfc_path):
        print(f"\033[91mFATAL ERROR: Compiler executable {yfc_path!r} not found. Maybe you forgot to build it?\033[0m")
        return 2

    for dir in os.scandir(test_dir):
        if not dir.is_dir():
            continue
        if not os.path.exists(os.path.join(dir.path, "index.json")):
            continue
        with open(os.path.join(dir.path, "index.json")) as f:
            index = json.load(f)
        flags = index['flags'] if 'flags' in index else []
        for unit, sig in index['tests'].items():
            testfile = os.path.join(dir.path, unit + '.yf')
            if not os.path.exists(testfile):
                print(f"\033[93mWarning: Test {dir.name}/{unit} does not exist\033[0m")
                continue
            add_test(testfile, flags, sig['pass'])

    return run_tests()

if __name__ == "__main__":
    sys.exit(main())
