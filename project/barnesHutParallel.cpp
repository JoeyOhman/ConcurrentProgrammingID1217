#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/times.h>
#include <pthread.h>
#include "header.h"

void initParticles();
void calculateForces(long);
void moveBodies();
double readTimer();
void* simulate(void*);
void start_clock();
long end_clock();
void barrier();

static clock_t st_time, en_time;
static struct tms st_cpu, en_cpu;

const long SIZE = 500, MASS_MAX = 1e9;
const double DT = 1;
int n, numTicks, far, numWorkers, particlesPerThread, rest;

double startTime, endTime;
FILE* output;

struct particle *particles;
struct vector *forces;

pthread_mutex_t barrier_mutex;  // mutex lock for the barrier
pthread_cond_t go;        // condition variable for leaving
int numArrived = 0;       // number who have arrived

// A size of 500 and barnes hut distance of 100 reduces force calc time to 10%
int main(int argc, char* argv[]) {
  if(argc == 1) {
    printf("Argument 1: #particles\nArgument 2: #timeSteps\nArgument 3: distance for barnes hut approx\nArgument 4: #threads\n");
    return 0;
  }
  n = argc > 1 ? atoi(argv[1]) : 10;
  numTicks = argc > 2 ? atoi(argv[2]) : 10;
  far = argc > 3 ? atoi(argv[3]) : 10;
  numWorkers = argc > 4 ? atoi(argv[4]) : 1;

  particlesPerThread = n / numWorkers;
  rest = n % numWorkers;

  pthread_t workerid[numWorkers];
  pthread_mutex_init(&barrier_mutex, NULL);
  pthread_cond_init(&go, NULL);

  printf("Number of bodies: %d\n", n);
  printf("Number of ticks: %d\n", numTicks);
  printf("Distance for barnes hut: %d\n", far);
  printf("Number of threads: %d\n", numWorkers);
  initParticles();

  start_clock();

  for (long i = 0; i < numWorkers; i++) {
    pthread_create(&workerid[i], NULL, simulate, (void *) i);
  }
  for (long i = 0; i < numWorkers; i++) {
    pthread_join(workerid[i], NULL);
  }

  long execTime = end_clock();

  printf("Exec time: %dms\n", execTime);

}

void* simulate(void* arg) {
  long id = (long) arg;

  for(int i = 0; i < numTicks; i++) {
    if(id == 0) {
      newTree(SIZE);
      for(int j = 0; j < n; j++)
        quadTreeInsert(&particles[j]);
      summarizeTree();
    }
    barrier();
    calculateForces(id);
    barrier();
    if(id == 0) {
      moveBodies();
      freeTree();
    }
  }
}

void initParticles() {
  particles = (particle*)malloc(sizeof(particle) * n);
  forces = (vector*)malloc(sizeof(vector) * n);

  for(int i = 0; i < n; i++) {
    struct vector pos = {(double) (rand() % SIZE), (double) (rand() % SIZE)};
    struct vector vel = ZERO_VECTOR();
    struct vector force = ZERO_VECTOR();
    particles[i].pos = pos;
    particles[i].vel = vel;
    forces[i] = force;
    particles[i].mass = (rand() % (MASS_MAX - 100000)) + 100000;
  }
}

void calculateForces(long id) {
  int workLoad = (id == numWorkers-1 ? particlesPerThread + rest : particlesPerThread);
  int startIndex = particlesPerThread*id;
  //printf("Startindex: %d, Endindex: %d\n", startIndex, startIndex+workLoad-1);
  //printf("Workload: %d, id: %d\n", workLoad, id);
  for(int i = startIndex; i < startIndex + workLoad; i++) {
    //printf("Gonna calc force on particle index: %d\n", i);
    forces[i] = quadTreeSumForces(&particles[i], far);
    //printf("Just calced force on particle index: %d\n", i);
  }
  //printf("Done calcing force, id: %d\n", id);
}

void moveBodies() {
  struct vector deltav; // dv = f/m * DT
  struct vector deltap; // dp = (v + dv/2) * DT

  for (int i = 0; i < n; i++) {
    deltav.x = forces[i].x/particles[i].mass * DT;
    deltav.y = forces[i].y/particles[i].mass * DT;
    deltap.x = (particles[i].vel.x + deltav.x/2) * DT;
    deltap.y = (particles[i].vel.y + deltav.y/2) * DT;
    particles[i].vel.x += deltav.x;
    particles[i].vel.y += deltav.y;
    particles[i].pos.x += deltap.x;
    particles[i].pos.y += deltap.y;
    if(particles[i].pos.x > SIZE) { particles[i].pos.x = SIZE; particles[i].vel.x *= -1; }
    if(particles[i].pos.y > SIZE) { particles[i].pos.y = SIZE; particles[i].vel.y *= -1; }
    if(particles[i].pos.x < 0) { particles[i].pos.x = 0; particles[i].vel.x *= -1; }
    if(particles[i].pos.y < 0) { particles[i].pos.y = 0; particles[i].vel.y *= -1; }
    forces[i] = ZERO_VECTOR();
  }
}

void start_clock() {
  st_time = times(&st_cpu);
}

long end_clock() {
  en_time = times(&en_cpu);
  return (long)(en_time - st_time);
}

/* a reusable counter barrier */
void barrier() {
  pthread_mutex_lock(&barrier_mutex);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go); // Signal and continue
  } else
    pthread_cond_wait(&go, &barrier_mutex);
  pthread_mutex_unlock(&barrier_mutex);
}
