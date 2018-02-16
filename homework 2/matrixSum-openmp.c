/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>

double start_time, end_time;

#include <stdio.h>
#include <stdlib.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size;
/*
int matrix[MAXSIZE][MAXSIZE];

int sums[MAXSIZE];
int maximums[MAXSIZE];
int rowMax[MAXSIZE];
int columnMax[MAXSIZE];
int minimums[MAXSIZE];
int rowMin[MAXSIZE];
int columnMin[MAXSIZE];
*/

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {

  int i, j, total=0;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  printf("Size: %d\n", size);
  int* matrix[size];
  for(i = 0; i < size; i++)
    matrix[i] = malloc(sizeof(int) * size);

  int* maximums = malloc(sizeof(int) * size);
  int* rowMax = malloc(sizeof(int) * size);
  int* columnMax = malloc(sizeof(int) * size);
  int* minimums = malloc(sizeof(int) * size);
  int* rowMin = malloc(sizeof(int) * size);
  int* columnMin = malloc(sizeof(int) * size);

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
  #pragma omp parallel for reduction (+:total) private(j)
  for (i = 0; i < size; i++) {

    int min = matrix[i][0];
    //int minRow = row;
    int minColumn = 0;
    int max = matrix[i][0];
    //int maxRow = row;
    int maxColumn = 0;

    for (j = 0; j < size; j++){
      total += matrix[i][j];

      if(matrix[i][j] < min) {
        min = matrix[i][j];
        //minRow = row;
        minColumn = j;
      }
      if(matrix[i][j] > max) {
        max = matrix[i][j];
        //maxRow = row;
        maxColumn = j;
      }
    }

    minimums[i] = min;
    //rowMin[i] = minRow;
    columnMin[i] = minColumn;
    maximums[i] = max;
    //rowMax[i] = maxRow;
    columnMax[i] = maxColumn;

  }
  // implicit barrier

  // Summarize data from all threads, after synchronization
  int min = minimums[0];
  int minI = 0, minJ = columnMin[0];
  int maxI = 0, maxJ = columnMax[0];
  int max = maximums[0];
  for(int i = 1; i < size; i++) {
    if(minimums[i] < min) {
      min = minimums[i];
      minI = i;
      minJ = columnMin[i];
    }
    if(maximums[i] > max) {
      max = maximums[i];
      maxI = i;
      maxJ = columnMax[i];
    }
  }
  printf("Minimum element: %d at index [%d][%d], Maximum elment: %d at index [%d][%d]\n", min, minI, minJ, max, maxI, maxJ);


  end_time = omp_get_wtime();

  printf("the total is %d\n", total);
  printf("it took %g seconds\n", end_time - start_time);

  for(i = 0; i < size; i++)
    free(matrix[i]);

  free(maximums);
  free(rowMax);
  free(columnMax);
  free(minimums);
  free(rowMin);
  free(columnMin);

}
