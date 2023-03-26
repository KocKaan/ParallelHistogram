#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
  int rank, size;
  int i, j, k, num_elements, num_bins;
  double range_size, num;
  double *data;
  int *local_histogram, *global_histogram;
  char *filename;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc != 3) {
    if (rank == 0) {
      printf("Usage: mpiexec -n <num_processes> %s <num_bins> <filename>\n", argv[0]);
    }
    MPI_Finalize();
    return 0;
  }

  num_bins = atoi(argv[1]);
  filename = argv[2];

  if (num_bins <= 0 || num_bins > 50) {
    if (rank == 0) {
      printf("Number of bins must be between 1 and 50\n");
    }
    MPI_Finalize();
    return 0;
  }

  if (rank == 0) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
      printf("Could not open file: %s\n", filename);
      MPI_Finalize();
      return 0;
    }
    fscanf(fp, "%d", &num_elements);

    data = (double*) malloc(num_elements * sizeof(double));
    for (i = 0; i < num_elements; i++) {
      fscanf(fp, "%lf", &data[i]);
    }
    fclose(fp);
  }

  MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int elements_per_proc = ceil(num_elements * 1.0 / size);

  local_histogram = (int*) calloc(num_bins, sizeof(int));

  int start_index = rank * elements_per_proc;
  int end_index = (rank + 1) * elements_per_proc;
  if (end_index > num_elements) {
    end_index = num_elements;
  }

  for (i = start_index; i < end_index; i++) {
    num = data[i];
    for (j = 0; j < num_bins; j++) {
      range_size = 100.0 / num_bins;
      if (num >= j * range_size && num < (j + 1) * range_size) {
        local_histogram[j]++;
        break;
      }
    }
  }

  global_histogram = (int*) calloc(num_bins, sizeof(int));
  MPI_Reduce(local_histogram, global_histogram, num_bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (k = 0; k < num_bins; k++) {
      printf("bin[%d] = %d\n", k, global_histogram[k]);
    }
    free(data);
  }

  free(local_histogram);
  free(global_histogram);

  MPI_Finalize();

  return 0;
}
