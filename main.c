#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include "barneshut.h"

#define POINTCNT 8


typedef struct {
  float mass;
  Vector3f position;
  Vector3f velocity;
  Vector3f acceleration;
  Vector3f force;
} Point;


Point points[POINTCNT];
BarnesHut *bh;


void init();
void draw();
void update();


int main(int argc, char **argv) {
  init();
  update();
  return 0;
}


void init() {
  bh = NULL;
  for (int i = 0; i < POINTCNT; i++) {
    points[i].mass = 1000.0f;
    points[i].position.x = rand()%200-100;
    points[i].position.y = rand()%200-100;
    points[i].position.z = rand()%200-100;
  }
}

void update() {
  Vector3f min = {-100,-100,-100};
  Vector3f max = {100,100,100};
  bh = BarnesHut_malloc(min,max);
  for (int i = 0; i < POINTCNT; i++)
    BarnesHut_add(bh, points[i].position, points[i].mass);
  puts("UPDATING");
  BarnesHut_finalize(bh);
  for (int i = 0; i < POINTCNT; i++) {
    points[i].force = BarnesHut_force(bh, points[i].position, points[i].mass);
    points[i].acceleration.x = points[i].force.x/points[i].mass;
    points[i].acceleration.y = points[i].force.y/points[i].mass;
    points[i].acceleration.z = points[i].force.z/points[i].mass;
    points[i].velocity.x += points[i].acceleration.x;
    points[i].velocity.y += points[i].acceleration.y;
    points[i].velocity.z += points[i].acceleration.z;
    points[i].position.x += points[i].velocity.x;
    points[i].position.y += points[i].velocity.y;
    points[i].position.z += points[i].velocity.z;
  }
  BarnesHut_free(bh);
  draw();
}

void draw() {
  for (int i = 0; i < POINTCNT; i++) {
    /*
    glPushMatrix();
    glTranslatef(points[i].position.x,
		 points[i].position.y,
		 points[i].position.z);
    glutSolidSphere(0.5,2,2);
    glPopMatrix();
    */
  }
}
