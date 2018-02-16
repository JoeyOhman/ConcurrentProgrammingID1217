/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t lock, bagMutex;  /* mutex lock for the barrier */
int numWorkers;           /* number of workers */
int bagOfRows = 0;

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size;  /* assume size is multiple of numWorkers */
int total = 0, min = INT_MAX, max, minI, minJ, maxI, maxJ;
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);
int claimTask();

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&bagMutex, NULL);

  /* read command line args if any */
  size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
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

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  //pthread_exit(NULL);

  /* wait for threads */
  for(int i = 0; i < numWorkers; i++) {
    pthread_join(workerid[i], NULL);
    // printf("Thread terminated, id: %d\n", i);
  }

  /* get end time */
  end_time = read_timer();
  /* print results */
  printf("Minimum element: %d at index [%d][%d], Maximum elment: %d at index [%d][%d]\n", min, minI, minJ, max, maxI, maxJ);
  printf("The total is %d\n", total);
  printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int j, row;

  #ifdef DEBUG
    printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
  #endif

  row = claimTask();

  int i = row == -1 ? 0 : row;

  /* sum values in my strip */
  int totalStrip = 0;
  int minStrip = matrix[i][0];
  int minRowStrip = i, minColumnStrip = 0;
  int maxStrip = matrix[i][0];
  int maxRowStrip = i, maxColumnStrip = 0;


  while (row != -1) {

    for (j = 0; j < size; j++) {
      totalStrip += matrix[row][j];

      if(matrix[row][j] < minStrip) {
        minStrip = matrix[row][j];
        minRowStrip = row;
        minColumnStrip = j;
      }
      if(matrix[row][j] > maxStrip) {
        maxStrip = matrix[row][j];
        maxRowStrip = row;
        maxColumnStrip = j;
      }
    }
    //printf("Calling claimTask!\n");
    row = claimTask();
  }
  // Lock
  pthread_mutex_lock(&lock);
  total += totalStrip;

  if(minStrip < min) {
    min = minStrip;
    minI = minRowStrip;
    minJ = minColumnStrip;
  }
  if(maxStrip > max) {
    max = maxStrip;
    maxI = maxRowStrip;
    maxJ = maxColumnStrip;
  }
  // Unlock
  pthread_mutex_unlock(&lock);
}

int claimTask() {
  int row;
  pthread_mutex_lock(&bagMutex);
  //printf("Rows done: %d\n",bagOfRows);
  //printf("Rows tot: %d\n",size);
  if (bagOfRows < size) {
    row = bagOfRows++;
  } else {
    row = -1;
    //printf("No more rows in bag!\n");
  }
  pthread_mutex_unlock(&bagMutex);
  return row;
}
