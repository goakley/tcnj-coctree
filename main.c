#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "barneshut.h"


#define POINTCNT 256
#define SIZER 400
#define MASS 5e9


void resize(int w, int h) {
  if (h == 0)
    h = 1;
  float ratio =  w * 1.0 / h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(45.0f, ratio, 0.1f, SIZER*8.0);
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27) exit(EXIT_SUCCESS);
}

float roty_speed = 0.005;
float zoom = SIZER*4.0;

void keys(int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_RIGHT:
    roty_speed += 0.001;
    break;
  case GLUT_KEY_LEFT:
    roty_speed -= 0.001;
    break;
  case GLUT_KEY_UP:
    zoom -= SIZER*0.05;
    break;
  case GLUT_KEY_DOWN:
    zoom += SIZER*0.05;
    break;
  }
}



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
void glut_setup();
void draw();
void update();


int main(int argc, char **argv) {
  init();
  glutInit(&argc, argv);
  glut_setup();
  glutMainLoop();
  return 0;
}


void init() {
  bh = NULL;
  Vector3f zerovector = {0,0,0};
  points[0].mass = MASS*10.0;
  points[0].position = zerovector;
  points[0].velocity = zerovector;
  points[0].acceleration = zerovector;
  points[0].force = zerovector;
  for (int i = 1; i < POINTCNT; i++) {
    points[i].mass = MASS;
    points[i].position.x = rand()%(SIZER*2)-SIZER;
    points[i].position.y = rand()%(SIZER*2)-SIZER;
    points[i].position.z = rand()%(SIZER*2)-SIZER;
    points[i].velocity = zerovector;
    points[i].acceleration = zerovector;
    points[i].force = zerovector;
  }
}

void glut_setup() {
  glutInitWindowPosition(0,0);
  glutInitWindowSize(640,480);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("WINDOW");
  glutDisplayFunc(update);
  glutReshapeFunc(resize);
  glutIdleFunc(update);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keys);
}

void update() {
  Vector3f min = {-SIZER*4,-SIZER*4,-SIZER*4};
  Vector3f max = {SIZER*4,SIZER*4,SIZER*4};
  bh = BarnesHut_malloc(min,max);
  for (int i = 0; i < POINTCNT; i++)
    BarnesHut_add(bh, points[i].position, points[i].mass);
  BarnesHut_finalize(bh);
  points[0].position.x = points[0].position.y = points[0].position.z = 0;
  for (int i = 1; i < POINTCNT; i++) {
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  static float roty = 0.0;
  gluLookAt(zoom*sin(roty), 0.0f, zoom*cos(roty),
	    0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f);
  roty += roty_speed;

  /* Draw a cube to show the bounds of the origin of the system */
  glutWireCube(SIZER*2);

  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < POINTCNT; i++) {
    glPushMatrix();
    if (points[i].position.x > SIZER || points[i].position.x < -SIZER || 
	points[i].position.y > SIZER || points[i].position.y < -SIZER || 
	points[i].position.z > SIZER || points[i].position.z < -SIZER)
      glColor3f(1.0f, 0.0f, 0.0f);
    glTranslatef(points[i].position.x,
		 points[i].position.y,
		 points[i].position.z);
    glutSolidSphere(1,4,4);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
  }

  /*
  glPushMatrix();
  glutSolidSphere(4,16,16);
  glPopMatrix();
  */


  glutSwapBuffers();
}
