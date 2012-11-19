#include <stdlib.h>
#include <math.h>
#include "barneshut.h"







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


void BarnesHut__treecalc(OctreeNode3f *node) {
  if (!node) return;
  /* If the node is a leaf, then its mass and COM are simply the mass and COM 
     of its data */
  if (node->elements == 1) {
    ((BarnesHutPoint*)(node->usr_val))->center_of_mass = *(node->position);
  }
  /* The mass and COM can be determined from the mass and COM of the node's
     children */
  else {
    BarnesHutPoint *pt = (BarnesHutPoint*)(node->usr_val);
    pt->mass = 0;
    pt->center_of_mass.x=0;pt->center_of_mass.y=0;pt->center_of_mass.z=0;
    for (int i = 0; i < 8; i++) {
      Vector3f child_com = 
	((BarnesHutPoint*)(node->children[i]->usr_val))->center_of_mass;
      float child_mass = ((BarnesHutPoint*)(node->children[i]->usr_val))->mass;
      pt->mass += child_mass;
      pt->center_of_mass.x += child_mass*child_com.x;
      pt->center_of_mass.y += child_mass*child_com.y;
      pt->center_of_mass.z += child_mass*child_com.z;
    }
    pt->center_of_mass.x /= pt->mass;
    pt->center_of_mass.y /= pt->mass;
    pt->center_of_mass.z /= pt->mass;
  }
}
void BarnesHut_finalize(BarnesHut *bh) {
  if (!bh) return;
  /* Calculate the mass and COM for all the nodes in the octree */
  BarnesHut__treecalc(bh->octree_root);
}


Vector3f BarnesHut_force(OctreeNode3f *node, BarnesHutPoint bhp) {
  Vector3f force = {0,0,0};
  if (!node) return force;
  BarnesHutPoint node_bhp = *(BarnesHutPoint*)(node->usr_val);
  /* Calculate the radius between the node's COM and the point */
  float radius = 
    sqrtf(powf(node_bhp.center_of_mass.x-bhp.center_of_mass.x,2) +
	  powf(node_bhp.center_of_mass.y-bhp.center_of_mass.y,2) +
	  powf(node_bhp.center_of_mass.z-bhp.center_of_mass.z,2));
  /* With a radius of 0, the point is in itself.  As general physics explodes 
     into burning death at this point, we'll return a zero-vector instead */
  if (radius == 0) return force;
  /* Calculate the average width of the node's bounding area 
     (averaging x,y,z)*/
  float width = ((node->bounds_top.x-node->bounds_bot.x) + 
		 (node->bounds_top.y-node->bounds_bot.y) + 
		 (node->bounds_top.z-node->bounds_top.z)) / 3;
  /* If the width/radius ratio is below our constant, then the node is 
     sufficiently far away to be used as an object in force calculations */
  if (width/radius < 0.5) {
    float radius3 = powf(radius,3);
    force.x = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.x-bhp.center_of_mass.x)/radius3;
    force.y = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.y-bhp.center_of_mass.y)/radius3;
    force.z = 6.672e11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.z-bhp.center_of_mass.z)/radius3;
  }
  /* Otherwise, the children of this node must be examined in order to 
     calculate force */
  else {
    for (int i = 0; i < 8; i++) {
      Vector3f child_force = {0,0,0};
      if (node->children[i]) {
	child_force = BarnesHut_force(node->children[i], bhp);
	force.x += child_force.x;
	force.y += child_force.y;
	force.z += child_force.z;
      }
    }
  }
  return force;
}
