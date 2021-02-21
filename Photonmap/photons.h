#ifndef PHOTON
#define PHOTON


#include <iostream>
#include "icVector.H"
#include "glut.h"


enum objType{ RECTANGLE_OBJ, SPHERE_OBJ};
//enum bounceType{ ABSORB , DIFF_REF, SPEC_REF };

struct photon{

	icVector3	pos; // position
	icVector3	nrg; // power for each color channel
	icVector3	dir; // 
	
	int objIdx;
	// change the copy function if changin something here!

};


struct object {
	objType	type;

	icVector3 diff;
	icVector3 spec;

	/* sphere */
	icVector3 center;
	float radius;

	/* plane */
	icVector3 v1;
	icVector3 v2;
	icVector3 v3;
	icVector3 v4;
	icVector3 normal;

};



double dRand(double dMin, double dMax);
void createRectangle(icVector3 v1, icVector3 v2, icVector3 v3, icVector3 v4, GLfloat diffMat[4], GLfloat specMat[4]);
void createSphere(icVector3 center, float r, int slices, int stacks, GLfloat diffMat[4], GLfloat specMat[4]);
void setLight(int lightID, GLfloat lightPos[4], GLfloat ambientLight[4], GLfloat diffuseLight[4], GLfloat specularLight[4]);
void emitPhotons(icVector3 lightPos, icVector3 lightCol);
void tracePhotons();
bool photonHitsRectangle(photon p, object rec, icVector3 *hitPos);
bool photonHitsSphere(photon p, object sphr, icVector3 *hitPos);
bool bounce(icVector3 d, icVector3 s, photon *p, icVector3 normal, icVector3 hitPos);
void getRandDir(icVector3 *v);
void setMatVector(GLfloat *vec, GLfloat a, GLfloat b, GLfloat c);
void copyPhoton(photon src, photon *dest);
void drawPhotMap();
icVector3 getPhotonsEnergy(icVector3 point, int objIdx, float r);
void image2cam(int xPix, int yPix, icVector3 *WorldPoint);
bool rayHitsRectangle(icVector3 ray, icVector3 start, object rec, icVector3 *hitPos);
bool rayHitsSphere(icVector3 ray, icVector3 start, object sphr, icVector3 *hitPos);
unsigned char* renderPhotonMap();

#endif