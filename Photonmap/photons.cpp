/*

Defines the functions for emitting, tracing, and bouncing photons as well as the functions for rendering the scene.

Arash Shahbaz, 2021

*/
#define NOMINMAX

#include "photons.h"
#include "icVector.H"
#include <algorithm>
//#include <stdlib.h>
#include "glut.h"
#include <limits>
#include <time.h>

const int maxNrPhotons = 1000;             //Max number of Photons Emitted
const int maxNrBounces =6 ;                //Max number of Times Each Photon Bounces

int photonCount=0;
int objCount=0;
unsigned int mapCount = 0;		
extern bool photonMapping;
const int maxNrObjects = 6;
photon photons[maxNrPhotons];
photon photMap[maxNrPhotons*maxNrBounces];
object objects[maxNrObjects];
extern struct kd_node_t *photTree;
extern int winW, winH;
extern icVector3 camPos;

void emitPhotons(icVector3 lightPos, icVector3 lightCol){
																			
	srand(0);
	icVector3 dir;
	while (photonCount <maxNrPhotons) {
		
		getRandDir(&dir);

		photons[photonCount].dir.set(dir);
		photons[photonCount].pos.set(lightPos);
		photons[photonCount].nrg.set(lightCol);
		photons[photonCount].nrg /= (double)maxNrPhotons;

		photonCount++;
	}
}

void tracePhotons() {

	bool absorbed = false;
	int i, j, objIndex=-1, bounces = 0;
	double dist= std::numeric_limits<double>::max();
	icVector3 tmpPos, hitPos;

	for (i = 0; i < photonCount; i++) {
		while (!absorbed && bounces < maxNrBounces) {			
			for (j = 0; j < objCount; j++) {

				if (objects[j].type == RECTANGLE_OBJ) {
					if (photonHitsRectangle(photons[i], objects[j], &tmpPos)) { 
						
						if (length(photons[i].pos - tmpPos) < dist && length(photons[i].pos - tmpPos) != 0){
							dist = length(photons[i].pos - tmpPos);
							hitPos = tmpPos;
							objIndex = j;
						}
					}
				}
				else if (objects[j].type == SPHERE_OBJ) {
					if (photonHitsSphere(photons[i], objects[j], &tmpPos)) {
						if (length(photons[i].pos - tmpPos) < dist && length(photons[i].pos - tmpPos) != 0) {
							dist = length(photons[i].pos - tmpPos);
							hitPos = tmpPos;
							objIndex = j;
						}
					}
				}
			}

			if (objIndex > -1) {

				photons[i].objIdx = objIndex;
				absorbed = bounce(objects[objIndex].diff, objects[objIndex].spec, photons + i, objects[objIndex].normal, hitPos);

				bounces++;
				objIndex = -1;
				dist = std::numeric_limits<double>::max();
			}
			else {
				i++;
				objIndex = -1;
				dist = std::numeric_limits<double>::max();
			}
		}
		bounces = 0;
		absorbed = false;
	}
}

bool bounce(icVector3 d, icVector3 s, photon *p, icVector3 normal, icVector3 hitPos){	

	bool absorbed = false;
	//bounceType type;
			
	icVector3 r = d + s;
	//normalize(r);
	float P_r = std::max(r.x, std::max(r.y, r.z)); // probability of reflection
	
	float P_d = P_r * (d.x + d.y + d.z) / (r.x + r.y + r.z);
	float P_s = P_r - P_d;
	double rnd = dRand(0.0, 1.0);

	if (rnd < P_d) {	// diffuse
		p->nrg.set(p->nrg.x * d.x / P_d, p->nrg.y * d.y / P_d, p->nrg.z * d.z / P_d);
		p->pos.set(hitPos);
		copyPhoton(*p, photMap + mapCount);
		mapCount++;
	}
	else if (rnd >= P_d && rnd < P_d + P_s) { // specular									
		p->nrg.set(p->nrg.x * s.x / P_s, p->nrg.y * s.y / P_s, p->nrg.z * s.z / P_s);		
		p->pos.set(hitPos);	
	}																	
	else { //absorption																	
		absorbed = true;
		p->nrg.set(0.,0.,0.);		
		p->pos.set(hitPos);
	}

	getRandDir(&(p->dir));	
	if (dot(p->dir, normal) < 0.0)		// bounce only to the upper hemisphere
		p->dir.set(-(p->dir));

	return absorbed;
}


