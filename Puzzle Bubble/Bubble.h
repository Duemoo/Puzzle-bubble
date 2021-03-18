#pragma once
#include "Material.h"
#include "Vector3f.h"

class Bubble
{
public:
	Bubble(int sl, int st);
	~Bubble();

	void setRadius(float r);
	float getRadius() const;
	void setSlice(float sl);
	void setStack(float st);
	void setCenter(const Vector3f& c);
	Vector3f getCenter() const;
	void setVelocity(const Vector3f& v);
	Vector3f getVelocity() const;
	void setChecked(bool a);
	bool getChecked() const;
	void setMarked(bool a);
	bool getMarked() const;

	Material getMtl() const;

	int getColorID() const;
	void setColorID(int a);


	void move();
	void launch(float angle, int s);
	void setMTL(const Material& m);
	void draw() const;

private:
	float radius;
	int slice;
	int stack;
	bool checked;
	bool marked;
	int colorID;

	Vector3f center;
	Vector3f velocity;
	Material mtl;
};

