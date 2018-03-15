#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "header.h"

void initParticles();
void calculateForces(pthread_t*);
void moveBodies();
void testInitParticles();
void printParticlePos(struct particle);
double readTimer();
void* forceWork(void*);

const long SIZE = 500, MASS_MAX = 1e9;
const double DT = 1;
int n, far, numWorkers, particlesPerThread, rest;

double startTime, endTime;
FILE* output;

struct particle *particles;
struct vector *forces;

// A size of 500 and barnes hut distance of 100 reduces force calc time to 10%
int main(int argc, char* argv[]) {
  if(argc == 1) {
    printf("Argument 1: #particles\nArgument 2: #timeSteps\nArgument 3: distance for barnes hut approx\nArgument 4: #threads\n");
    return 0;
  }
  n = argc > 1 ? atoi(argv[1]) : 10;
  int numTicks = argc > 2 ? atoi(argv[2]) : 10;
  far = argc > 3 ? atoi(argv[3]) : 10;
  numWorkers = argc > 4 ? atoi(argv[4]) : 1;

  particlesPerThread = n / numWorkers;
  rest = n % numWorkers;

  pthread_t workerid[numWorkers];

  //output = fopen("data", "w");

  printf("Number of bodies: %d\n", n);
  printf("Number of ticks: %d\n", numTicks);
  printf("Distance for barnes hut: %d\n", far);
  printf("Number of threads: %d\n", numWorkers);

  //fprintf(output, "Particle positions: \n");

  initParticles();
  //testInitParticles();
  double execTime;
  double forceTime = 0, forceStart, forceEnd;
  double buildTime = 0, buildStart, buildEnd;
  double moveTime = 0, moveStart, moveEnd;
  startTime = readTimer();
  for(int i = 0; i < numTicks; i++) {
    buildStart = readTimer();
    newTree(SIZE);
    for(int j = 0; j < n; j++)
      quadTreeInsert(&particles[j]);
    //printf("GON SUMMARIZE REAL SOON\n");
    summarizeTree();
    buildEnd = readTimer();
    buildTime += buildEnd - buildStart;
    //printf("SUMMARIZED BIATCH\n");
    forceStart = readTimer();
    calculateForces(workerid);
    forceEnd = readTimer();
    forceTime += forceEnd - forceStart;
    moveStart = readTimer();
    moveBodies();
    moveEnd = readTimer();
    moveTime += moveEnd - moveStart;
    buildStart = readTimer();
    freeTree();
    buildEnd = readTimer();
    buildTime += buildEnd - buildStart;
  }
  endTime = readTimer();
  execTime = endTime - startTime;

  printf("Move bodies time: %*g%%\n", 10, (moveTime/execTime)*100);
  printf("Tree building time: %*g%%\n", 10, (buildTime/execTime)*100);
  printf("Force calculation time: %*g%%\n", 10, (forceTime/execTime)*100);
  printf("Force calculation time: %*g\n", 10, forceTime);
  printf("Execution time: %g seconds\n", execTime);

  //fclose(output);

}

void initParticles() {
  particles = (particle*)malloc(sizeof(particle) * n);
  forces = (vector*)malloc(sizeof(vector) * n);
  //fprintf(output, "| \t");
  for(int i = 0; i < n; i++) {
    struct vector pos = {(double) (rand() % SIZE), (double) (rand() % SIZE)};
    struct vector vel = ZERO_VECTOR();
    struct vector force = ZERO_VECTOR();
    particles[i].pos = pos;
    particles[i].vel = vel;
    forces[i] = force;
    particles[i].mass = (rand() % (MASS_MAX - 100000)) + 100000;
    //printParticlePos(particles[i]);
  }
  //fprintf(output, "\n");
}

void testInitParticles() {
  n = 2; // DT should be 10
  // Expected particle positions after 1 tick
  // x: 9.952836, y: 10.047164	x: 5.000005, y: 14.999995
  particles = (particle*)malloc(sizeof(particle) * n);
  forces = (vector*)malloc(sizeof(vector) * n);
  /*particles[0].pos.x = 10;
  particles[0].pos.y = 10;
  particles[1].pos.x = 5;
  particles[1].pos.y = 15;*/
  particles[0].pos.x = 69;
  particles[0].pos.y = 32;
  particles[1].pos.x = 93;
  particles[1].pos.y = 30;
  particles[0].vel = particles[1].vel = ZERO_VECTOR();
  forces[0] = forces[1] = ZERO_VECTOR();
  particles[0].mass = MASS_MAX;
  particles[1].mass = MASS_MAX/2;
  //fprintf(output, "| \t");
  printParticlePos(particles[0]);
  printParticlePos(particles[1]);
  //fprintf(output, "\n");
}

void calculateForces(pthread_t *workerid) {
  for (long i = 0; i < numWorkers; i++) {
    pthread_create(&workerid[i], NULL, forceWork, (void *) i);
  }
  for (long i = 0; i < numWorkers; i++) {
    pthread_join(workerid[i], NULL);
  }
}

void* forceWork(void* arg) {
  long i = (long) arg;
  int workLoad = i == numWorkers-1 ? particlesPerThread + rest : particlesPerThread;

  for(int j = particlesPerThread*i; j < particlesPerThread*i+workLoad; j++)
    forces[j] = quadTreeSumForces(&particles[j], far);
}

void moveBodies() {
  struct vector deltav; // dv = f/m * DT
  struct vector deltap; // dp = (v + dv/2) * DT
  //fprintf(output, "| \t");
  for (int i = 0; i < n; i++) {
    //printf("Force x: %lf, Force y: %lf\n", forces[i].x, forces[i].y);
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
    //printParticlePos(particles[i]);
  }
  //fprintf(output, "\n");
}

void printParticlePos(struct particle p) {
  fprintf(output, "x: %*lf,  y: %*lf\t | \t", 10, p.pos.x, 10, p.pos.y);
}

double readTimer() {
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
