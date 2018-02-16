#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

#define MAXWORKERS 10

void* quicksort(void*);
int* partition(int*, int*);
void swap(int*, int*);
void printArr(int[], int);
double read_timer();
void randomize(int[], int);

struct lowHighStruct {
  int* low;
  int* high;
};

double start_time, end_time; /* start and end times */
int maxThreads;

int n;

int main(int argc, char* argv[]) {
  maxThreads = MAXWORKERS;
  n = argc > 1 ? atoi(argv[1]) : 10;
  if(argc > 2)
    maxThreads = atoi(argv[2]) > MAXWORKERS ? MAXWORKERS : atoi(argv[2]);

  omp_set_num_threads(maxThreads);
  printf("MaxThreads: %d\n", maxThreads);

  int* list = malloc(sizeof(int) * n);

  #pragma omp parallel for
  for(int i = 0; i < n; i++) {
    list[i] = i;
  }
  randomize(list, n);
  list[0] = n/2;

  printf("First element: %d\n", list[0]);
  struct lowHighStruct lowHigh;
  lowHigh.low = list;
  lowHigh.high = &list[n-1];

  printf("Length of array: %d\n", n);
  //printArr(list, n);
  start_time = read_timer();

  quicksort((void*) &lowHigh);

  end_time = read_timer();
  //printArr(list, n);
  printf("Time elapsed while sorting: %g millis\n", (end_time - start_time)*1000);
}

// A function to generate a random permutation of arr[]
void randomize ( int arr[], int n ) {
    #pragma omp parallel for
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        swap(&arr[i], &arr[j]);
    }
}

void printArr(int list[], int n) {
  printf("\n[");
  for(int i = 0; i < n; i++)
    printf(" %d ", list[i]);
  printf("]\n");
}

void* quicksort(void* lowHigh) {

    //omp_set_num_threads(maxThreads);
    struct lowHighStruct* lhs = (struct lowHighStruct*) lowHigh;
    int* low = (*lhs).low;
    int* high = (*lhs).high;

    if(low < high) {
      int* mid = partition(low, high);
      struct lowHighStruct lowHighLeft;
      lowHighLeft.low = low;
      lowHighLeft.high = mid-1;
      struct lowHighStruct lowHighRight;
      lowHighRight.low = mid+1;
      lowHighRight.high = high;

      #pragma omp parallel
      {
        //printf("Threads: %d\n", omp_get_num_threads());
        #pragma omp single nowait
        {
          #pragma omp task
          {
            //int id = omp_get_thread_num();
            //printf("Threadid L: %d out of %d threads.\n", id, omp_get_num_threads());
            quicksort((void *) &lowHighLeft);
            //printf("Threads: %d\n", omp_get_num_threads());
          }
          quicksort((void *) &lowHighRight);
          //printf("Threadid R: %d out of %d threads.\n", omp_get_thread_num(), omp_get_num_threads());
          #pragma omp taskwait
        }
      }
      // Implicit barrier
  }

}

// Partitions list using first element as pivot, returning pointer to pivot.
int* partition(int* low, int* high) {

  int* pivot = low; // Selects first element in sublist as pivot
  int* upperLimit = high;
  high++;
  while(true) {
    // Move pointers until a swap should be made, or they have passed each other
    while (*(++low) < *pivot)
      if (low == upperLimit) break;

    while (*(--high) > *pivot)
      if (high == pivot) break;

    if(low >= high) break;
    swap(low, high);

  }

  // Swap pivot into its place
  swap(pivot, high);
  return high;

}

// Recieves two pointers, swaps the values which they point to
void swap(int* a, int* b) {
  //printf("Swapping %d with %d\n", *a, *b);
  int temp = *a;
  *a = *b;
  *b = temp;
}

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
