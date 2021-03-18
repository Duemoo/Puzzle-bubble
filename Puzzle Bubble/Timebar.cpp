#include "Timebar.h"

Timebar::Timebar() {
	length = 63;
	time = 10;
	setCenter(Vector3f(-300,310,0));
};

float Timebar::getLength() const { return length; }
void Timebar::setLength(float l) { length = l; }
void Timebar::setCenter(const Vector3f& c) { center = c; }
Vector3f Timebar::getCenter() const { return center; }

float Timebar::getTime() const { return time; }
void Timebar::setTime(float t) { 
	if (t < 2.0) { time = 2.0; }
	else time = t;
}
void Timebar::updateLength(float t){}

void Timebar::draw() const {
	glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex3f(57, 230.f, 0.f);
		glVertex3f(57 + length, 230.f, 0.f);
		glVertex3f(57 + length, 224.f, 0.f);
		glVertex3f(57.f, 224.f, 0.f);
	glEnd();
}

//initial length: 63
