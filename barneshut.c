#include <stdlib.h>
#include <math.h>
#include "barneshut.h"


typedef struct BarnesHutPoint {
  float mass;
  Vector3f center_of_mass;
} BarnesHutPoint;


struct BarnesHut {
  OctreeNode3f *octree_root;
  int finalized;
};


BarnesHut* BarnesHut_malloc(Vector3f bound1, Vector3f bound2) {
  BarnesHut *bh = malloc(sizeof(BarnesHut));
  if (!bh)
    return NULL;
  bh->octree_root = OctreeNode3f_malloc(bound1, bound2);
  if (!(bh->octree_root)) {
    free(bh);
    return NULL;
  }
  bh->finalized = 0;
  return bh;
}


void BarnesHut__free(OctreeNode3f *node) {
  if (!node) return;
  free(node->usr_val);
  BarnesHut__free(node->children[0]);
  BarnesHut__free(node->children[1]);
  BarnesHut__free(node->children[2]);
  BarnesHut__free(node->children[3]);
  BarnesHut__free(node->children[4]);
  BarnesHut__free(node->children[5]);
  BarnesHut__free(node->children[6]);
  BarnesHut__free(node->children[7]);
}
void BarnesHut_free(BarnesHut *bh) {
  BarnesHut__free(bh->octree_root);
  OctreeNode3f_free(bh->octree_root);
  free(bh);
}


int BarnesHut_add(BarnesHut *bh, Vector3f position, float mass) {
  if (!bh) return 0;
  BarnesHutPoint *bhp = malloc(sizeof(BarnesHutPoint));
  if (!bhp) return 0;
  bhp->mass = mass;
  bhp->center_of_mass = position;
  /* Add to the tree */
  OctreeNode3f_insert(bh->octree_root, position, bhp);
  return 1;
}


void BarnesHut__treecalc(OctreeNode3f *node) {
  if (!node) return;
  /* If the node is a leaf, then its mass and COM are simply the mass and COM 
     of its data, which were stored when the data was added */
  if (node->elements == 1)
    return;
  /* The mass and COM can be determined from the mass and COM of the node's
     children */
  else {
    /* The node is an empty non-leaf, so add the custom data to it */
    node->usr_val = malloc(sizeof(BarnesHutPoint));
    BarnesHutPoint *pt = (BarnesHutPoint*)(node->usr_val);
    pt->mass = 0;
    pt->center_of_mass.x=0;pt->center_of_mass.y=0;pt->center_of_mass.z=0;
    for (int i = 0; i < 8; i++) {
      if (!node->children[i])
	continue;
      BarnesHut__treecalc(node->children[i]);
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


Vector3f BarnesHut__force(OctreeNode3f *node, BarnesHutPoint bhp) {
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
    force.x = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.x-bhp.center_of_mass.x)/radius3;
    force.y = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.y-bhp.center_of_mass.y)/radius3;
    force.z = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.center_of_mass.z-bhp.center_of_mass.z)/radius3;
  }
  /* Otherwise, the children of this node must be examined in order to 
     calculate force */
  else {
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
  return force;
}
Vector3f BarnesHut_force(BarnesHut *bh, Vector3f position, float mass) {
  Vector3f zero = {0,0,0};
  if (!bh) return zero;
  BarnesHutPoint pt = {mass, position};
  return BarnesHut__force(bh->octree_root, pt);
}
