#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/times.h>
#include <pthread.h>
#include "header.h"

void initParticles();
void allocMem();
void calculateForces(long);
void moveBodies(long);
void testInitParticles();
void printParticlePos(struct particle);
void barrier();
double readTimer();
void* Worker(void*);
void start_clock();
void end_clock();

static clock_t st_time, en_time;
static struct tms st_cpu, en_cpu;

struct particle *particles;
struct vector** forces;

struct vector ZERO_VECTOR = {0, 0};
const double G = 6.67e-11;
const long SIZE = 100, MASS_MAX = 1e9;
const int DT = 10;
int n, numWorkers, numTicks;

FILE* output;

pthread_mutex_t barrier_mutex;  // mutex lock for the barrier
pthread_cond_t go;        // condition variable for leaving
int numArrived = 0;       // number who have arrived

int main(int argc, char* argv[]) {
  if(argc == 1) {
    printf("Argument 1: #particles\nArgument 2: #timeSteps\nArgument 3: #threads");
    return 0;
  }
  n = argc > 1 ? atoi(argv[1]) : 10;
  numTicks = argc > 2 ? atoi(argv[2]) : 10;
  numWorkers = argc > 3 ? atoi(argv[3]) : 1;
  if(numWorkers > n)
    numWorkers = n;

  pthread_t workerid[numWorkers];
  pthread_mutex_init(&barrier_mutex, NULL);
  pthread_cond_init(&go, NULL);

  output = fopen("data", "w");

  printf("Number of bodies: %d\n", n);
  printf("Number of ticks: %d\n", numTicks);
  printf("Number of threads: %d\n", numWorkers);

  fprintf(output, "Particle positions: \n");

  allocMem();
  //printf("Allocated memory\n");
  initParticles();
  //testInitParticles();

  //printf("Initiated particles\n");

  start_clock();
  long l;
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], NULL, Worker, (void *) l);

  //printf("Initiated threads\n");
  for(l = 0; l < numWorkers; l++)
    pthread_join(workerid[l], NULL);

  //printf("Joined threads\n");
  end_clock();

  fclose(output);

}

void* Worker(void* arg) {
  long id = (long) arg;
  for(int i = 0; i < numTicks; i++) {
    calculateForces(id);
    //printf("Calculated forces!\n");
    barrier();
    //printf("Barrier done");
    moveBodies(id);
    //printf("Moved bodies!\n");
    if(id == 0) {
      for(int j = 0; j < n; j++)
        printParticlePos(particles[j]);
      fprintf(output, "\n");
    }
    barrier();
  }
}

void initParticles() {
  for(int i = 0; i < n; i++) {
    struct vector pos = {(double) (rand() % SIZE), (double) (rand() % SIZE)};
    particles[i].pos = pos;
    particles[i].vel = ZERO_VECTOR;
    for(int w = 0; w < numWorkers; w++)
      forces[w][i] = ZERO_VECTOR;
    particles[i].mass = (rand() % MASS_MAX) + 1;
    printParticlePos(particles[i]);
  }
  fprintf(output, "\n");
}

void testInitParticles() {
  n = 2; // DT should be 10
  // Expected particle positions after 1 tick
  // x: 9.952836, y: 10.047164	x: 5.000005, y: 14.999995
  particles[0].pos.x = 10;
  particles[0].pos.y = 10;
  particles[1].pos.x = 5;
  particles[1].pos.y = 15;
  particles[0].vel = particles[1].vel = ZERO_VECTOR;
  for(int w = 0; w < numWorkers; w++) {
    forces[w][0] = ZERO_VECTOR;
    forces[w][1] = ZERO_VECTOR;
  }
  particles[0].mass = 1e5;
  particles[1].mass = MASS_MAX;
  printParticlePos(particles[0]);
  printParticlePos(particles[1]);
  fprintf(output, "\n");
}

void allocMem() {
  particles = (particle*) malloc(sizeof(particle) * n);

  forces = (vector **) malloc(sizeof(vector*) * numWorkers); // Allocate pointers
  for(int i = 0; i < n; i++)
    forces[i] = (vector*) malloc(sizeof(vector) * n); // Allocate arrays which the pointers point to
}

void calculateForces(long w) {
  double distance, magnitude;
  struct vector direction;
  for (long i = w; i < n-1; i+=numWorkers) {
    for (long j = i+1; j < n; j++) {
      distance = sqrt( pow(particles[i].pos.x - particles[j].pos.x, 2)
      + pow(particles[i].pos.y - particles[j].pos.y, 2));
      if(distance == 0)
        distance = 1;
      magnitude = (G*particles[i].mass*particles[j].mass) / pow(distance, 2);
      direction.x = particles[j].pos.x-particles[i].pos.x;
      direction.y = particles[j].pos.y-particles[i].pos.y;
      forces[w][i].x += magnitude*direction.x/distance;
      forces[w][j].x -= magnitude*direction.x/distance;
      forces[w][i].y += magnitude*direction.y/distance;
      forces[w][j].y -= magnitude*direction.y/distance;
    }
  }
}

void moveBodies(long w) {
  struct vector deltav; // dv = f/m * DT
  struct vector deltap; // dp = (v + dv/2) * DT
  for(long i = w; i < n; i += numWorkers) {
    struct vector forceSum = {0, 0};
    for(long k = 0; k < numWorkers; k++) { // Sum forces into forceSum and reset forces
      forceSum.x += forces[k][i].x;
      forceSum.y += forces[k][i].y;
      forces[k][i] = ZERO_VECTOR;
    }

    deltav.x = forceSum.x/particles[i].mass * DT;
    deltav.y = forceSum.y/particles[i].mass * DT;
    deltap.x = (particles[i].vel.x + deltav.x/2) * DT;
    deltap.y = (particles[i].vel.y + deltav.y/2) * DT;
    particles[i].vel.x += deltav.x;
    particles[i].vel.y += deltav.y;
    particles[i].pos.x += deltap.x;
    particles[i].pos.y += deltap.y;
  }
}

void printParticlePos(struct particle p) {
  fprintf(output, "x: %lf, y: %lf\t", p.pos.x, p.pos.y);
}

/* a reusable counter barrier */
void barrier() {
  pthread_mutex_lock(&barrier_mutex);
  numArrived++;
  //printf("num arrived: %d of total: %d\n", numArrived, numWorkers);
  if (numArrived == numWorkers) {
    numArrived = 0;
    //printf("All arrived, broadcasting\n");
    pthread_cond_broadcast(&go); // Signal and continue
    //printf("All arrived, broadcasted\n");
  } else
    pthread_cond_wait(&go, &barrier_mutex);
  pthread_mutex_unlock(&barrier_mutex);
}

void start_clock() {
  st_time = times(&st_cpu);
}

void end_clock() {
  en_time = times(&en_cpu);
  printf("Real Time: %jdms", (intmax_t)(en_time - st_time));

}
