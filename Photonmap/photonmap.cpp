/*

Example animation using GLUT routines.

Greg Turk, April 1998

*/
#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <math.h>
#include "photons.h"
#include "icVector.H"

/* light position */
GLfloat  lightPos[] = { 0.0, 0.0, 0.0, 1.0 };

/* light contribution */
GLfloat  ambientLight[] = { 0.8, 0.8, 0.8, 0.0 };
GLfloat  diffuseLight[] = { 0.7, 0.7, 0.7, 0.0 };
GLfloat  specularLight[] = { 1.0, 1.0, 1.0, 0.0 };

GLfloat whiteLight[] = { 1.0, 1.0, 1.0, 0.0 };
GLfloat noLight[] = { 0.0, 0.0, 0.0, 0.0 };


/* two examples of specular contributions */
GLfloat  shiny_surface[] = { 1.0, 1.0, 1.0, 0.0 };

GLuint tex;
GLUquadric *quad;

int		winW, winH;
bool	trackOn = false;
int		lastMouseX, lastMouseY;
float	rotq[4], modelViewMat[16];

void surface2trackball(int x, int y, float pos[3]);
void calcRotation(float oldpos[3], float newpos[3], float rotAxis[3], float &rotAngle);
void mat16_to_m4x4(float mat16[16], float m[][4]);
void mat2quat(float mat[16], float q[4]);
void getQuat(float axis[3], float angle, float q[4]);
void quatMult(float q1[4], float q2[4], float product[4]);
void quat2mat(float quat[4], float mat[16]);
float calcNormal(float *vector, int dimension);

bool photonMapping = true;
icVector3 camPos;
/******************************************************************************
Draw a shiny sphere with a rotating cube in front of it.
******************************************************************************/

