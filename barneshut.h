#ifndef BARNESHUT_H
#define BARNESHUT_H


#include "octree.h"
#include "vectors.h"


/*! \brief A mass/center_of_mass representation
 * A structure that holds the mass and center of mass for some generic thing.
 */
typedef struct BarnesHutPoint {
  float mass;
  Vector3f center_of_mass;
  Vector3f force;
} BarnesHutPoint;


/*! \brief A BarnesHut simulation
 * A 3D simulation of the BarnesHut algorithm, including some number of 
 * massive objects.
 */
typedef struct BarnesHut {
  BarnesHutPoint *bodies;
  unsigned int body_count;
  unsigned int body_cap;
  BarnesHutPoint *fillers;
  unsigned int filler_count;
  unsigned int filler_cap;
  OctreeNode3f *octree_root;
  int finalized;
} BarnesHut;

#endif


/*! \brief Allocs a BarnesHut
 * Allocates and initializes a BarnesHut algorithm.  The bounding area for 
 * this algorithm will be determined by the two control points passed to this 
 * function.
 * \param bound1 A point that defines the bounding cuboid for this algorithm.
 * \param bound2 A point that defines the bounding cuboid for this algorithm.
 * \return A malloced BarnesHut; NULL if there was no room in memory for 
 *         the algorithm.
 */
BarnesHut* BarnesHut_malloc(Vector3f bound1, Vector3f bound2);

/*! \brief Frees a BarnesHut
 * Frees the memory for a BarnesHut.
 * \param node The BarnesHut algorithm instance to free from memory.
 */
void BarnesHut_free(BarnesHut *bh);

/*! \brief Adds an item to the BarnesHut instance
 * Adds an item with a specified position and mass to an instance of the 
 * BarnesHut algorithm.  Duplicate positions WILL be accepted, but may 
 * cause unintended results when calculations are performed.
 * \param bh The BarnesHut instance to add to.
 * \param position The position of the item to add to the BarnesHut.
 * \param mass The mass of the item to add to the BarnesHut.
 * \return 0 if the item could not be added, non-zero otherwise.
 */
int BarnesHut_add(BarnesHut *bh, Vector3f position, float mass);

/*! \brief Finalizes a BarnesHut instance
 * Performs final calculations on the items contained in the BarnesHut.  No 
 * more items may be added to the instance after this.
 * \param bh The BarnesHut instance to finalize.
 */
void BarnesHut_finalize(BarnesHut *bh);

/*! \brief Calculates the force on an item
 * Calculates the force on an item with the specified position and mass.  The 
 * BarnesHut instance MUST be finalized for this function to execute properly.
 * \param bh The BarnesHut instance to calculate force against.
 * \param position The position of the item to be acted upon.
 * \param mass The mass of the item to be acted upon.
 */
Vector3f BarnesHut_force(BarnesHut *bh, Vector3f position, float mass);
