// The OPEN_CL_FLAG switches the program to OPEN_CL mode
// Turning this flag off allows the program to compile without OpenCL data
//#define OPEN_CL_FLAG

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/gl.h>
#ifdef OPEN_CL_FLAG
#include <CL/opencl.h>
#endif
#include "barneshut.h"


#define SIZER 1e7
#define MASS 6e24
#define _POINTCNT_ 512

const int POINTCNT = _POINTCNT_;


/* This OpenCL kernel defines the calculations that turn a foce and mass into 
 * velocity and position */
const char *KERNEL_calc = "__kernel void calc("				\
  "  __global float* mass,"						\
  "  __global float* force_x,"						\
  "  __global float* force_y,"						\
  "  __global float* force_z,"						\
  "  __global float* velocity_x,"					\
  "  __global float* velocity_y,"					\
  "  __global float* velocity_z,"					\
  "  __global float* position_x,"					\
  "  __global float* position_y,"					\
  "  __global float* position_z,"					\
  "  const unsigned int count) {"					\
  "    int i = get_global_id(0);"					\
  "    if (i < count) {"						\
  "      velocity_x[i] += force_x[i]/mass[i];"				\
  "      position_x[i] += velocity_x[i];"				\
  "    } else if (i < count*2) {"					\
  "      i -= count;"							\
  "      velocity_y[i] += force_y[i]/mass[i];"				\
  "      position_y[i] += velocity_y[i];"				\
  "    } else if (i < count*3) {"					\
  "      i -= count*2;"							\
  "      velocity_z[i] += force_z[i]/mass[i];"				\
  "      position_z[i] += velocity_z[i];"				\
  "    }"								\
  "  }";


void init();
void deinit();
void draw();
void update();


#ifdef OPEN_CL_FLAG
void cl_printerr(cl_int error) {
  switch(error) {
  case CL_BUILD_PROGRAM_FAILURE :
    fprintf(stderr, "Failed to build the program\n");
    break;
  case CL_COMPILER_NOT_AVAILABLE :
    fprintf(stderr, "Compiler not available\n");
    break;
  case CL_DEVICE_NOT_AVAILABLE :
    fprintf(stderr, "Device not currently available\n");
    break;
  case CL_DEVICE_NOT_FOUND :
    fprintf(stderr, "Device not found\n");
    break;
  case CL_IMAGE_FORMAT_MISMATCH :
    fprintf(stderr, "Image format mismatch\n");
    break;
  case CL_IMAGE_FORMAT_NOT_SUPPORTED :
    fprintf(stderr, "Image format not supported\n");
    break;
  case CL_INVALID_ARG_INDEX :
    fprintf(stderr, "Invalid argument index\n");
    break;
  case CL_INVALID_ARG_SIZE :
    fprintf(stderr, "Invalid argument size\n");
    break;
  case CL_INVALID_ARG_VALUE :
    fprintf(stderr, "Invalid argument index\n");
    break;
  case CL_INVALID_BINARY :
    fprintf(stderr, "Invalid program binary\n");
    break;
  case CL_INVALID_BUFFER_SIZE :
    fprintf(stderr, "Invalid buffer size\n");
    break;
  case CL_INVALID_BUILD_OPTIONS :
    fprintf(stderr, "Invalid build options\n");
    break;
  case CL_INVALID_COMMAND_QUEUE :
    fprintf(stderr, "Invalid command queue\n");
    break;
  case CL_INVALID_CONTEXT :
    fprintf(stderr, "Invalid context\n");
    break;
  case CL_INVALID_DEVICE :
    fprintf(stderr, "Invalid device\n");
    break;
  case CL_INVALID_KERNEL :
    fprintf(stderr, "Invalid kernel\n");
    break;
  case CL_INVALID_KERNEL_ARGS :
    fprintf(stderr, "Invalid kernel argument(s)\n");
    break;
  case CL_INVALID_OPERATION :
    fprintf(stderr, "Invalid operation\n");
    break;
  case CL_INVALID_PROGRAM :
    fprintf(stderr, "Invalid program\n");
    break;
  case CL_INVALID_PROGRAM_EXECUTABLE :
    fprintf(stderr, "Invalid program executable\n");
    break;
  case CL_INVALID_VALUE :
    fprintf(stderr, "Invalid value\n");
    break;
  case CL_INVALID_WORK_DIMENSION :
    fprintf(stderr, "Invalid work dimension\n");
    break;
  case CL_INVALID_WORK_GROUP_SIZE :
    fprintf(stderr, "invalid work group size\n");
    break;
  case CL_OUT_OF_HOST_MEMORY :
    fprintf(stderr, "Out of host memory\n");
    break;
  case CL_SUCCESS :
    fprintf(stderr, "Execution successful!\n");
    break;
  default :
    fprintf(stderr, "Unknown error!\n");
  }
}
#endif


