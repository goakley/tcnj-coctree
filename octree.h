#ifndef OCTREE_H
#define OCTREE_H

#include "vectors.h"


/* \brief A node of an Octree
 * A node that can represent any node in an Octree.  A node with no children 
 * is a leaf, and has a position attribute associated with it.  Nodes with 
 * children (non-leaves) do not have a position associated with them.
 */
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


/* \brief Mallocs an OctreeNode3f
 * Allocates memory for an OctreeNode3f and initializes its attributes to 
 * their base value.  The bounds of this node is defined by the two arguments 
 * to this function, which define the location and size of the node's 
 * bounding cuboid.
 * \param point1 A point that defines the bounding cuboid for this node.
 * \param point2 A point that defines the bounding cuboid for this node.
 * \return A malloced OctreeNode3f; NULL if there was no room in memory for 
 *         the node.
 */
OctreeNode3f* OctreeNode3f_malloc(Vector3f point1, Vector3f point2);

/* \brief Frees an OctreeNode3f
 * Frees the memory for an OctreeNode3f as well as its children (recursively). 
 * Note that if the node contains a user-defined value, that value is 
 * NOT FREED.
 * \param node The node to free from memory.
 */
void OctreeNode3f_free(OctreeNode3f *node);

/* \brief Inserts a position into a node
 * Inserts a position into the OctreeNode3f.  The user can also specify a 
 * pointer to some data that will be associated with the position being 
 * inserted.
 * \param node The root of the tree in which to insert the poisition.
 * \param position The position to insert into the tree.
 * \param usrval The user value to associate with the specified position.  If 
 *               NULL, no data will be associated.
 * \return The number of elements inside the tree rooted at the specified node.
 */
int OctreeNode3f_insert(OctreeNode3f *node, Vector3f poisition, void *usrval);


#endif
