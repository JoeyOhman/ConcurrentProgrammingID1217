#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <windows.h>


#define SHARED 0
#define WORKTIME 4000
#define WCTIME 1000

void* manEnter(void*);
void* womanEnter(void*);
void sleepRandom(int, int*);
void useBathroom(bool, long, int*);
void printEntered(bool, long);
void printLeft(bool, long);
void printQueued(bool, long);
void printNumber(bool);
void printDelayed(bool);
int rand_r(int*);

//int threads = 2;
int amountOfMen = 0, amountOfWomen = 0;
sem_t mutex, men, women, printMutex;

// Keep track of how many in wc and waiting for wc
int numberMen = 0, numberWomen = 0, delayedMen = 0, delayedWomen = 0;

int main(int argc, char* argv[]) {
  if(argc > 1)
    amountOfMen = atoi(argv[1]);
  if(argc > 2)
    amountOfWomen = atoi(argv[2]);

  sem_init(&mutex, SHARED, 1);
  sem_init(&men, SHARED, 0);
  sem_init(&women, SHARED, 0);
  sem_init(&printMutex, SHARED, 1);

  pthread_t men[amountOfMen];
  pthread_t women[amountOfWomen];

  printf("Amount of men: %d\n", amountOfMen);
  printf("Amount of women: %d\n", amountOfWomen);

  long i;
  for(i = 0; i < amountOfMen; i++)
    pthread_create(&men[i], NULL, manEnter, (void*)i);

  for(i = 0; i < amountOfWomen; i++)
    pthread_create(&women[i], NULL, womanEnter, (void*)i);

  pthread_exit(NULL);
}

void* manEnter(void* arg) {
  long id = (long)arg;
  int seed = id;

  while(true) {
    sleepRandom(WORKTIME, &seed); // Work a while

    //printf("Just slept, trying to lock mutex...\n");
    sem_wait(&mutex);
    //printf("Locked mutex!\n");
    // Wait if women inside or women already waiting
    if(numberWomen > 0 || delayedWomen > 0) {
      delayedMen++;
      sem_post(&mutex);
      printQueued(true, id);
      sem_wait(&men);
    }
    numberMen++;
    // Let in delayed men if any (pass mutex)
    if(delayedMen > 0) {
      delayedMen--;
      sem_post(&men);
    } else {
      sem_post(&mutex);
    }

    useBathroom(true, id, &seed); // Use bathroom a while

    sem_wait(&mutex);
    printLeft(true, id);
    numberMen--;
    // Pass the baton (mutex) to delayed woman
    if(numberMen == 0 && delayedWomen > 0) {
      delayedWomen--;
      sem_post(&women);
    } else {
      sem_post(&mutex);
    }

  }
}

void* womanEnter(void* arg) {
  long id = (long)arg;
  int seed = id*100;

  while(true) {
    sleepRandom(WORKTIME, &seed); // Work a while

    //printf("Just slept, trying to lock mutex...\n");
    sem_wait(&mutex);
    //printf("Locked mutex!\n");
    // Wait if men inside or men already waiting
    if(numberMen > 0 || delayedMen > 0) {
      delayedWomen++;
      sem_post(&mutex);
      printQueued(false, id);
      sem_wait(&women);
    }
    numberWomen++;
    // Let in delayed women if any (pass mutex)
    if(delayedWomen > 0) {
      delayedWomen--;
      sem_post(&women);
    } else {
      sem_post(&mutex);
    }

    useBathroom(false, id, &seed); // Use bathroom a while

    sem_wait(&mutex);
    printLeft(false, id);
    numberWomen--;
    // Pass the baton(mutex) to delayed man
    if(numberWomen == 0 && delayedMen > 0) {
      delayedMen--;
      sem_post(&men);
    } else {
      sem_post(&mutex);
    }

  }
}

void sleepRandom(int maxMillis, int* seed) {
  Sleep(rand_r(seed)%maxMillis);
}

void useBathroom(bool man, long id, int* seed) {
  //printNumber(man);
  printEntered(man, id);
  sleepRandom(WCTIME, seed);
  //printLeft(man, id);
}

void printEntered(bool man, long id) {
  sem_wait(&printMutex);
  if(man)
    printf("Man entered, id: %d\n", id);
  else
    printf("Woman entered, id: %d\n", id);
  sem_post(&printMutex);
}

void printLeft(bool man, long id) {
  sem_wait(&printMutex);
  if(man)
    printf("Man left, id: %d\n", id);
  else
    printf("Woman left, id: %d\n", id);
  sem_post(&printMutex);
}

void printQueued(bool man, long id) {
  sem_wait(&printMutex);
  if(man)
    printf("Man queued, id: %d\n", id);
  else
    printf("Woman queued, id: %d\n", id);
  sem_post(&printMutex);
}

void printNumber(bool man) {
  sem_wait(&printMutex);
  if(man)
    printf("#Men in wc: %d\n", numberMen);
  else
    printf("#Women in wc: %d\n", numberWomen);
  sem_post(&printMutex);
}

void printDelayed(bool man) {
  sem_wait(&printMutex);
  if(man)
    printf("#Delayed men: %d\n", delayedMen);
  else
    printf("#Delayed women: %d\n", delayedWomen);
  sem_post(&printMutex);
}

int rand_r(int *seedp) {
    *seedp = *seedp * 1103515245 + 12345 * time(NULL);
    return((unsigned)(*seedp/65536) % 32768);
}