/* Resize function called when GLUT resizes */
void resize(int w, int h) {
  if (h == 0)
    h = 1;
  float ratio =  w * 1.0 / h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(45.0f, ratio, 0.1f, SIZER*32.0);
  glMatrixMode(GL_MODELVIEW);
}

/* The radius of each body in the system */
int bodysize = SIZER*0.005;
/* Whether or not to draw the initialization box and its features */
int drawbox = 1;

/* Keyboard called when keyboard input is detected in GLUT */
void keyboard(unsigned char key, int x, int y) {
  if (key == 27) deinit();
  if (key == 43 || key == 61) bodysize += SIZER*0.01;
  if (key == 45 || key == 95) bodysize -= SIZER*0.01;
  if (key == 32) drawbox = !drawbox;
}

/* The rotational velocity of the camera */
float roty_speed = 0.005;
/* The zoom of the camera */
float zoom = SIZER*4.0;

/* Keys called when special key input is detected in GLUT */
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

/* Calculates the frames per second that GLUT is operating at */
float calculateFPS() {
    static unsigned int frameCount = 0;
    static int previoustime = 0;
    static float fps = 0;
    frameCount++;
    int gluttime =  glutGet(GLUT_ELAPSED_TIME);
    int timeinterval = gluttime - previoustime;
    if (timeinterval > 1000) {
        fps = frameCount / (timeinterval / 1000.0f);
        previoustime = gluttime;
        frameCount = 0;
        printf("%f\n", fps);
    }
    return fps;
}


float mass[_POINTCNT_];
float position_x[_POINTCNT_];
float position_y[_POINTCNT_];
float position_z[_POINTCNT_];
float velocity_x[_POINTCNT_];
float velocity_y[_POINTCNT_];
float velocity_z[_POINTCNT_];
float force_x[_POINTCNT_];
float force_y[_POINTCNT_];
float force_z[_POINTCNT_];

BarnesHut *bh;


int main(int argc, char **argv) {
  init();
  // setup glut
  glutInit(&argc, argv);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(640,480);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("WINDOW");
  glutDisplayFunc(update);
  glutReshapeFunc(resize);
  glutIdleFunc(update);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keys);
  glutMainLoop();
  // the following will never be encountered
  return 0;
}

#ifdef OPEN_CL_FLAG
cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_command_queue command_queue;
cl_program program_calc;
cl_kernel kernel_calc;
size_t cl_calc_local;
size_t cl_calc_global;
cl_mem cl_calc_mass;
cl_mem cl_calc_force_x;
cl_mem cl_calc_force_y;
cl_mem cl_calc_force_z;
cl_mem cl_calc_velocity_x;
cl_mem cl_calc_velocity_y;
cl_mem cl_calc_velocity_z;
cl_mem cl_calc_position_x;
cl_mem cl_calc_position_y;
cl_mem cl_calc_position_z;
#endif

