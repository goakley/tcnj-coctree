#define OPEN_CL_FLAG


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#ifdef OPEN_CL_FLAG
#include <CL/opencl.h>
#endif
#include "barneshut.h"


#define POINTCNT 512
//512
//#define SIZER (150e6)
#define SIZER 1e7
#define MASS 6e24


int OPEN_CL_FLAG_VAR = 0;


int POINT_COUNT = POINTCNT;


const char *KERNEL_velocity = "__kernel void calc_velocity("		\
  "  __global float* mass,"						\
  "  __global float* force_x,"						\
  "  __global float* force_y,"						\
  "  __global float* force_z,"						\
  "  __global float* velocity_x,"					\
  "  __global float* velocity_y,"					\
  "  __global float* velocity_z,"					\
  "  const int count)\n"						\
  "  {\n"								\
  "    int i = get_global_id(0);"					\
  "    if (i >= count) return;\n"					\
  "    velocity_x[i] += force_x[i]/mass[i];\n"				\
  "    velocity_y[i] += force_y[i]/mass[i];\n"				\
  "    velocity_z[i] += force_z[i]/mass[i];\n"				\
  "  }\n";

const char *KERNEL_position = "__kernel void calc_position("		\
  "  __global float* velocity_x,"					\
  "  __global float* velocity_y,"					\
  "  __global float* velocity_z,"					\
  "  __global float* position_x,"					\
  "  __global float* position_y,"					\
  "  __global float* position_z,"					\
  "  const unsigned int count)\n"					\
  "  {\n"								\
  "    int i = get_global_id(0);\n"					\
  "    if (i >= count) return;\n"					\
  "    position_x[i] = position_x[i] + velocity_x[i];\n"		\
  "    position_y[i] = position_y[i] + velocity_y[i];\n"		\
  "    position_z[i] = position_y[i] + velocity_z[i];\n"		\
  "  }\n";

const char *kernel_calc = "__kernel void calcforce("			\
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
  "  const unsigned int count)\n"					\
  "  {\n"								\
  "    int i = get_global_id(0);"					\
  "    if (i >= count) return;\n"					\
  "    velocity_x[i] += force_x[i]/mass[i];\n"			\
  "    velocity_y[i] += force_y[i]/mass[i];\n"			\
  "    velocity_z[i] += force_z[i]/mass[i];\n"			\
  "    position_x[i] += velocity_x[i];\n"				\
  "    position_y[i] += velocity_y[i];\n"				\
  "    position_z[i] += velocity_z[i];\n"				\
  "  }\n";



void init();
void deinit();
void glut_setup();
void draw();
void update();



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

int bodysize = SIZER*0.01;

