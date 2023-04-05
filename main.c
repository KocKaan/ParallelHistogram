#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
  int rank, size;
  int num_elements, num_bins;
  double range_size, num;
  int *local_histogram, *global_histogram;
  char *filename;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  //printf("The size is %d \n",size);

  //check if there is right amount of inputs
  if (argc != 3) {
    if (rank == 0) {
      printf("Usage: mpiexec -n <num_processes> %s <num_bins> <filename>\n", argv[0]);
    }
    MPI_Finalize();
    return 0;
  }
  //printf("IM rank %d \n", rank);

  // get the number of bins and filename
  num_bins = atoi(argv[1]);
  filename = argv[2];


  // If the number of bins is less than 0 or bigger its wrong input
  if (num_bins <= 0 || num_bins > 50) {
    if (rank == 0) {
      printf("Number of bins must be between 1 and 50\n");
    }
    MPI_Finalize();
    return 0;
  }
  // the number of process has to be between 0 and 50
  if (size <= 0 || size > 50) {
    if (rank == 0) {
      printf("Number of process must be between 1 and 50\n");
    }
    MPI_Finalize();
    return 0;
  }

  FILE *fp =NULL;

  // if the rank is 0 meaning root process open the file
  if (rank == 0){
    fp = fopen(filename, "r");
    if (!fp) {
      printf("Could not open file: %s\n", filename);
      MPI_Finalize();
      return 0;
    }
    // num_elements is a pointer and points to the fp. %d is digit that will be pointed towards
    fscanf(fp, "%d", &num_elements);
    //printf("num elements %d\n", num_elements);
  }

  // collectively send the other processes the number of elements that we will render
  MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //dynamic allocation of space for the data that we have gathered from file
  int *data = (int*) malloc(num_elements * sizeof(int));

  // read the integers into the array
  if(rank==0){
    for (int i = 0; i < num_elements; i++) {
      fscanf(fp, "%d", &data[i]);
    }
    // for (int i = 0; i < num_elements; i++) {
    //   printf("data we store %d \n", data[i]);
    // }
    fclose(fp);
  }


  // data array holds every value that we took from the file now.
  // broadcast to all process the data we now store
  MPI_Bcast(data, num_elements, MPI_INT, 0, MPI_COMM_WORLD);


  //number of elements that each process will crunch.
  int elements_per_proc = ceil(num_elements * 1.0 / size);
  // if(rank ==0){
  //   printf("elements per data  %d \n" ,elements_per_proc);
  // }

  //printf("my rank right now %d\n",rank  );

  // dynamically allocate space for the local histogram
  local_histogram = (int*) calloc(num_bins, sizeof(int));

  //calculate the start and end index for to traverse the array
  int start_index = rank * elements_per_proc;
  int end_index = (rank + 1) * elements_per_proc;
  if (end_index > num_elements) {
    end_index = num_elements;
  }
  //printf("as a rank %d the start is %d and end is %d \n",rank, start_index,end_index);


  //traverse the data set between the start and stop index and perfor calculations
  for (int i = start_index; i < end_index; i++) {
    num = data[i];

    // for each bin check if the data fits in there.
    for (int j = 0; j < num_bins; j++) {
      range_size = 100.0 / num_bins;
      if (num >= j * range_size && num < (j + 1) * range_size) {
        local_histogram[j]++;
        break;
      }
    }
  }
  // for(int i =0; i< num_bins; i++){
  //   printf("rank %d local histogram[%d] = %d\n",rank ,i,local_histogram[i] );
  // }

  //printf("Do we still have rank? %d \n", rank );

  // this is the final result dynamically allocated
  global_histogram = (int*) calloc(num_bins, sizeof(int));

  // sum up every local histogram in the global historgram
  MPI_Reduce(local_histogram, global_histogram, num_bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // print the global histogram
  if (rank == 0) {
    for (int k = 0; k < num_bins; k++) {
      printf("bin[%d] = %d\n", k, global_histogram[k]);
    }
    free(data);
  }else{
    free(data);
  }
  //printf("do we have rank here? %d\n",rank);

  // free the dynamically allocated arrays
  free(local_histogram);
  free(global_histogram);

  MPI_Finalize();

  return 0;
}
