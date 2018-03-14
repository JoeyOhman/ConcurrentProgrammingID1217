#ifndef HEADER_H
#define HEADER_H
#include <math.h>

struct vector {
  double x, y;
};
vector ZERO_VECTOR();

struct particle {
  struct vector pos, vel;
  double mass;
};

struct node {
  struct node *ur, *ul, *bl, *br; // order of quadrants
  struct vector pos, centerOfMass; // particlePos relevant if leaf
  int size; // side of square
  bool hasParticle = false, isLeaf = true;
  struct particle p;
  double totalMass = 0;
};

const double G = 6.67e-11;

double calcDistance(struct vector, struct vector);

// body 1 is a particle, body 2 is a node
vector calcForce(struct vector, struct vector, double, double);

void newTree(int);
void insertParticle(node*, struct particle);
void quadTreeInsert(struct particle);
node* findChild(struct node, struct vector);
void initiateChildren(struct node*);
void setCenterOfMasses(struct node*);
vector calcNumeratorCOM(struct node*);
void summarizeTree();
vector quadTreeSumForces(struct particle, int);
vector sumForces(struct particle, struct node*, int);


#endif /* HEADER_H */