void keyboard(unsigned char key, int x, int y) {
  if (key == 27) deinit();
  if (key == 43 || key == 61) bodysize += SIZER*0.01;
  if (key == 45 || key == 95) bodysize -= SIZER*0.01;
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


float mass[POINTCNT];
float position_x[POINTCNT];
float position_y[POINTCNT];
float position_z[POINTCNT];
float velocity_x[POINTCNT];
float velocity_y[POINTCNT];
float velocity_z[POINTCNT];
float force_x[POINTCNT];
float force_y[POINTCNT];
float force_z[POINTCNT];

BarnesHut *bh;



int main(int argc, char **argv) {
  init();
  glutInit(&argc, argv);
  glut_setup();
  glutMainLoop();
  return 0;
}

#ifdef OPEN_CL_FLAG
cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_command_queue command_queue;
cl_program program_velocity;
cl_kernel kernel_velocity;
size_t kernel_velocity_local;
size_t kernel_velocity_global;
cl_mem cl_velocity_mass;
cl_mem cl_velocity_force_x;
cl_mem cl_velocity_force_y;
cl_mem cl_velocity_force_z;
cl_mem cl_velocity_velocity_x;
cl_mem cl_velocity_velocity_y;
cl_mem cl_velocity_velocity_z;
cl_program program_position;
cl_kernel kernel_position;
size_t kernel_position_local;
size_t kernel_position_global;
cl_mem cl_position_velocity_x;
cl_mem cl_position_velocity_y;
cl_mem cl_position_velocity_z;
cl_mem cl_position_position_x;
cl_mem cl_position_position_y;
cl_mem cl_position_position_z;
#endif

void init() {
  bh = NULL;
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
  do {
    cl_uint count;
    /* get the first cl_platform, if there is one */
    clGetPlatformIDs(0, NULL, &count);
    if (!count) {
      puts("No CL platform found!");
      OPEN_CL_FLAG_VAR = 0;
      break;
    }
    clGetPlatformIDs(1, &platform, NULL);
    /* get the first cl_device for the first cl_platform */
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &count);
    if (!count) {
      puts("No CL device on the first platform!");
      OPEN_CL_FLAG_VAR = 0;
      break;
    }
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    /* THINGS */
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
    program_velocity = clCreateProgramWithSource(context, 1, 
						 &KERNEL_velocity, 
						 NULL, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create program_velocity, with error:");
      cl_printerr(error);
      break;
    }
    error = clBuildProgram(program_velocity, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to build program_velocity, with error:");
      cl_printerr(error);
      break;
    }
    kernel_velocity = clCreateKernel(program_velocity, "calc_velocity", &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create kernel_velocity, with error:");
      cl_printerr(error);
    }
    clGetKernelWorkGroupInfo(kernel_velocity, device, 
			     CL_KERNEL_WORK_GROUP_SIZE, 
			     sizeof(kernel_velocity_local), 
			     &kernel_velocity_local, NULL);
    if (kernel_velocity_local > POINTCNT) kernel_velocity_local = POINTCNT;
    kernel_velocity_global = POINTCNT % kernel_velocity_local;
    if (kernel_velocity_global == 0)
      kernel_velocity_global = POINTCNT;
    else
      kernel_velocity_global = 
	kernel_velocity_local + POINTCNT - kernel_velocity_global;
    printf("%d %d\n", kernel_velocity_local, kernel_velocity_global);
    cl_velocity_mass = clCreateBuffer(context, CL_MEM_READ_ONLY,
				      sizeof(cl_float)*POINTCNT, 
				      NULL, NULL);
    cl_velocity_force_x = clCreateBuffer(context, CL_MEM_READ_ONLY,
					 sizeof(cl_float)*POINTCNT,
					 NULL, NULL);
    cl_velocity_force_y = clCreateBuffer(context, CL_MEM_READ_ONLY,
					 sizeof(cl_float)*POINTCNT,
					 NULL, NULL);
    cl_velocity_force_z = clCreateBuffer(context, CL_MEM_READ_ONLY, 
					 sizeof(cl_float)*POINTCNT,
					 NULL, NULL);
    cl_velocity_velocity_x = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_velocity_velocity_y = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_velocity_velocity_z = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    clSetKernelArg(kernel_velocity, 0, sizeof(cl_mem), &cl_velocity_mass);
    clSetKernelArg(kernel_velocity, 1, sizeof(cl_mem), &cl_velocity_force_x);
    clSetKernelArg(kernel_velocity, 2, sizeof(cl_mem), &cl_velocity_force_y);
    clSetKernelArg(kernel_velocity, 3, sizeof(cl_mem), &cl_velocity_force_z);
    clSetKernelArg(kernel_velocity, 4, sizeof(cl_mem), &cl_velocity_velocity_x);
    clSetKernelArg(kernel_velocity, 5, sizeof(cl_mem), &cl_velocity_velocity_y);
    clSetKernelArg(kernel_velocity, 6, sizeof(cl_mem), &cl_velocity_velocity_z);
    clSetKernelArg(kernel_velocity, 7, sizeof(cl_int), &POINT_COUNT);
    program_position = clCreateProgramWithSource(context, 1, 
						 &KERNEL_position,
						 NULL, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create program_position, with error:");
      cl_printerr(error);
    }
    error = clBuildProgram(program_position, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to build program_position, with error:");
      cl_printerr(error);
    }
    kernel_position = clCreateKernel(program_position, "calc_position", &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Unable to create kernel_position, with error:");
      cl_printerr(error);
    }
    clGetKernelWorkGroupInfo(kernel_position, device, 
			     CL_KERNEL_WORK_GROUP_SIZE, 
			     sizeof(kernel_position_local), 
			     &kernel_position_local, NULL);
    if (kernel_position_local > POINTCNT) kernel_position_local = POINTCNT;
    kernel_position_global = POINTCNT % kernel_position_local;
    if (kernel_position_global == 0)
      kernel_position_global = POINTCNT;
    else
      kernel_position_global = 
	kernel_position_local + POINTCNT - kernel_position_global;
    printf("%d %d\n", kernel_position_local, kernel_position_global);
    cl_position_velocity_x = clCreateBuffer(context, CL_MEM_READ_ONLY,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_position_velocity_y = clCreateBuffer(context, CL_MEM_READ_ONLY,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_position_velocity_z = clCreateBuffer(context, CL_MEM_READ_ONLY,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_position_position_x = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_position_position_y = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    cl_position_position_z = clCreateBuffer(context, CL_MEM_READ_WRITE,
					    sizeof(cl_float)*POINTCNT,
					    NULL, NULL);
    clSetKernelArg(kernel_position, 0, sizeof(cl_mem), &cl_position_velocity_x);
    clSetKernelArg(kernel_position, 1, sizeof(cl_mem), &cl_position_velocity_y);
    clSetKernelArg(kernel_position, 2, sizeof(cl_mem), &cl_position_velocity_z);
    clSetKernelArg(kernel_position, 3, sizeof(cl_mem), &cl_position_position_x);
    clSetKernelArg(kernel_position, 4, sizeof(cl_mem), &cl_position_position_y);
    clSetKernelArg(kernel_position, 5, sizeof(cl_mem), &cl_position_position_z);
    clSetKernelArg(kernel_position, 6, sizeof(cl_int), &POINT_COUNT);
  } while (0);
#endif
}
void deinit() {
#ifdef OPEN_CL_FLAG
  clReleaseMemObject(cl_velocity_mass);
  clReleaseMemObject(cl_velocity_force_x);
  clReleaseMemObject(cl_velocity_force_y);
  clReleaseMemObject(cl_velocity_force_z);
  clReleaseMemObject(cl_velocity_velocity_x);
  clReleaseMemObject(cl_velocity_velocity_y);
  clReleaseMemObject(cl_velocity_velocity_z);
  clReleaseKernel(kernel_velocity);
  clReleaseProgram(program_velocity);
  clReleaseMemObject(cl_position_velocity_x);
  clReleaseMemObject(cl_position_velocity_y);
  clReleaseMemObject(cl_position_velocity_z);
  clReleaseMemObject(cl_position_position_x);
  clReleaseMemObject(cl_position_position_y);
  clReleaseMemObject(cl_position_position_z);
  clReleaseKernel(kernel_position);
  clReleaseProgram(program_position);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);
#endif
  exit(EXIT_SUCCESS);
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


Vector3f min = {-SIZER*4,-SIZER*4,-SIZER*4};
Vector3f max = {SIZER*4,SIZER*4,SIZER*4};


void update() {
  bh = BarnesHut_malloc(min,max);
  min.x = min.y = min.z = 0;
  max.x = max.y = max.z = 0;
  //printf("\t%f %f %f\n", position_x[0], position_y[0], position_z[0]);
  for (int i = 0; i < POINTCNT; i++) {
    BarnesHut_add(bh, (Vector3f){position_x[i],position_y[i],position_z[i]}, 
                  mass[i]);
  }
  BarnesHut_finalize(bh);
  for (int i = 0; i < POINTCNT; i++) {
    Vector3f force = BarnesHut_force(bh, (Vector3f){position_x[i],position_y[i],position_z[i]}, mass[i]);
    force_x[i] = force.x; force_y[i] = force.y; force_z[i] = force.z;
#ifndef OPEN_CL_FLAG
    float acceleration_x = force_x[i]/mass[i];
    float acceleration_y = force_y[i]/mass[i];
    float acceleration_z = force_z[i]/mass[i];
    velocity_x[i] += acceleration_x;
    velocity_y[i] += acceleration_y;
    velocity_z[i] += acceleration_z;
    position_x[i] += velocity_x[i];
    if (position_x[i] < min.x) min.x = position_x[i];
    if (position_x[i] > max.x) max.x = position_x[i];
    position_y[i] += velocity_y[i];
    if (position_y[i] < min.y) min.y = position_y[i];
    if (position_y[i] > max.y) max.y = position_y[i];
    position_z[i] += velocity_z[i];
    if (position_z[i] < min.z) min.z = position_z[i];
    if (position_z[i] > max.z) max.z = position_z[i];
#endif
  }
  BarnesHut_free(bh);
  bh = NULL;
#ifdef OPEN_CL_FLAG
  // GPU
  // velocity
  clEnqueueWriteBuffer(command_queue, cl_velocity_mass, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, mass, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_force_x, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_force_y, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_force_z, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, force_z, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_velocity_x, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_velocity_y, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_velocity_velocity_z, CL_FALSE, 0,
		       sizeof(cl_float)*POINTCNT, velocity_z, 0, NULL, NULL);
  clEnqueueNDRangeKernel(command_queue, kernel_velocity, 1, NULL, 
			 &kernel_velocity_global, &kernel_velocity_local, 
			 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_velocity_velocity_x, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_x, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_velocity_velocity_y, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_y, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_velocity_velocity_z, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, velocity_z, 0, NULL, NULL);
  // position
  clEnqueueWriteBuffer(command_queue, cl_position_velocity_x, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, velocity_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_position_velocity_y, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, velocity_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_position_velocity_z, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, velocity_z, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_position_position_x, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, position_x, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_position_position_y, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, position_y, 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queue, cl_position_position_z, CL_FALSE, 0, 
		       sizeof(cl_float)*POINTCNT, position_z, 0, NULL, NULL);
  clEnqueueNDRangeKernel(command_queue, kernel_position, 1, NULL, 
			 &kernel_position_global, &kernel_position_local, 
			 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_position_position_x, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_x, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_position_position_y, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_y, 0, NULL, NULL);
  clEnqueueReadBuffer(command_queue, cl_position_position_z, CL_TRUE, 0, 
		      sizeof(cl_float)*POINTCNT, position_z, 0, NULL, NULL);
  for (int i = 0; i < POINTCNT; i++) {
    if (position_x[i] < min.x) min.x = position_x[i];
    if (position_x[i] > max.x) max.x = position_x[i];
    if (position_y[i] < min.y) min.y = position_y[i];
    if (position_y[i] > max.y) max.y = position_y[i];
    if (position_z[i] < min.z) min.z = position_z[i];
    if (position_z[i] > max.z) max.z = position_z[i];
  }

#endif
  draw();
  calculateFPS();
  static int asdf = 0;
  asdf++;
  //if (asdf > 3)
  //  deinit();
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
    if (position_x[i] > SIZER || position_x[i] < -SIZER || 
	position_y[i] > SIZER || position_y[i] < -SIZER || 
	position_z[i] > SIZER || position_z[i] < -SIZER)
      glColor3f(1.0f, 0.0f, 0.0f);
    else {
      float color = 1-(position_x[i]+position_y[i]+position_z[i])/(SIZER*3);
      glColor3f(1.0f, color, color);
      glColor3f(1.0f,1.0f,1.0f);
    }
    glTranslatef(position_x[i], position_y[i], position_z[i]);
    glutSolidSphere(bodysize,3,3);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
  }

  glutSwapBuffers();

}
