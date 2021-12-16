"""
Test the lexer. This creates two files - one of 25,000 lines and one of 50,000
lines. Then, it runs the lexer on each of these 10
times, and uses the time to calculate the time per line, by finding the slope
of the line."""

import os
import time

file1 = None
file2 = None

def setup_files():
    global file1
    global file2
    file1 = open("temp-file-25k.txt", "w")
    file2 = open("temp-file-50k.txt", "w")
    for i in range(12500):
        file1.write("x: int = 4;\nxyz: float = 53;\n")
    for i in range(25000):
        file2.write("x: int = 4;\nxyz: float = 53;\n")

def release_files():
    file1.close()
    file2.close()
    os.system("rm temp-file-25k.txt")
    os.system("rm temp-file-50k.txt")

def main():

    setup_files()

    stime = time.time()
    for i in range(10):
        os.system("./cmake/yfc temp-file-25k.txt --dump-tokens 1>/dev/null")
    etime = time.time()
    time_25k = (etime - stime) / 10
    print("25k: " + str(time_25k))

    stime = time.time()
    for i in range(10):
        os.system("./cmake/yfc temp-file-50k.txt --dump-tokens 1>/dev/null")
    etime = time.time()
    time_50k = (etime - stime) / 10
    print("50k: " + str(time_50k))

    release_files()

    tdiff = time_50k - time_25k

    print("Time diff: " + str(tdiff))
    print("Constant startup time: " + str(time_25k - tdiff))
    print(
        "Time per line: " + str((tdiff / 25000)) +
        " (" + str(int(25000 / tdiff)) + " lines per second)"
    )

if __name__ == "__main__":
    main()