unsigned char* renderPhotonMap () {
	unsigned char * buff=new unsigned char[winW*winH*3];
	icVector3 ray, tmpPos, hitPos, energy;
	double dist = std::numeric_limits<double>::max();

	int i, j, k, objIndex = -1;

	for (i = 0; i < winW; i++) {
		for (j = 0; j < winH; j++) {
			image2cam(i, winH - j, &ray);
			normalize(ray);
			for (k = 0; k < objCount; k++) {
				if (objects[k].type == RECTANGLE_OBJ) {
					if (rayHitsRectangle(ray, camPos, objects[k], &tmpPos)) {
							dist = length(camPos - tmpPos);
							hitPos = tmpPos;
							objIndex = k;
					}
				}
				else if (objects[k].type == SPHERE_OBJ) {
					if (rayHitsSphere(ray, camPos, objects[k], &tmpPos)) {
						if (length(camPos - tmpPos) < dist ) {
							dist = length(camPos - tmpPos);
							hitPos = tmpPos;
							objIndex = k;
						}
					}
				}
			}
			if (objIndex > -1) {
					energy = getPhotonsEnergy(hitPos, objIndex, 10.);

				buff[3*(j*winW + i)] = (unsigned char) std::min(1.,energy.x) * 255;
				buff[3*(j*winW + i )+ 1] = (unsigned char)std::min(1., energy.y) * 255;
				buff[3*(j*winW + i )+ 2] =  (unsigned char)std::min(1., energy.z) * 255;
				
				objIndex = -1;
				dist = std::numeric_limits<double>::max();
			}
		}
	}
	return buff;
}

bool rayHitsRectangle(icVector3 ray, icVector3 start, object rec, icVector3 *hitPos) {


	if (dot(ray, rec.normal) == 0)  // ray paralell to rectangle
		return false;

	icVector3 point = (dot(rec.v1 - start, rec.normal) / dot(ray, rec.normal))*ray + start; // intersection

	icVector3 AB = rec.v2 - rec.v1;
	icVector3 AD = rec.v4 - rec.v1;
	icVector3 CB = rec.v2 - rec.v3;
	icVector3 CD = rec.v4 - rec.v3;
	icVector3 AP = point - rec.v1;
	icVector3 CP = point - rec.v3;

	normalize(AB); normalize(AD);normalize(CB); normalize(CD); normalize(AP);normalize(CP);

	if (dot(AP, AB) >= 0 && dot(AP, AD) >= 0 && dot(CP, CB) >= 0 && dot(CP, CD) >= 0) {	// within the boundaries
		hitPos->set(point);
		return true;
	}

	return false;
}

bool rayHitsSphere(icVector3 ray, icVector3 start, object sphr, icVector3 *hitPos) {

	float a = dot(ray, ray);
	float b = 2. * (dot(ray, start - sphr.center));
	float c = dot(start - sphr.center, start - sphr.center) - (sphr.radius*sphr.radius);
	float d = b*b - 4. * a*c;

	if (d < 0) {		// no intersection
		return false;
	}
	else {				// intersection at possibbly two points
		icVector3 x1, x2;
		x1.set(((-b + sqrt(d)) / (2.*a)) * ray + start);
		x2.set(((-b - sqrt(d)) / (2.*a)) * ray + start);

		if (length(x1 - start) < length(x2 - start))
			hitPos->set(x1);
		else
			hitPos->set(x2);
	}

	return true;

}

bool photonHitsRectangle(photon p, object rec, icVector3 *hitPos) {


	if (dot(p.dir, rec.normal) == 0)  // ray paralell to rectangle
		return false;
	
	icVector3 point = (dot(rec.v1 - p.pos, rec.normal) / dot(p.dir, rec.normal))*p.dir + p.pos; // intersection
	
	icVector3 AB = rec.v2 - rec.v1;
	icVector3 AD = rec.v4 - rec.v1;
	icVector3 CB = rec.v2 - rec.v3;
	icVector3 CD = rec.v4 - rec.v3;
	icVector3 AP = point - rec.v1;
	icVector3 CP = point - rec.v3;

	normalize(AB); normalize(AD); normalize(CB); normalize(CD); normalize(AP); normalize(CP);

	if (dot(AP, AB) >= 0 && dot(AP, AD) >= 0 && dot(CP, CB) >= 0 && dot(CP, CD) >= 0) {	// within the boundaries?
		hitPos->set(point);
		return true;
	}

	return false;
}


bool photonHitsSphere(photon p, object sphr, icVector3 *hitPos) {

	float a = dot(p.dir, p.dir);
	float b = 2 * (dot(p.dir, p.pos - sphr.center));
	float c = dot(p.pos - sphr.center, p.pos - sphr.center) - (sphr.radius*sphr.radius);
	float d = b*b - 4 * a*c;

	if (d < 0) {		// no intersection
		return false;
	}
	else {				// intersection at possibbly two points
		icVector3 x1, x2;
		x1.set(( (-b + sqrt(d)) / (2.*a) ) * p.dir + p.pos);
		x2.set(( (-b - sqrt(d)) / (2.*a) ) * p.dir + p.pos);

		if (length(x1 - p.pos) < length(x2 - p.pos))
			hitPos->set(x1);
		else 
			hitPos->set(x2);
	}

	return true;
}


