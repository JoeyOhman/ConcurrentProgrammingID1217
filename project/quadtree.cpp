#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "header.h"

struct tree {
  struct node* root;
} quadTree;

void newTree(int worldSize) {
  struct node *root = (node*)malloc(sizeof(node));
  quadTree.root = root;
  quadTree.root->size = worldSize;
  quadTree.root->pos = ZERO_VECTOR();
}

void summarizeTree() {
  setCenterOfMasses(quadTree.root);
}

void quadTreeInsert(struct particle p) {
  insertParticle(quadTree.root, p);
}

void insertParticle(struct node *n, struct particle p) {
  n->totalMass += p.mass;
  // Base case
  if(!n->hasParticle && n->isLeaf) { // Insert particle
    n->hasParticle = true;
    n->p = p;

  } else if(n->hasParticle && !n->isLeaf) { // Recurr into child (keep looking)
    struct node *child = findChild(*n, p.pos);
    insertParticle(child, p);

  } else if(n->hasParticle && n->isLeaf) { // Initiate children, insert already existing particle and then insert new recursively
    n->isLeaf = false;
    initiateChildren(n);
    struct node *oldParticleChild = findChild(*n, n->p.pos);
    oldParticleChild->hasParticle = true;
    insertParticle(oldParticleChild, n->p); // Will always hit base case
    struct node *newParticleChild = findChild(*n, p.pos);
    newParticleChild->hasParticle = true;
    insertParticle(newParticleChild, p);
  }

}

// Finds the child node which the position belongs in
node* findChild(struct node parent, struct vector pos) {
  if(pos.x > parent.size/2) {
    if(pos.y > parent.size/2)
      return parent.ur;
    else
      return parent.br;
  } else {
    if(pos.y > parent.size/2)
      return parent.ul;
    else
      return parent.bl;
  }
}

// Initiates sizes and positions of children nodes
void initiateChildren(struct node *parent) {
  parent->ur = (node*)malloc(sizeof(node));
  parent->ul = (node*)malloc(sizeof(node));
  parent->bl = (node*)malloc(sizeof(node));
  parent->br = (node*)malloc(sizeof(node));

  int childSize = parent->size/2;
  parent->ur->size = parent->ul->size = parent->br->size = parent->bl->size = childSize;

  parent->ur->pos.x = parent->pos.x + childSize;
  parent->ur->pos.y = parent->pos.y + childSize;

  parent->ul->pos.x = parent->pos.x;
  parent->ul->pos.y = parent->pos.y + childSize;

  parent->bl->pos = parent->pos;

  parent->br->pos.x = parent->pos.x + childSize;
  parent->br->pos.y = parent->pos.y;
}

void setCenterOfMasses(struct node* n) {
  // Only set center of mass on internal nodes
  if(n->isLeaf)
    return;

  struct vector v = calcNumeratorCOM(n);
  struct vector res = ZERO_VECTOR();
  res.x = v.x / n->totalMass;
  res.y = v.y / n->totalMass;
  n->centerOfMass = res;

  setCenterOfMasses(n->ur);
  setCenterOfMasses(n->ul);
  setCenterOfMasses(n->bl);
  setCenterOfMasses(n->br);
}

vector calcNumeratorCOM(struct node* n) {
  struct vector v;
  if(n->isLeaf && !n->hasParticle) {
    v = ZERO_VECTOR();

  } else if(n->isLeaf && n->hasParticle) {
    v = {n->p.mass*n->p.pos.x, n->p.mass*n->p.pos.y};

  } else {
    struct vector v1, v2, v3, v4;
    v1 = calcNumeratorCOM(n->ur);
    v2 = calcNumeratorCOM(n->ul);
    v3 = calcNumeratorCOM(n->bl);
    v4 = calcNumeratorCOM(n->br);
    v = {v1.x + v2.x + v3.x + v4.x, v1.y + v2.y + v3.y + v4.y};
  }

  return v;
}

vector quadTreeSumForces(struct particle p, int far) {
  sumForces(p, quadTree.root, far);
}

vector sumForces(struct particle p, struct node *n, int far) {
  struct vector force = ZERO_VECTOR(), v1, v2, v3, v4;
  if(calcDistance(p.pos, n->centerOfMass) < far) {
    if(!n->isLeaf) {
      v1 = sumForces(p, n->ur, far);
      v2 = sumForces(p, n->ul, far);
      v3 = sumForces(p, n->bl, far);
      v4 = sumForces(p, n->br, far);
      force = {v1.x + v2.x + v3.x + v4.x, v1.y + v2.y + v3.y + v4.y};
    } else if(n->hasParticle) {
      force = calcForce(p.pos, n->p.pos, p.mass, n->p.mass);
    }
  } else {
    force = calcForce(p.pos, n->centerOfMass, p.mass, n->totalMass);
  }
  return force;
}
