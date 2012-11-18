#ifndef OCTREE_H
#define OCTREE_H


/* A generic 3-point space vector */
typedef struct Vector3f {
  float x; /* x-coord */
  float y; /* y-coord */
  float z; /* z-coord */
} Vector3f;


/* A node of an octree, including its root and leaves */
typedef struct OctreeNode3f {
  /* Stores pointers to each of the 8 possible children of this octree node */
  struct OctreeNode3f* children[8];
  /* Stores the count of leaves (points) contained under this node */
  unsigned int elements;
  /* Stores a pointer to user-defined data (NOT a copy of it) for this node */
  void *usr_val;
  /* A pointer to the position contained by this node IF this is a leaf */
  Vector3f *position;
  /* The minimal coordinate for the bounds covered by this node */
  Vector3f bounds_bot;
  /* The midpoint coordinate for the bounds covered by this node */
  Vector3f bounds_mid;
  /* The minimal coordinate for the bounds covered by this node */
  Vector3f bounds_top;
} OctreeNode3f;


/* Allocates and initializes the memory for a node that is bounded by 
 * the two specified points */
OctreeNode3f* OctreeNode3f_malloc(Vector3f, Vector3f);

/* Frees the memory entirely for the specified node */
void OctreeNode3f_free(OctreeNode3f *node);

/* Insert a position into the tree with the specified root node
 * RETURNS - 0 on failure
 *           Number of elements in the tree rooted at the specified node
 */
int OctreeNode3f_insert(OctreeNode3f *node, Vector3f, void *);

/* Traverses the entire tree rooted at the specified node, calling the 
 * f_node or f_leaf function when appropriate */
int OctreeNode3f_preorder(OctreeNode3f *node, 
			  void (*f_node)(OctreeNode3f*), 
			  void (*f_leaf)(OctreeNode3f*));

/* Traverses the entire tree rooted at the specified node, calling the 
 * f_node or f_leaf function when appropriate */
int OctreeNode3f_postorder(OctreeNode3f *node, 
			   void (*f_node)(OctreeNode3f*), 
			   void (*f_leaf)(OctreeNode3f*));


#endif
