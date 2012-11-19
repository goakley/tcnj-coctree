#ifndef BARNESHUT_H
#define BARNESHUT_H


#include "octree.h"
#include "vectors.h"


/* \brief A mass/center_of_mass representation
 * A structure that holds the mass and center of mass for some generic thing.
 */
typedef struct BarnesHutPoint {
  float mass;
  Vector3f center_of_mass;
  Vector3f force;
} BarnesHutPoint;


/* \brief A BarnesHut simulation
 * A 3D simulation of the BarnesHut algorithm, including some number of 
 * massive objects.
 */
typedef struct BarnesHut {
  BarnesHutPoint *bodies;
  unsigned int body_count;
  unsigned int body_cap;
  OctreeNode3f *octree_root;
  int finalized;
} BarnesHut;

#endif
