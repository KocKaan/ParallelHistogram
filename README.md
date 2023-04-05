# ParallelHistogram

Assume your program is called histogram, the command line is expected to be:
mpiexec -n x ./histogram b filename
Where:
b: is the number of bins, 0 < b <= 50
x: number of processes, 0 < t <= 50
filename: the name of the text file that contains the floating point numbers

You compile your code with:
mpicc -Wall -std=c99 -o histogram main.c

Do the compilation and execution on crunchyx (x = 1, 3, 5, or 6) machines.
