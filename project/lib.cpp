#include "header.h"

vector ZERO_VECTOR() {
  struct vector v = {0.0, 0.0};
  return v;
}

double calcDistance(struct vector pos1, struct vector pos2) {
  return sqrt(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2));
}

// body 1 is a particle, body 2 is a node
vector calcForce(struct vector pos1, struct vector pos2, double mass1, double mass2) {
  struct vector force, direction;
  double distance = calcDistance(pos1, pos2);
  if(distance < 1)
    distance = 1;
  double magnitude = (G*mass1*mass2) / pow(distance, 2);
  direction.x = pos2.x-pos1.x;
  direction.y = pos2.y-pos1.y;
  force.x = magnitude*direction.x/distance;
  force.y = magnitude*direction.y/distance;
  return force;
}