void init() {
  bh = NULL;
  /* Randomize the starting positions and initial velocities of the bodies */
  for (int i = 0; i < POINTCNT; i++) {
    mass[i] = MASS;
    position_x[i] = (rand()/((float)RAND_MAX))*SIZER*2 - SIZER;
    position_y[i] = (rand()/((float)RAND_MAX))*SIZER*2 - SIZER;
    position_z[i] = (rand()/((float)RAND_MAX))*SIZER*2 - SIZER;
    velocity_x[i] = (rand()/((float)RAND_MAX))*-20000.0f + 10000.0f;
    velocity_y[i] = (rand()/((float)RAND_MAX))*-20000.0f + 10000.0f;
    velocity_z[i] = (rand()/((float)RAND_MAX))*-20000.0f + 10000.0f;
    force_x[i] = force_y[i] = force_z[i] = 0.0f;
  }
#ifdef OPEN_CL_FLAG
  /* Initialize OpenCL and the necessary kernels */
  do {
    cl_uint count;
    /* get the first cl_platform, if there is one */
    clGetPlatformIDs(0, NULL, &count);
    if (!count) {
      puts("No CL platform found!");
      break;
    }
    clGetPlatformIDs(1, &platform, NULL);
    /* get the first cl_device for the first cl_platform */
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &count);
    if (!count) {
      puts("No CL device on the first platform!");
      break;
    }
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    /* setup OpenCL */
    cl_int error;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create context, with error:");
      cl_printerr(error);
      break;
    }
    command_queue = clCreateCommandQueue(context, device, 0, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create command queue, with error:");
      cl_printerr(error);
      break;
    }
    /* Fully set up the calculation kernel so that it is ready for execution */
    program_calc = clCreateProgramWithSource(context, 1, &KERNEL_calc,
					     NULL, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create program, with error:");
      cl_printerr(error);
      break;
    }
    error = clBuildProgram(program_calc, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to build program, with error:");
      cl_printerr(error);
      break;
    }
    kernel_calc = clCreateKernel(program_calc, "calc", &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create kernel, with error:");
      cl_printerr(error);
    }
    clGetKernelWorkGroupInfo(kernel_calc, device, CL_KERNEL_WORK_GROUP_SIZE, 
			     sizeof(cl_calc_local), &cl_calc_local, NULL);
    if (cl_calc_local > POINTCNT*3) cl_calc_local = POINTCNT*3;
    cl_calc_global = (POINTCNT*3) % cl_calc_local;
    if (cl_calc_global == 0)
      cl_calc_global = POINTCNT*3;
    else
      cl_calc_global = 
	cl_calc_local + POINTCNT*3 - cl_calc_global;
    printf("%d %d\n", cl_calc_local, cl_calc_global);
    cl_calc_mass = clCreateBuffer(context, CL_MEM_READ_ONLY,
				  sizeof(cl_float)*POINTCNT, 
				  NULL, NULL);
    cl_calc_force_x = clCreateBuffer(context, CL_MEM_READ_ONLY,
				     sizeof(cl_float)*POINTCNT,
				     NULL, NULL);
    cl_calc_force_y = clCreateBuffer(context, CL_MEM_READ_ONLY,
				     sizeof(cl_float)*POINTCNT,
				     NULL, NULL);
    cl_calc_force_z = clCreateBuffer(context, CL_MEM_READ_ONLY, 
				     sizeof(cl_float)*POINTCNT,
				     NULL, NULL);
    cl_calc_velocity_x = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    cl_calc_velocity_y = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    cl_calc_velocity_z = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    cl_calc_position_x = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    cl_calc_position_y = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    cl_calc_position_z = clCreateBuffer(context, CL_MEM_READ_WRITE,
					sizeof(cl_float)*POINTCNT,
					NULL, NULL);
    clSetKernelArg(kernel_calc, 0, sizeof(cl_mem), &cl_calc_mass);
    clSetKernelArg(kernel_calc, 1, sizeof(cl_mem), &cl_calc_force_x);
    clSetKernelArg(kernel_calc, 2, sizeof(cl_mem), &cl_calc_force_y);
    clSetKernelArg(kernel_calc, 3, sizeof(cl_mem), &cl_calc_force_z);
    clSetKernelArg(kernel_calc, 4, sizeof(cl_mem), &cl_calc_velocity_x);
    clSetKernelArg(kernel_calc, 5, sizeof(cl_mem), &cl_calc_velocity_y);
    clSetKernelArg(kernel_calc, 6, sizeof(cl_mem), &cl_calc_velocity_z);
    clSetKernelArg(kernel_calc, 7, sizeof(cl_mem), &cl_calc_position_x);
    clSetKernelArg(kernel_calc, 8, sizeof(cl_mem), &cl_calc_position_y);
    clSetKernelArg(kernel_calc, 9, sizeof(cl_mem), &cl_calc_position_z);
    clSetKernelArg(kernel_calc, 10, sizeof(cl_int), &POINTCNT);
  } while (0);
#endif
}
void deinit() {
#ifdef OPEN_CL_FLAG
  /* Cleanup all the OpenCL memory */
  clReleaseMemObject(cl_calc_mass);
  clReleaseMemObject(cl_calc_force_x);
  clReleaseMemObject(cl_calc_force_y);
  clReleaseMemObject(cl_calc_force_z);
  clReleaseMemObject(cl_calc_velocity_x);
  clReleaseMemObject(cl_calc_velocity_y);
  clReleaseMemObject(cl_calc_velocity_z);
  clReleaseMemObject(cl_calc_position_x);
  clReleaseMemObject(cl_calc_position_y);
  clReleaseMemObject(cl_calc_position_z);
  clReleaseKernel(kernel_calc);
  clReleaseProgram(program_calc);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);
#endif
  exit(EXIT_SUCCESS);
}


/* Stores the minimum and maximum bounds of the system */
float bound_min_x = -SIZER;
float bound_min_y = -SIZER;
float bound_min_z = -SIZER;
float bound_max_x = SIZER;
float bound_max_y = SIZER;
float bound_max_z = SIZER;


