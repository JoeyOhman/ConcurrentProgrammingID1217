#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <iostream>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

#ifndef numberOfThreads
#define numberOfThreads 10
#endif

void *tf (void *threadid) {
  long tid = (long)threadid;
  int x = rand() % 10000000;
  cout << x << endl;
  for(int i = 0; i < tid*10000; i++) {
    for(int j = 0; j < 1000; j++);
  }
  cout << "threadId: " << tid << " done!" << endl;
  pthread_exit(NULL);
}


int main() {
  cout << "Hello" << " World!!!!!" << endl;

  int n = numberOfThreads;

  pthread_t threads[n];

  int i;
  for (i = 0; i < n; i++) {
    pthread_create(&threads[i], NULL, tf, (void *)i);
  }

  /*
  int rc = pthread_create(&thread, NULL, tf, (void *)x);

  if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
  } else {
    cout << "Thread created!" << endl;
  }

  */

  pthread_exit(NULL);
}



/*
#include <iostream>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 5

void *PrintHello(void *threadid) {
   long tid;
   tid = (long)threadid;
   cout << "Hello World! Thread ID, " << tid << endl;
   pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   int rc;
   int i;

   for( i = 0; i < NUM_THREADS; i++ ) {
      cout << "main() : creating thread, " << i << endl;
      rc = pthread_create(&threads[i], NULL, PrintHello, (void *)i);

      if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   pthread_exit(NULL);
}
*/
