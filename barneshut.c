#include <stdlib.h>
#include <math.h>
#include "barneshut.h"

typedef struct BarnesHutPoint {
  float mass;
  float com_x;
  float com_y;
  float com_z;
} BarnesHutPoint;


struct BarnesHut {
  OctreeNode3f *octree_root;
  int finalized;
};


BarnesHut* BarnesHut_malloc(float minx, float miny, float minz,
			    float maxx, float maxy, float maxz) {
  BarnesHut *bh = malloc(sizeof(BarnesHut));
  if (!bh)
    return NULL;
  bh->octree_root = OctreeNode3f_malloc(minx,miny,minz,maxx,maxy,maxz);
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


int BarnesHut_add(BarnesHut *bh, float x, float y, float z, float mass) {
  if (!bh) return 0;
  BarnesHutPoint *bhp = malloc(sizeof(BarnesHutPoint));
  if (!bhp) return 0;
  bhp->mass = mass;
  bhp->com_x = x;
  bhp->com_y = y;
  bhp->com_z = z;
  /* Add to the tree */
  OctreeNode3f_insert(bh->octree_root, x,y,z, bhp);
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
    pt->com_x = 0; pt->com_y = 0; pt->com_z = 0;
    for (int i = 0; i < 8; i++) {
      if (!node->children[i])
	continue;
      BarnesHut__treecalc(node->children[i]);
      BarnesHutPoint *child_pt = (BarnesHutPoint*)node->children[i]->usr_val;
      float child_mass = child_pt->mass;
      pt->mass += child_mass;
      pt->com_x += child_mass*(child_pt->com_x);
      pt->com_y += child_mass*(child_pt->com_y);
      pt->com_z += child_mass*(child_pt->com_z);
    }
    pt->com_x /= pt->mass;
    pt->com_y /= pt->mass;
    pt->com_z /= pt->mass;
  }
}
void BarnesHut_finalize(BarnesHut *bh) {
  if (!bh) return;
  /* Calculate the mass and COM for all the nodes in the octree */
  BarnesHut__treecalc(bh->octree_root);
}


int BarnesHut__force(OctreeNode3f *node, BarnesHutPoint bhp,
		     float *retx, float *rety, float *retz) {
  if (!node) return 0;
  *retx = *rety = *retz = 0;
  BarnesHutPoint node_bhp = *(BarnesHutPoint*)(node->usr_val);
  /* Calculate the radius between the node's COM and the point */
  float radius = 
    sqrtf(powf(node_bhp.com_x-bhp.com_x,2) +
	  powf(node_bhp.com_y-bhp.com_y,2) +
	  powf(node_bhp.com_z-bhp.com_z,2));
  /* With a radius of 0, the point is in itself.  As general physics explodes 
     into burning death at this point, we'll return a zero-vector instead */
  if (radius == 0) return 1;
  /* Calculate the average width of the node's bounding area 
     (averaging x,y,z)*/
  float width = ((node->bound_top_x-node->bound_bot_x) + 
		 (node->bound_top_y-node->bound_bot_y) + 
		 (node->bound_top_z-node->bound_bot_z)) / 3;
  /* If the width/radius ratio is below our constant, then the node is 
     sufficiently far away to be used as an object in force calculations */
  if (width/radius < 0.5) {
    float radius3 = powf(radius,3);
    *retx = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.com_x-bhp.com_x)/radius3;
    *rety = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.com_y-bhp.com_y)/radius3;
    *retz = 6.672e-11F * node_bhp.mass * bhp.mass * 
      (node_bhp.com_z-bhp.com_z)/radius3;
  }
  /* Otherwise, the children of this node must be examined in order to 
     calculate force */
  else {
    for (int i = 0; i < 8; i++) {
      float child_x = 0; float child_y = 0; float child_z = 0;
      if (node->children[i]) {
	BarnesHut__force(node->children[i], bhp, &child_x,&child_y,&child_z);
	*retx += child_x;
	*rety += child_y;
	*retz += child_z;
      }
    }
  }
  return 1;
}
int BarnesHut_force(BarnesHut *bh, float x, float y, float z, float mass,
		    float *retx, float *rety, float *retz) {
  if (!bh) return 0;
  BarnesHutPoint pt = {mass, x,y,z};
  return BarnesHut__force(bh->octree_root, pt, retx,rety,retz);
}
