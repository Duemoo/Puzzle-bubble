#include "Arrow.h"

Arrow::Arrow() {
	center.setPos(0, -250, 0);
	angle = 0;
	anglespeed = 4.3;

	mtl.setEmission(0.1, 0.1, 0.1, 1);
	mtl.setAmbient(0.3, 0.3, 0.3, 1);
	mtl.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl.setSpecular(1.0, 1.0, 1.0, 1);
	mtl.setShininess(10);
}

void Arrow::setCenter(const Vector3f& c) { center = c; }
Vector3f Arrow::getCenter() const { return center; }
float  Arrow::getAngle()const { return angle; }
void Arrow::setAnglespeed(float a) { anglespeed = a; }

void Arrow::rotate(int a) {
	if (a == 1) {
		if (angle + 4 >= 85) angle = 85;
		else angle += anglespeed;
	}
	else if (a == -1) {
		if (angle - 4 <= -85) {
		angle = -85;
		}
		else angle -= anglespeed;
	}
}

void Arrow::draw() const{
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_EMISSION, mtl.getEmission());
	glMaterialfv(GL_FRONT, GL_AMBIENT, mtl.getAmbient());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl.getDiffuse());
	glMaterialfv(GL_FRONT, GL_SPECULAR, mtl.getSpecular());
	glMaterialfv(GL_FRONT, GL_SHININESS, mtl.getShininess());
	glPushMatrix();
	glTranslatef(center[0], center[1], center[2]);
	glutSolidSphere(10,30,30);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(center[0], center[1], center[2]);
	glRotatef(-90, 1, 0, 0);
	glRotatef(angle, 0, 1, 0);
	//glTranslatef(5,5,0);
	glutSolidCone(10, 50, 30, 30);
	glPopMatrix();

}