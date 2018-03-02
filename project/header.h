struct vector {
  double x, y;
};

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

void newTree(int);
void insertParticle(node*, struct particle);
void quadTreeInsert(struct particle);
node* findChild(struct node, struct vector);
void initiateChildren(struct node*);
void setCenterOfMasses(struct node*);
vector calcNumeratorCOM(struct node*);
void summarizeTree();
