#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define MAX_INT 100

int main(int argc, char **argv) {
    int rank, size, x, b, i, j, *arr, *hist, *local_hist;
    double bin_size;
    FILE *fp;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: mpiexec -n x ./histogram b filename\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    b = atoi(argv[1]);
    if (b <= 0 || b > 50) {
        if (rank == 0) {
            printf("Error: number of bins (b) must be between 1 and 50\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (rank == 0) {
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            printf("Error: could not open file %s\n", argv[2]);
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        fscanf(fp, "%d", &x);

        arr = (int*) malloc(x * sizeof(int));
        for (i = 0; i < x; i++) {
            fscanf(fp, "%d", &arr[i]);
        }

        fclose(fp);

        hist = (int*) calloc(b, sizeof(int));
        bin_size = ceil((double) MAX_INT / b);

        for (i = 0; i < x; i++) {
            for (j = 0; j < b; j++) {
                if (arr[i] < (j + 1) * bin_size) {
                    hist[j]++;
                    break;
                }
            }
        }
    }

    MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_hist = (int*) calloc(b, sizeof(int));
    bin_size = ceil((double) MAX_INT / b);

    int chunk_size = x / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? x : (rank + 1) * chunk_size;

    for (i = start; i < end; i++) {
        for (j = 0; j < b; j++) {
            if (arr[i] < (j + 1) * bin_size) {
                local_hist[j]++;
                break;
            }
        }
    }

    MPI_Reduce(local_hist, hist, b, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (i = 0; i < b; i++) {
            printf("bin[%d] = %d\n", i, hist[i]);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