void draw_scene(int i)
{

	icVector3 v1, v2, v3, v4;
	GLfloat diffMat[3], specMat[3];

	glPushMatrix();

	/* specify the camera position (from, at, up) */

	camPos.set(0.0, 0.0, 100.0);
	gluLookAt(camPos.x, camPos.y, camPos.z ,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	if (trackOn) {
		glMultMatrixf(modelViewMat);
	}

	glEnable(GL_LIGHTING);
	
	glPushMatrix();
	
		glBegin(GL_QUADS);
		setMatVector(diffMat, 0.5, 0.0, 0.0); 
		setMatVector(specMat, 0.3, 0.3, 0.3);
		v1.set(-80.0, -80.0, -80.0);
		v2.set(80.0, -80.0, -80.0);
		v3.set(80.0, 80.0, -80.0);
		v4.set(-80.0, 80.0, -80.0);
		createRectangle( v1, v2, v3, v4, diffMat, specMat);


		setMatVector(diffMat, 0.0, 0.5, 0.0); 
		setMatVector(specMat, 0.3, 0.3, 0.3);
		v1.set(-80.0, -80.0, 80.0);
		v2.set(-80.0, -80.0, -80.0);
		v3.set(-80.0, 80.0, -80.0);
		v4.set(-80.0, 80.0, 80.0);
		createRectangle( v1, v2, v3, v4, diffMat, specMat);

		setMatVector(diffMat, 0.0, 0.5, 0.0); 
		setMatVector(specMat, 0.3, 0.3, 0.3);
		v1.set(80.0, -80.0, -80.0);
		v2.set(80.0, -80.0, 80.0);
		v3.set(80.0, 80.0, 80.0);
		v4.set(80.0, 80.0, -80.0);


		createRectangle( v1, v2, v3, v4, diffMat, specMat);

		setMatVector(diffMat, 0.0, 0.0, 0.5); 
		setMatVector(specMat, 0.3, 0.3, 0.3);
		v1.set(-80.0, -80.0, 80.0);
		v2.set(80.0, -80.0, 80.0);
		v3.set(80.0, -80.0, -80.0);
		v4.set(-80.0, -80.0, -80.0);
		createRectangle( v1, v2, v3, v4, diffMat, specMat);

		setMatVector(diffMat, 0.0, 0.0, 0.5); 
		setMatVector(specMat, 0.3, 0.3, 0.3);
		v1.set(80.0, 80.0, 80.0);
		v2.set(-80.0, 80.0, 80.0);
		v3.set(-80.0, 80.0, -80.0);
		v4.set(80.0, 80.0, -80.0);
		createRectangle( v1, v2, v3, v4, diffMat, specMat);


		glEnd();
	glPopMatrix();

	glPushMatrix();
	setMatVector(diffMat, 0.5, 0.5, 0.0);
	setMatVector(specMat, 0.3, 0.3, 0.3);
	icVector3 cen(30.,-50.,-50.);
	createSphere(cen, 30.f, 20, 20, diffMat, specMat);
	

	glPopMatrix();

	glPopMatrix();
}


/******************************************************************************
The actual animation loop that creates multiple frames.
******************************************************************************/

void display()
{
	int i = 0;
	unsigned char * buffer = new unsigned char[winW * winH* 3];

	winW = glutGet(GLUT_WINDOW_WIDTH);
	winH = glutGet(GLUT_WINDOW_HEIGHT);

	/* clear the color and z-buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* set up the perspective transformation */
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(90.0, 1.0, 1.0, 500.0);
	

	
	/* from now on we're affecting the modeling matrix */
	glMatrixMode(GL_MODELVIEW);

	/* draw our scene */
	draw_scene(i);

	tracePhotons();
	buffer = renderPhotonMap();

	
	glDrawBuffer(GL_BACK);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(winW, winH, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	/* swap that performs double-buffering */
	glutSwapBuffers();

	delete[] buffer;
}


/******************************************************************************
Initilize various graphics things such as surface material and lights.
******************************************************************************/
void
init_graphics()
{
	/* enable depth buffering */
	glEnable(GL_DEPTH_TEST);

	/* surface shading info */
	glFrontFace(GL_CCW);
	glShadeModel(GL_FLAT);


	/* turn on lighting */
	glEnable(GL_LIGHTING);

	setLight(GL_LIGHT0, lightPos, ambientLight, whiteLight, whiteLight);

	/* select clearing color */
	glClearColor(1.0, 1.0, 1.0, 0.0);

	// Texture
	int width, height;


}


/******************************************************************************
Process a keyboard action.  In particular, exit the program when an
"escape" is pressed in the window.
******************************************************************************/

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	}
}


void
quat2mat(float quat[4], float mat[16]) {

	float w, x, y, z, xx, xy, xz, xw, yy, yz, yw, zz, zw;

	w = quat[0];
	x = quat[1];
	y = quat[2];
	z = quat[3];

	xx = x * x;
	xy = x * y;
	xz = x * z;
	xw = x * w;

	yy = y * y;
	yz = y * z;
	yw = y * w;

	zz = z * z;
	zw = z * w;

	mat[0] = 1 - 2 * (yy + zz);
	mat[1] = 2 * (xy - zw);
	mat[2] = 2 * (xz + yw);

	mat[4] = 2 * (xy + zw);
	mat[5] = 1 - 2 * (xx + zz);
	mat[6] = 2 * (yz - xw);

	mat[8] = 2 * (xz - yw);
	mat[9] = 2 * (yz + xw);
	mat[10] = 1 - 2 * (xx + yy);

	mat[3] = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
	mat[15] = 1;
}


void
quatMult(float q1[4], float q2[4], float product[4]) {

	product[0] = (q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3]);
	product[1] = (q1[0] * q2[1] + q1[1] * q2[0] + q1[2] * q2[3] - q1[3] * q2[2]);
	product[2] = (q1[0] * q2[2] - q1[1] * q2[3] + q1[2] * q2[0] + q1[3] * q2[1]);
	product[3] = (q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1] + q1[3] * q2[0]);

	float norm = calcNormal(product, 4);
	product[0] = product[0] / norm;
	product[1] = product[1] / norm;
	product[2] = product[2] / norm;
	product[3] = product[3] / norm;
}


