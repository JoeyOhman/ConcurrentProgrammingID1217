#ifndef HEADER_H
#define HEADER_H
#include <math.h>
#include <stdbool.h>

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
  double size; // side of square
  bool hasParticle, isLeaf;
  struct particle *p;
  double totalMass;
};

const double G = 6.67e-11;

double calcDistance(struct vector, struct vector);

// body 1 is a particle, body 2 is a node or a particle
vector calcForce(struct vector, struct vector, double, double);

void newTree(int);
void insertParticle(struct node*, struct particle*);
void quadTreeInsert(struct particle*);
node* findChild(struct node*, struct vector);
void initiateChildren(struct node*);
void setCenterOfMasses(struct node*);
vector calcNumeratorCOM(struct node*);
void summarizeTree();
vector quadTreeSumForces(struct particle*, int);
vector sumForces(struct particle*, struct node*, int);
void initiateAttr(struct node*);
void freeTree();
void freeTree(struct node*);


#endif /* HEADER_H */
