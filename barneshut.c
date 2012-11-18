#include <stdlib.h>
#include "octree.h"
#include "vectors.h"


typedef struct BarnesHutPoint {
  float mass;
  Vector3f center_of_mass;
} BarnesHutPoint;


typedef struct BarnesHut {
  BarnesHutPoint *bodies;
  OctreeNode3f *octree_root;
} BarnesHut;





void BarnesHut_treecalc_leaf(OctreeNode3f *node) {
  // mass already set
  /* ensure the center of mass is set */
  ((BarnesHutPoint*)(node->usr_val))->center_of_mass = *(node->position);
}
void BarnesHut_treecalc_node(OctreeNode3f *node) {
  BarnesHutPoint *pt = (BarnesHutPoint*)(node->usr_val);
  pt->mass = 0;
  pt->center_of_mass.x = 0; pt->center_of_mass.y = 0; pt->center_of_mass.z = 0;
  float child_mass;
  Vector3f child_com;
  for (int i = 0; i < 8; i++) {
    if (node->children[i]) {
      child_com = 
	((BarnesHutPoint*)(node->children[1]->usr_val))->center_of_mass;
      child_mass = ((BarnesHutPoint*)(node->children[i]->usr_val))->mass;
      pt->mass += child_mass;
      pt->center_of_mass.x += child_mass*child_com.x;
      pt->center_of_mass.y += child_mass*child_com.y;
      pt->center_of_mass.z += child_mass*child_com.z;
    }
  }
  pt->center_of_mass.x /= pt->mass;
  pt->center_of_mass.y /= pt->mass;
  pt->center_of_mass.z /= pt->mass;
}






BarnesHut* BarnesHut_malloc(Vector3f bound1, Vector3f bound2) {
  BarnesHut *bh = malloc(sizeof(BarnesHut));
  if (!bh)
    return NULL;
  bh->bodies = malloc(sizeof(BarnesHutPoint)*8);
  if (!(bh->bodies)) {
    free(bh);
    return NULL;
  }
  bh->octree_root = OctreeNode3f_malloc(bound1, bound2);
  if (!(bh->octree_root)) {
    free(bh->bodies);
    free(bh);
    return NULL;
  }
  return bh;
}

void BarnesHut_free(BarnesHut *bh) {
  OctreeNode3f_free(bh->octree_root);
  free(bh->bodies);
  free(bh);
}
