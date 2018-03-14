#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "header.h"

void initParticles();
void calculateForces();
void moveBodies();
void testInitParticles();
void printParticlePos(struct particle);
double readTimer();

const long SIZE = 100, MASS_MAX = 1e9;
const double DT = 1000000000;
int n, far;

double startTime, endTime;
FILE* output;

struct particle *particles;
struct vector *forces;

int main(int argc, char* argv[]) {
  if(argc == 1) {
    printf("Argument 1: #particles\nArgument 2: #timeSteps\nArgument 3: distance for barnes hut approx");
    return 0;
  }
  n = argc > 1 ? atoi(argv[1]) : 10;
  int numTicks = argc > 2 ? atoi(argv[2]) : 10;
  far = argc > 3 ? atoi(argv[3]) : 10;

  output = fopen("data", "w");

  printf("Number of bodies: %d\n", n);
  printf("Number of ticks: %d\n", numTicks);
  printf("Distance for barnes hut: %d\n", far);

  fprintf(output, "Particle positions: \n");

  //initParticles();
  testInitParticles();

  startTime = readTimer();
  for(int i = 0; i < numTicks; i++) {
    newTree(SIZE);
    for(int j = 0; j < n; j++)
      quadTreeInsert(&particles[j]);
    printf("GON SUMMARIZE REAL SOON\n");
    summarizeTree();
    printf("SUMMARIZED BIATCH\n");
    calculateForces();
    moveBodies();
    freeTree();
  }
  endTime = readTimer();

  printf("Execution time: %g\n", endTime - startTime);

  fclose(output);

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
    printParticlePos(particles[i]);
  }
  fprintf(output, "\n");
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
}

void calculateForces() {
  for (int i = 0; i < n; i++) {
    forces[i] = quadTreeSumForces(&particles[i], far);
  }
}

void moveBodies() {
  struct vector deltav; // dv = f/m * DT
  struct vector deltap; // dp = (v + dv/2) * DT
  for (int i = 0; i < n; i++) {
    printf("Force x: %lf, Force y: %lf\n", forces[i].x, forces[i].y);
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
    printParticlePos(particles[i]);
  }
  fprintf(output, "\n");
}

void printParticlePos(struct particle p) {
  fprintf(output, "x: %lf, y: %lf\t", p.pos.x, p.pos.y);
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
