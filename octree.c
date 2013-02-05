#include <stdlib.h>
#include "octree.h"

OctreeNode3f* OctreeNode3f_malloc(float bound_x1,float bound_y1,float bound_z1,
				  float bound_x2,float bound_y2,float bound_z2)
{
  OctreeNode3f *node = malloc(sizeof(OctreeNode3f));
  if (!node)
    return NULL;
  /* Calculate the bounds of the node, ensuring that the minimal x, y, and z 
   values are in bound_bot, and the maximal are in bound_top */
  node->bound_bot_x = (bound_x1 < bound_x2 ? bound_x1 : bound_x2);
  node->bound_bot_y = (bound_y1 < bound_y2 ? bound_y1 : bound_y2);
  node->bound_bot_z = (bound_z1 < bound_z2 ? bound_z1 : bound_z2);
  node->bound_top_x = (bound_x1 < bound_x2 ? bound_x2 : bound_x1);
  node->bound_top_y = (bound_y1 < bound_y2 ? bound_y2 : bound_y1);
  node->bound_top_z = (bound_z1 < bound_z2 ? bound_z2 : bound_z1);
  node->bound_mid_x = (bound_x1+bound_x2)/2;
  node->bound_mid_y = (bound_y1+bound_y2)/2;
  node->bound_mid_z = (bound_z1+bound_z2)/2;
  /* Zero-NULL all the other attributes */
  node->position_x = 0;
  node->position_y = 0;
  node->position_z = 0;
  node->children[0] = NULL;
  node->children[1] = NULL;
  node->children[2] = NULL;
  node->children[3] = NULL;
  node->children[4] = NULL;
  node->children[5] = NULL;
  node->children[6] = NULL;
  node->children[7] = NULL;
  node->elements = 0;
  node->usr_val = NULL;
  return node;
}


void OctreeNode3f_free(OctreeNode3f *node) {
  if (!node) return;
  node->usr_val = NULL;
  OctreeNode3f_free(node->children[0]);
  OctreeNode3f_free(node->children[1]);
  OctreeNode3f_free(node->children[2]);
  OctreeNode3f_free(node->children[3]);
  OctreeNode3f_free(node->children[4]);
  OctreeNode3f_free(node->children[5]);
  OctreeNode3f_free(node->children[6]);
  OctreeNode3f_free(node->children[7]);
  free(node);
}


int OctreeNode3f__insert_sub(OctreeNode3f*, float,float,float, void*);

int OctreeNode3f_insert(OctreeNode3f *node, float x, float y, float z,
			void *usr_val)
{
  if (!node) return 0;
  /* if this node is empty, it will be turned into a leaf by placing the 
     data directly inside of it */
  if (node->elements == 0) {
    node->position_x = x;
    node->position_y = y;
    node->position_z = z;
    node->usr_val = usr_val;
  }
  /* handle a node that already contains data */
  else {
    /* If this node is a leaf, take its position and place it in the 
       appropriate child, no longer making this a leaf */
    if (node->elements == 1) {
      OctreeNode3f__insert_sub(node, node->position_x, node->position_y, 
			       node->position_z, node->usr_val);
      node->usr_val = NULL;
    }
    /* Since this node is occupied, recursively add the data to the 
     appropriate child node */
    OctreeNode3f__insert_sub(node, x,y,z, usr_val);
  }
  /* A data point was inserted into this node, therefor the element count 
     must be incremented */
  (node->elements)++;
  return (node->elements);
}
/* *** HELPER ***
 * Inserts a value into the appropriate subnode of the specified node
 * RETURNS - 0 on failure
 *           The new count of elements in the subnode that was added to */
int OctreeNode3f__insert_sub(OctreeNode3f *node, float x, float y, float z,
			     void *usr_val)
{
  if (!node) return 0;
  int sub = 0;
  float min_x, min_y, min_z;
  float max_x, max_y, max_z;
  if (x > node->bound_mid_x) {
    /* Children 0,2,4,8 have positive x-coordinates */
    sub += 1;
    min_x = node->bound_mid_x;
    max_x = node->bound_top_x;
  } else {
    min_x = node->bound_bot_x;
    max_x = node->bound_mid_x;
  }
  if (y > node->bound_mid_y) {
    /* Children 0,1,3,4 have positive y-coordinates */
    sub += 2;
    min_y = node->bound_mid_y;
    max_y = node->bound_top_y;
  } else {
    min_y = node->bound_bot_y;
    max_y = node->bound_mid_y;
  }
  if (z > node->bound_mid_z) {
    /* Children 0,1,2,3 have positive z-coordinates */
    sub += 4;
    min_z = node->bound_mid_z;
    max_z = node->bound_top_z;
  } else {
    min_z = node->bound_bot_z;
    max_z = node->bound_mid_z;
  }
  if (!(node->children)[sub])
    (node->children)[sub] = OctreeNode3f_malloc(min_x, min_y, min_z,
						max_x, max_y, max_z);
  // (the next line naturally checks for a successful malloc)
  return OctreeNode3f_insert((node->children)[sub], x,y,z, usr_val);
}
