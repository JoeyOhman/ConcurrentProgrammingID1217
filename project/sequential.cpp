#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

void initParticles();
void calculateForces();
void moveBodies();
void testInitParticles();
void printParticlePos(struct particle);
double readTimer();

struct vector {
  double x, y;
};

struct particle {
  struct vector pos, vel, force;
  double mass;
};

const double G = 6.67e-11;
const long SIZE = 100, MASS_MAX = 1e9;
const int DT = 10;
int n;

double startTime, endTime;
FILE* output;

struct particle *particles;

int main(int argc, char* argv[]) {
  n = argc > 1 ? atoi(argv[1]) : 10;
  int numTicks = argc > 2 ? atoi(argv[2]) : 10;

  output = fopen("data", "w");

  printf("Number of bodies: %d\n", n);
  printf("Number of ticks: %d\n", numTicks);

  fprintf(output, "Particle positions: \n");

  initParticles();
  //testInitParticles();

  startTime = readTimer();
  for(int i = 0; i < numTicks; i++) {
    calculateForces();
    moveBodies();
  }
  endTime = readTimer();

  printf("Execution time: %g\n", endTime - startTime);

  fclose(output);

}

void initParticles() {
  particles = (particle*)malloc(sizeof(particle) * n);
  for(int i = 0; i < n; i++) {
    struct vector pos = {rand() % SIZE, rand() % SIZE};
    struct vector vel = {0, 0};
    struct vector force = {0, 0};
    particles[i].pos = pos;
    particles[i].vel = vel;
    particles[i].force = force;
    particles[i].mass = (rand() % MASS_MAX) + 1;
    printParticlePos(particles[i]);
  }
  fprintf(output, "\n");
}

void testInitParticles() {
  n = 2; // DT should be 10
  // Expected particle positions after 1 tick
  // x: 9.952836, y: 10.047164	x: 5.000005, y: 14.999995
  particles = (particle*)malloc(sizeof(particle) * n);
  struct vector zero = {0, 0};
  particles[0].pos.x = 10;
  particles[0].pos.y = 10;
  particles[1].pos.x = 5;
  particles[1].pos.y = 15;
  particles[0].vel = particles[1].vel = zero;
  particles[0].force = particles[1].force = zero;
  particles[0].mass = 1e5;
  particles[1].mass = MASS_MAX;
}

void calculateForces() {
  double distance, magnitude;
  struct vector direction;
  for (int i = 0; i < n-1; i++) {
    for (int j = i+1; j < n; j++) {
      distance = sqrt( pow(particles[i].pos.x - particles[j].pos.x, 2)
      + pow(particles[i].pos.y - particles[j].pos.y, 2));
      if(distance < 1)
        distance = 1;
      magnitude = (G*particles[i].mass*particles[j].mass) / pow(distance, 2);
      direction.x = particles[j].pos.x-particles[i].pos.x;
      direction.y = particles[j].pos.y-particles[i].pos.y;
      particles[i].force.x += magnitude*direction.x/distance;
      particles[j].force.x -= magnitude*direction.x/distance;
      particles[i].force.y += magnitude*direction.y/distance;
      particles[j].force.y -= magnitude*direction.y/distance;
    }
  }
}

void moveBodies() {
  struct vector deltav; // dv = f/m * DT
  struct vector deltap; // dp = (v + dv/2) * DT
  for (int i = 0; i < n; i++) {
    deltav.x = particles[i].force.x/particles[i].mass * DT;
    deltav.y = particles[i].force.y/particles[i].mass * DT;
    deltap.x = (particles[i].vel.x + deltav.x/2) * DT;
    deltap.y = (particles[i].vel.y + deltav.y/2) * DT;
    particles[i].vel.x += deltav.x;
    particles[i].vel.y += deltav.y;
    particles[i].pos.x += deltap.x;
    particles[i].pos.y += deltap.y;
    particles[i].force.x = particles[i].force.y = 0.0; // reset force vector
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