void update() {
  /* Create, fill, and finalize a new Barnes-Hut setup */
  bh = BarnesHut_malloc(bound_min_x,bound_min_y,bound_min_z,
			bound_max_x,bound_max_y,bound_max_z);
  for (int i = 0; i < POINTCNT; i++) {
    BarnesHut_add(bh, position_x[i], position_y[i], position_z[i], mass[i]);
  }
  BarnesHut_finalize(bh);
  /* Clear the bounds of the system; they will be recalculated */
  bound_min_x = bound_min_y = bound_min_z = 0;
  bound_max_x = bound_max_y = bound_max_z = 0;
  for (int i = 0; i < POINTCNT; i++) {
    /* get the force of body[i] */
    BarnesHut_force(bh, position_x[i], position_y[i], position_z[i], mass[i], 
		    &force_x[i], &force_y[i], &force_z[i]);
    /*Vector3f force = BarnesHut_force(bh, (Vector3f){position_x[i],position_y[i],position_z[i]}, mass[i]);
      force_x[i] = force.x; force_y[i] = force.y; force_z[i] = force.z;*/
#ifndef OPEN_CL_FLAG
    /* calculate velocity and position of body[i] */
    float acceleration_x = force_x[i]/mass[i];
    float acceleration_y = force_y[i]/mass[i];
    float acceleration_z = force_z[i]/mass[i];
    velocity_x[i] += acceleration_x;
    velocity_y[i] += acceleration_y;
    velocity_z[i] += acceleration_z;
    position_x[i] += velocity_x[i];
    if (position_x[i] < bound_min_x) bound_min_x = position_x[i];
    if (position_x[i] > bound_max_x) bound_max_x = position_x[i];
    position_y[i] += velocity_y[i];
    if (position_y[i] < bound_min_y) bound_min_y = position_y[i];
    if (position_y[i] > bound_max_y) bound_max_y = position_y[i];
    position_z[i] += velocity_z[i];
    if (position_z[i] < bound_min_z) bound_min_z = position_z[i];
    if (position_z[i] > bound_max_z) bound_max_z = position_z[i];
#endif
  }
  BarnesHut_free(bh);
  bh = NULL;
#ifdef OPEN_CL_FLAG
  /* Pass the appropriate memory to OpenCL and calculate the mass and velocity 
   * of each body in the system */
  clEnqueueWriteBuffer(command_queue, cl_calc_mass, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, mass, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_force_x, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_force_y, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_force_z, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_z, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_velocity_x, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_velocity_y, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_velocity_z, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_z, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_position_x, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, position_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_position_y, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, position_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_calc_position_z, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, position_z, 0, NULL, NULL);
  clEnqueueNDRangeKernel(command_queue, kernel_calc, 1, NULL, 
			 &cl_calc_global, &cl_calc_local, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_velocity_x, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_x, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_velocity_y, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_y, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_velocity_z, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_z, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_position_x, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_x, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_position_y, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_y, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_calc_position_z, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_z, 0, NULL, NULL);
  /* recalculate the bounds of the system */
  for (int i = 0; i < POINTCNT; i++) {
    if (position_x[i] < bound_min_x) bound_min_x = position_x[i];
    if (position_x[i] > bound_max_x) bound_max_x = position_x[i];
    if (position_y[i] < bound_min_y) bound_min_y = position_y[i];
    if (position_y[i] > bound_max_y) bound_max_y = position_y[i];
    if (position_z[i] < bound_min_z) bound_min_z = position_z[i];
    if (position_z[i] > bound_max_z) bound_max_z = position_z[i];
  }
#endif
  draw();
  calculateFPS();
}


void draw() {
  /* (re)set the appropriate OpenGL settings */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  static float roty = 0.0;
  gluLookAt(zoom*sin(roty), 0.0f, zoom*cos(roty),
	    0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f);
  roty += roty_speed;
  if (drawbox)
    glutWireCube(SIZER*2);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < POINTCNT; i++) {
    glPushMatrix();
    if (drawbox) {
      /* Determine the color of the object based on its position */
      if (position_x[i] > SIZER || position_x[i] < -SIZER || 
	  position_y[i] > SIZER || position_y[i] < -SIZER || 
	  position_z[i] > SIZER || position_z[i] < -SIZER)
	glColor3f(1.0f, 0.0f, 0.0f);
      else {
	float color = 1-(position_x[i]+position_y[i]+position_z[i])/(SIZER*3);
	glColor3f(1.0f, color, color);
	glColor3f(1.0f,1.0f,1.0f);
      }
    }
    glTranslatef(position_x[i], position_y[i], position_z[i]);
    glutSolidSphere(bodysize,4,4);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
  }
  /*DRAW!*/
  glutSwapBuffers();
}
