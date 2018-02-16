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

// 10 Million elements
//10000000
#define N 10
#define MAXWORKERS 10

//void* quicksort(int*, int*);
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
int threads = 1;
pthread_mutex_t lock;

int list[N];

int main(int argc, char* argv[]) {
  pthread_mutex_init(&lock, NULL);
  if(argc > 1)
    maxThreads = atoi(argv[1]) > MAXWORKERS ? MAXWORKERS : atoi(argv[1]);
  int n = N;

  for(int i = 0; i < n; i++) {
    list[i] = i;
  }
  randomize(list, n);
  list[0] = n/2;

  printf("First element: %d\n", list[0]);
  struct lowHighStruct lowHigh;
  lowHigh.low = list;
  lowHigh.high = &list[n-1];

  printArr(list, n);
  start_time = read_timer();
  //for(int i = 0; i < iterations; i++) {
  quicksort((void*) &lowHigh);
  //}
  end_time = read_timer();
  printArr(list, n);
  printf("Time elapsed while sorting: %g millis\n", (end_time - start_time)*1000);
}

// A function to generate a random permutation of arr[]
void randomize ( int arr[], int n ) {
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        swap(&arr[i], &arr[j]);
    }
}

void printArr(int list[], int n) {
  //printf("Length of array: %d\n", n);
  printf("\n[");
  for(int i = 0; i < n; i++)
    printf(" %d ", list[i]);
  printf("]\n");
}

//void* quicksort(int* low, int* high) {
void* quicksort(void* lowHigh) {
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

    bool threadCreated = false;

    pthread_t tid;
    pthread_mutex_lock(&lock);
    if(threads < maxThreads) {
      threads++;
      //printf("Creating thread number: %d\n", threads);
      //printf("Maxthreads: %d\n", maxThreads);

      pthread_create(&tid, NULL, quicksort, (void *) &lowHighLeft);
      threadCreated = true;
    }
    pthread_mutex_unlock(&lock);

    if(! threadCreated)
      quicksort((void *) &lowHighLeft);
    //quicksort(low, mid);
    //quicksort(mid+1, high);
    quicksort((void *) &lowHighRight);
    if(threadCreated) {
      pthread_join(tid, NULL);
      threads--;
    }
  }

}

// Partitions list using first element as pivot, returning pointer to pivot.
int* partition(int* low, int* high) {

  int* pivot = low; // Selects first element in sublist as pivot
  //low++; // First element in list comes after the pivot element
  // Amount of elements (pivot excluded)
  //int amountOfElements = (high - low + 1);
  //printf("int: %d, high-low: %d\n", sizeof(int), high-low);
  //printf("Pivot: %d, Low: %d, High: %d, Amount of elements: %d\n", *pivot, *low, *high, amountOfElements);
  int* upperLimit = high;
  high++;
  //while(low < high) {
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
