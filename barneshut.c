#include <stdlib.h>
#include <math.h>
#include "barneshut.h"




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
  bh->body_cap = 8;
  bh->bodies = malloc(sizeof(BarnesHutPoint));
  if (!(bh->bodies)) {
    free(bh);
    return NULL;
  }
  bh->body_count = 0;
  bh->octree_root = OctreeNode3f_malloc(bound1, bound2);
  if (!(bh->octree_root)) {
    free(bh->bodies);
    free(bh);
    return NULL;
  }
  bh->finalized = 0;
  return bh;
}

void BarnesHut_free(BarnesHut *bh) {
  OctreeNode3f_free(bh->octree_root);
  free(bh->bodies);
  free(bh);
}


int BarnesHut_add(BarnesHut *bh, Vector3f position, float mass) {
  if (!bh) return 0;
  if ((bh->body_count) == (bh->body_cap)) {
    unsigned int test_cap = (bh->body_cap)*2;
    BarnesHutPoint *temp = NULL;
    /* repeadedly attempt to make room for new bodies (asking for less room
       each time */
    while (test_cap > 2) {
      temp = realloc(bh->bodies, sizeof(BarnesHutPoint)*test_cap);
      if (temp) {
	bh->bodies = temp;
	break;
      }
      test_cap /= 2;
    }
    /* return 0 if unable to allocate more room for bodies */
    if (!temp)
      return 0;
    bh->body_cap = test_cap;
  }
  (bh->bodies)[bh->body_count].mass = mass;
  // center of mass will be set when finalizing the tree; not setting now
  Vector3f zerovector = {0,0,0};
  (bh->bodies)[bh->body_count].force = zerovector;
  bh->body_count++;
  return 1;
}


Vector3f BarnesHut__force(OctreeNode3f *node, BarnesHutPoint bhp);
int BarnesHut_finalize(BarnesHut *bh) {
  if (!bh) return 0;
  /* finalize the octree */
  int finalized = OctreeNode3f_postorder(bh->octree_root, 
					 &BarnesHut_treecalc_node, 
					 &BarnesHut_treecalc_leaf);
  if (!finalized)
    return 0;
  /* calculate the forces */
  for (int i = 0; i < bh->body_count; i++) {
    bh->bodies[i].force = BarnesHut__force(bh->octree_root, 
					   bh->bodies[i]);
  }
  return 1;
}

Vector3f BarnesHut__force(OctreeNode3f *node, BarnesHutPoint bhp) {
  Vector3f force = {0,0,0};
  if (!node) return force;
  BarnesHutPoint node_bhp = *(BarnesHutPoint*)(node->usr_val);
  float radius = 
    sqrtf(powf(node_bhp.center_of_mass.x-bhp.center_of_mass.x,2) +
	  powf(node_bhp.center_of_mass.y-bhp.center_of_mass.y,2) +
	  powf(node_bhp.center_of_mass.z-bhp.center_of_mass.z,2));
  if (radius > 1.175494351e-38F) {
    float radius3 = powf(radius,3);
    force.x = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.x-bhp.center_of_mass.x)/radius3;
    force.y = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.y-bhp.center_of_mass.y)/radius3;
    force.z = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.z-bhp.center_of_mass.z)/radius3;
  }
  if (node->elements > 1) {
    float widthy = ((node->bounds_top.x-node->bounds_bot.x) + 
		    (node->bounds_top.y-node->bounds_bot.y) + 
		    (node->bounds_top.z-node->bounds_top.z)) / 3;
    if (widthy/radius > 0.5) {
      force.x = 0; force.y = 0; force.z = 0;
      for (int i = 0; i < 8; i++) {
	Vector3f child_force = {0,0,0};
	if (node->children[i]) {
	  child_force = BarnesHut__force(node->children[i], bhp);
	  force.x += child_force.x;
	  force.y += child_force.y;
	  force.z += child_force.z;
	}
      }
    }
  }
  return force;
}
