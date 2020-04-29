#ifndef LIGHT_H
#define LIGHT_H

#include"vector3.h"
#include"color.h"
#include"primitive.h"
#include<sstream>
#include<string>
#include<cmath>

extern const double EPS;

class Light {
protected:
	int sample;
	Color color;
	Light* next;
	Primitive* lightPrimitive;

public:

	Light();
	~Light() {}
	
	int GetSample() { return sample; }
	Color GetColor() { return color; }
	Light* GetNext() { return next; }
	void SetNext( Light* light ) { next = light; }

	virtual bool IsPointLight() = 0;
	virtual void Input( std::string , std::stringstream& );
	virtual Vector3 GetO() = 0;
	virtual double CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality ) = 0;
	virtual Primitive* CreateLightPrimitive() = 0;
};

class PointLight : public Light {
	Vector3 O;
public:
	PointLight() : Light() {}
	~PointLight() {}
	
	bool IsPointLight() { return true; }
	Vector3 GetO() { return O; }
	void Input( std::string , std::stringstream& );
	double CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality );
	Primitive* CreateLightPrimitive(){return NULL;}
};

class SquareLight : public Light {
	Vector3 O;
	Vector3 Dx, Dy;
public:
	SquareLight() : Light() {}
	~SquareLight() {}
	
	bool IsPointLight() { return false; }
	Vector3 GetO() { return O; }
	void Input( std::string , std::stringstream& );
	double CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality );
	Primitive* CreateLightPrimitive();
};

class SphereLight : public Light {
	Vector3 O;
	double R;
public:
	SphereLight() : Light() {}
	~SphereLight() {}
	
	bool IsPointLight() { return false; }
	Vector3 GetO() { return O; }
	void Input( std::string , std::stringstream& );
	double CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality );
	Primitive* CreateLightPrimitive();
};


#endif
