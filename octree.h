#ifndef OCTREE_H
#define OCTREE_H


/*! \brief A node of an Octree
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
  ///* A pointer to the position contained by this node IF this is a leaf */
  //Vector3f *position;
  float position_x;
  float position_y;
  float position_z;
  ///* The minimal coordinate for the bounds covered by this node */
  //Vector3f bounds_bot;
  float bound_bot_x;
  float bound_bot_y;
  float bound_bot_z;
  ///* The midpoint coordinate for the bounds covered by this node */
  //Vector3f bounds_mid;
  float bound_mid_x;
  float bound_mid_y;
  float bound_mid_z;
  ///* The minimal coordinate for the bounds covered by this node */
  //Vector3f bounds_top;
  float bound_top_x;
  float bound_top_y;
  float bound_top_z;
} OctreeNode3f;


/*! \brief Mallocs an OctreeNode3f
 * Allocates memory for an OctreeNode3f and initializes its attributes to 
 * their base value.  The bounds of this node is defined by the two arguments 
 * to this function, which define the location and size of the node's 
 * bounding cuboid.
 * \param bound_x1 One of two x-coordinates for the system bounds
 * \param bound_y1 One of two y-coordinates for the system bounds
 * \param bound_z1 One of two z-coordinates for the system bounds
 * \param bound_x2 One of two x-coordinates for the system bounds
 * \param bound_y1 One of two y-coordinates for the system bounds
 * \param bound_z1 One of two z-coordinates for the system bounds
 * \return A malloced OctreeNode3f; NULL if there was no room in memory for 
 *         the node.
 */
OctreeNode3f* OctreeNode3f_malloc(float bound_x1,float bound_y1,float bound_z1,
				  float bound_x2,float bound_y2,float bound_z2);

/*! \brief Frees an OctreeNode3f
 * Frees the memory for an OctreeNode3f as well as its children (recursively). 
 * Note that if the node contains a user-defined value, that value is 
 * NOT FREED.
 * \param node The node to free from memory.
 */
void OctreeNode3f_free(OctreeNode3f *node);

/*! \brief Inserts a position into a node
 * Inserts a position into the OctreeNode3f.  The user can also specify a 
 * pointer to some data that will be associated with the position being 
 * inserted.
 * \param node The root of the tree in which to insert the poisition.
 * \param x The x value of the coordinate to insert into the tree.
 * \param y The y value of the coordinate to insert into the tree.
 * \param z The z value of the coordinate to insert into the tree.
 * \param usrval The user value to associate with the specified position.  If 
 *               NULL, no data will be associated.
 * \return The number of elements inside the tree rooted at the specified node.
 */
int OctreeNode3f_insert(OctreeNode3f *node, float x, float y, float z,
			void *usrval);


#endif
