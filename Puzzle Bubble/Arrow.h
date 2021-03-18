#pragma once
#include "Material.h"
#include "Vector3f.h"

class Arrow{
public:
	Arrow();
	void rotate(int a);
	void draw() const;
	void setCenter(const Vector3f& c);
	float getAngle() const;
	void setAnglespeed(float a);
	Vector3f getCenter() const;

private:
	float angle;
	float anglespeed;
	Vector3f center;
	Material mtl;
};

