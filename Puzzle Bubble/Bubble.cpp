#include "Bubble.h"
#include <math.h>

Bubble::Bubble(int sl, int st) {
	velocity.setPos(0, 0, 0);

	setRadius(20);
	setSlice(sl);
	setStack(st);
	setChecked(false);
	setMarked(false);
}

Bubble::~Bubble(){}

void Bubble::setRadius(float r) { radius = r; }
float Bubble::getRadius() const { return radius; }
void Bubble::setSlice(float sl) { slice = sl; }
void Bubble::setStack(float st) { stack = st; }

void Bubble::setCenter(const Vector3f& c) { center = c; }
Vector3f Bubble::getCenter() const { return center; }
void Bubble::setVelocity(const Vector3f& v) { velocity = v; }
Vector3f Bubble::getVelocity() const { return velocity; }


void Bubble::setChecked(bool a) { checked = a; }
bool Bubble::getChecked() const { return checked; }

void Bubble::setMarked(bool a) { marked = a; }
bool Bubble::getMarked() const { return marked; }

Material Bubble::getMtl() const { return mtl; }

void Bubble::setColorID(int a) { colorID = a; }
int Bubble::getColorID() const { return colorID; }

void Bubble::move(){
	setCenter(center + velocity);
	if (center[0] + radius > 160 || center[0] - radius < -160) {
		setVelocity(Vector3f(-velocity.getX(), velocity.getY(), velocity.getZ()));
	}
}

void Bubble::setMTL(const Material& m) { mtl = m; }

void Bubble::draw() const{
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_EMISSION, mtl.getEmission());
	glMaterialfv(GL_FRONT, GL_AMBIENT, mtl.getAmbient());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl.getDiffuse());
	glMaterialfv(GL_FRONT, GL_SPECULAR, mtl.getSpecular());
	glMaterialfv(GL_FRONT, GL_SHININESS, mtl.getShininess());
//	glColor4f(color[0], color[1], color[2],0.5);
	glPushMatrix();
	glTranslatef(center.getX(),center.getY(),center.getZ());
	glutSolidSphere(radius, slice, stack);
	glPopMatrix();
}

void Bubble::launch(float angle, int s) {
	Vector3f vel(-s*4*cos((angle+90) * 3.141592 / 180),s*4*sin((angle+90) * 3.141592 / 180),0 );
	setVelocity(vel);
}