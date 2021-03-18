#pragma once
#include "Vector3f.h"
#include "GL/glut.h"

class Timebar{
public:
	Timebar();

	void draw() const;
	float getLength() const;
	void setLength(float l);
	void setCenter(const Vector3f& c);
	float getTime() const;
	void setTime(float t);
	void updateLength(float t);
	Vector3f getCenter() const;

private:
	Vector3f center;
	float length;
	float time;
};