void
mat16_to_m4x4(float mat16[16], float m[][4]) {

	int i, j;
	for (int idx = 0; idx < 16; idx++) {
		i = idx / 4;
		j = idx % 4;

		m[i][j] = mat16[idx];
	}
}

/* Convert rotation matrix to quaternion*/
void
mat2quat(float mat[16], float q[4]) {

	float m[4][4], trace, s;
	mat16_to_m4x4(mat, m);


	trace = m[0][0] + m[1][1] + m[2][2];

	if (trace > 0) {
		s = sqrtf(trace + 1.0) * 2;
		q[0] = 0.25 * s;
		q[1] = (m[2][1] - m[1][2]) / s;
		q[2] = (m[0][2] - m[2][0]) / s;
		q[3] = (m[1][0] - m[0][1]) / s;
	}
	else if ((m[0][0] > m[1][1])&(m[0][0] > m[2][2])) {
		s = sqrtf(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2;
		q[0] = (m[2][1] - m[1][2]) / s;
		q[1] = 0.25 * s;
		q[2] = (m[0][1] + m[1][0]) / s;
		q[3] = (m[0][2] + m[2][0]) / s;
	}
	else if (m[1][1] > m[2][2]) {
		s = sqrtf(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2;
		q[0] = (m[0][2] - m[2][0]) / s;
		q[1] = (m[0][1] + m[1][0]) / s;
		q[2] = 0.25 * s;
		q[3] = (m[1][2] + m[2][1]) / s;
	}
	else {
		s = sqrtf(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2;
		q[0] = (m[1][0] - m[0][1]) / s;
		q[1] = (m[0][2] + m[2][0]) / s;
		q[2] = (m[1][2] + m[2][1]) / s;
		q[3] = 0.25 * s;
	}

}

void
surface2trackball(int x, int y, float pos[3]) {

	float norm, tmpX, tmpY, tmpZ, r;

	r = 0.5;		// size of trackball

	tmpX = (2.0*x - (float)winW) / (float)winW;
	tmpY = ((float)winH - 2.0*y) / (float)winH;

	norm = sqrtf(tmpX *tmpX + tmpY * tmpY);

	norm = (norm<r*r) ? norm : r*r;	// limit coordinates
	tmpZ = sqrtf(r*r - norm*norm);


	norm = sqrtf(tmpX *tmpX + tmpY * tmpY + tmpZ * tmpZ);

	pos[0] = tmpX / norm;
	pos[1] = tmpY / norm;
	pos[2] = tmpZ / norm;
}

void
calcRotation(float oldpos[3], float newpos[3], float rotAxis[3], float &rotAngle) {

	rotAxis[0] = newpos[1] * oldpos[2] - newpos[2] * oldpos[1];
	rotAxis[1] = newpos[2] * oldpos[0] - newpos[0] * oldpos[2];
	rotAxis[2] = newpos[0] * oldpos[1] - newpos[1] * oldpos[0];

	rotAngle = acos(oldpos[0] * newpos[0] + oldpos[1] * newpos[1] + oldpos[2] * newpos[2]) * 180.0 / M_PI;

}


/* Compute the quaternion for a given angle and axis */
void
getQuat(float axis[3], float angle, float q[4]) {

	float norm = calcNormal(axis, 3);
	float rdn = angle * (M_PI / 180);

	q[0] = cos(rdn / 2);
	q[1] = (axis[0] / norm) * sin(rdn / 2);
	q[2] = (axis[1] / norm) * sin(rdn / 2);
	q[3] = (axis[2] / norm) * sin(rdn / 2);
}

float
calcNormal(float *vector, int dimension) {

	int i;
	float norm = 0;

	for (i = 0; i < dimension; i++) {
		norm += vector[i] * vector[i];
	}
	norm = sqrtf(norm);
	return norm;
}


/******************************************************************************
Main routine.
******************************************************************************/

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition(20, 20);
	glutCreateWindow(argv[0]);

	init_graphics();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

