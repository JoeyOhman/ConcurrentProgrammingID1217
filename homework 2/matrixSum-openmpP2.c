/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>

double start_time, end_time;

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size;

int total = 0, min = INT_MAX, max = INT_MIN;
int minI, minJ, maxI, maxJ;

int main(int argc, char *argv[]) {

  int i, j;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  printf("Size: %d\n", size);
  int* matrix[size];
  for(i = 0; i < size; i++)
    matrix[i] = malloc(sizeof(int) * size);

  omp_set_num_threads(numWorkers);
  printf("Num of threads: %d\n", numWorkers);//omp_get_num_threads());

  /* initialize the matrix */
  #pragma omp parallel for private(j)
  for (i = 0; i < size; i++) {
    //#pragma omp parallel for
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
  #ifdef DEBUG
    for (i = 0; i < size; i++) {
      printf("[ ");
      for (j = 0; j < size; j++) {
        printf(" %d", matrix[i][j]);
      }
      printf(" ]\n");
    }
  #endif

  start_time = omp_get_wtime();
  #pragma omp parallel for reduction (+:total) reduction (min:min) reduction (max:max) private(j)
  for (i = 0; i < size; i++) {

    for (j = 0; j < size; j++){
      total += matrix[i][j];

      if(matrix[i][j] < min) {
        min = matrix[i][j];
        minI = i;
        minJ = j;
      }

      if(matrix[i][j] > max) {
        max = matrix[i][j];
        maxI = i;
        maxJ = j;
      }
    }

  }
  // implicit barrier
  end_time = omp_get_wtime();

  printf("Minimum element: %d at index [%d][%d], Maximum elment: %d at index [%d][%d]\n", min, minI, minJ, max, maxI, maxJ);

  printf("the total is %d\n", total);
  printf("it took %g milliseconds\n", (end_time - start_time) * 1000);

  for(i = 0; i < size; i++)
    free(matrix[i]);

}
