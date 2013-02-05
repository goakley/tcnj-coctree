#ifndef BARNESHUT_H
#define BARNESHUT_H


#include "octree.h"


/*! \brief A BarnesHut simulation
 * A 3D simulation of the BarnesHut algorithm, including some number of 
 * massive objects.
 */
typedef struct BarnesHut BarnesHut;


/*! \brief Allocs a BarnesHut
 * Allocates and initializes a BarnesHut algorithm.  The bounding area for 
 * this algorithm will be determined by the two control points passed to this 
 * function.
 * \return A malloced BarnesHut; NULL if there was no room in memory for 
 *         the algorithm.
 */
BarnesHut* BarnesHut_malloc(float minx, float miny, float minz,
			    float maxx, float maxy, float maxz);

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
 * \param x The x position of the item to add to the BarnesHut.
 * \param y The y position of the item to add to the BarnesHut.
 * \param z The z position of the item to add to the BarnesHut.
 * \param mass The mass of the item to add to the BarnesHut.
 * \return 0 if the item could not be added, non-zero otherwise.
 */
int BarnesHut_add(BarnesHut *bh, float x, float y, float z, float mass);

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
 * \param x The x position of the item to be acted upon.
 * \param y The y position of the item to be acted upon.
 * \param z The z position of the item to be acted upon.
 * \param retx The force in the x direction that is being applied to the item
 * \param rety The force in the y direction that is being applied to the item
 * \param retz The force in the z direction that is being applied to the item
 * \param mass The mass of the item to be acted upon.
 */
int BarnesHut_force(BarnesHut *bh, float x, float y, float z, float mass,
		    float *retx, float *rety, float *retz);

#endif