void image2cam(int xPix, int yPix, icVector3 *WorldPoint) {
	double mvMat[16], projMat[16], point3d[3];
	int viewportMat[4];
	float depth;

	glGetDoublev(GL_MODELVIEW_MATRIX, mvMat);
	glGetDoublev(GL_PROJECTION_MATRIX, projMat);
	glGetIntegerv(GL_VIEWPORT, viewportMat);

	gluUnProject(xPix, winH - yPix, 0., mvMat, projMat, viewportMat, point3d, (point3d + 1), (point3d + 2));
	WorldPoint->x = point3d[0];
	WorldPoint->y = point3d[1];
	WorldPoint->z = point3d[2];
}

icVector3 getPhotonsEnergy(icVector3 point, int objIdx, float r) {

	icVector3 nrg(0., 0., 0.);
	int i, j;
	double d = -1;
	int counter = 0;
	int neighborNr;
	float maxR;

	for (i = 0; i < mapCount; i++) {
		if (photMap[i].objIdx == objIdx) {
			d = length(photMap[i].pos - point);
			if (d < r) {
				nrg += (1. - (d / (double)r))*photMap[i].nrg;
				counter++;
			}
		}
	}

	neighborNr = 15;
	maxR = 50;
	if (counter <neighborNr && r < maxR) {
		nrg = getPhotonsEnergy(point, objIdx, 1.5 * r);
	}
	else {
		nrg *= (float(neighborNr)*maxNrPhotons/(.25*std::max(1.f,float(counter))));
	}

	return nrg;

}


void createRectangle(icVector3 v1, icVector3 v2, icVector3 v3, icVector3 v4, GLfloat diffMat[4], GLfloat specMat[4]) {

	
	objects[objCount].type = RECTANGLE_OBJ;
	
	objects[objCount].v1 = v1;
	objects[objCount].v2 = v2;
	objects[objCount].v3 = v3;
	objects[objCount].v4 = v4;

	objects[objCount].normal = cross(v2 - v1 , v3 - v1);
	normalize(objects[objCount].normal);

	objects[objCount].diff.set(diffMat[0], diffMat[1], diffMat[2]);
	objects[objCount].spec.set(specMat[0], specMat[1], specMat[2]);

	objCount++;


	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffMat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specMat);

	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v4.x, v4.y, v4.z);

}

void createSphere(icVector3 center, float r, int slices, int stacks, GLfloat diffMat[4], GLfloat specMat[4]) {

	
	objects[objCount].type = SPHERE_OBJ;
	
	objects[objCount].center.set(center);
	objects[objCount].radius=r;

	objects[objCount].diff.set(diffMat[0], diffMat[1], diffMat[2]);
	objects[objCount].spec.set(specMat[0], specMat[1], specMat[2]);

	objCount++;

	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffMat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specMat);

	glPushMatrix();
//	glLoadIdentity();
	glTranslatef(center.x, center.y, center.z);
	glutSolidSphere(r, slices, stacks);

	glPopMatrix();

}

void setLight(int lightID,GLfloat lightPos[4], GLfloat ambientLight[4], GLfloat diffuseLight[4], GLfloat specularLight[4]){
	glLightfv(lightID, GL_POSITION, lightPos);

	glLightfv(lightID, GL_AMBIENT, ambientLight);
	glLightfv(lightID, GL_DIFFUSE, diffuseLight);
	glLightfv(lightID, GL_SPECULAR, specularLight);

	glEnable(lightID);

	if (photonMapping) {
		icVector3 pos(lightPos[0], lightPos[1], lightPos[2]);
		icVector3 col(diffuseLight[0], diffuseLight[1], diffuseLight[2]);

		emitPhotons(pos, col);
	}
}

void copyPhoton(photon src, photon *dest) {
	
	dest->pos.set(src.pos);
	dest->nrg.set(src.nrg);
	dest->dir.set(src.dir);
	dest->objIdx = src.objIdx;

}

void setMatVector(GLfloat *vec, GLfloat a, GLfloat b, GLfloat c) {
	vec[0] = a;
	vec[1] = b;
	vec[2] = c;

}

void getRandDir(icVector3 *v) {
	do {
		v->set(dRand(-1.0, 1.0), dRand(-1.0, 1.0), dRand(-1.0, 1.0));
	} while (length(*v) > 1.);

}

double dRand(double dMin, double dMax){
	double d = (double)rand() / RAND_MAX;
	return dMin + d * (dMax - dMin);

}

void drawPhotMap() {
		
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	
	glMatrixMode(GL_MODELVIEW);

	float a = maxNrPhotons;
	glPushMatrix();
	glPointSize(8.0);
		glBegin(GL_POINTS);	

		for (unsigned int i = 0; i < mapCount; i++) {
			if (photMap[i].pos.z != 80) {
				glLoadIdentity();

					glTranslatef(photMap[i].pos.x, photMap[i].pos.y, photMap[i].pos.z);
					glColor3f(a*photMap[i].nrg.x, a*photMap[i].nrg.y, a*photMap[i].nrg.z);
					glVertex3f(photMap[i].pos.x, photMap[i].pos.y, photMap[i].pos.z);		
			}
		}
		glEnd();
	glPopMatrix();

	

	glEnable(GL_LIGHTING);
}
