#include"primitive.h"
#include<sstream>
#include<cstdio>
#include<string>
#include<cmath>
#include<iostream>
#include<cstdlib>
#include <algorithm>
#define ran() ( double( rand() % 32768 ) / 32768 )

const int Bezier_MAX_DEGREE = 5;
const int Combination[Bezier_MAX_DEGREE + 1][Bezier_MAX_DEGREE + 1] =
{	0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 0,
	1, 3, 3, 1, 0, 0,
	1, 4, 6, 4, 1, 0,
	1, 5, 10,10,5, 1
};

const int MAX_COLLIDE_TIMES = 10;
const int MAX_COLLIDE_RANDS = 10;


std::pair<double, double> ExpBlur::GetXY()
{
	double x,y;
	x = ran();
	x = pow(2, x)-1;
	y = rand();
	return std::pair<double, double>(x*cos(y),x*sin(y));
}

Vector3 ExpBlur::GetXYZ()
{
	double a = ran();
	double b = ran();
	double theta = pow(2, a) - 1;
	double phi = 2 * PI * b;
	double x = sin(phi) * cos(theta);
	double y = cos(phi);
	double z = sin(phi) * sin(theta);
	return Vector3(x, y, z);
}

Material::Material() {
	color = absor = Color();
	refl = refr = 0;
	diff = spec = 0;
	rindex = 0;
	drefl = 0;
	texture = NULL;
	blur = new ExpBlur();
}

void Material::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
	if ( var == "absor=" ) absor.Input( fin );
	if ( var == "refl=" ) fin >> refl;
	if ( var == "refr=" ) fin >> refr;
	if ( var == "diff=" ) fin >> diff;
	if ( var == "spec=" ) fin >> spec;
	if ( var == "drefl=" ) fin >> drefl;
	if ( var == "rindex=" ) fin >> rindex;
	if ( var == "texture=" ) {
		std::string file; fin >> file;
		texture = new Bmp;
		texture->Input( file );
	}
	if ( var == "blur=" ) {
		std::string blurname; fin >> blurname;
		if(blurname == "exp")
			blur = new ExpBlur();
	}
}

Primitive::Primitive() {
	sample = rand();
	material = new Material;
	next = NULL;
}

Primitive::Primitive( const Primitive& primitive ) {
	*this = primitive;
	material = new Material;
	*material = *primitive.material;
}

Primitive::~Primitive() {
	delete material;
}

void Primitive::Input( std::string var , std::stringstream& fin ) {
	material->Input( var , fin );
}

Sphere::Sphere() : Primitive() {
	De = Vector3( 0 , 0 , 1 );
	Dc = Vector3( 0 , 1 , 0 );
}

void Sphere::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "De=" ) De.Input( fin );
	if ( var == "Dc=" ) Dc.Input( fin );
	Primitive::Input( var , fin );
}

CollidePrimitive Sphere::Collide( Vector3 ray_O , Vector3 ray_V ) {
	ray_V = ray_V.GetUnitVector();
	Vector3 P = ray_O - O;
	double b = -P.Dot( ray_V );
	double det = b * b - P.Module2() + R * R;
	CollidePrimitive ret;

	if ( det > EPS ) {
		det = sqrt( det );
		double x1 = b - det  , x2 = b + det;

		if ( x2 < EPS ) return ret;
		if ( x1 > EPS ) {
			ret.dist = x1;
			ret.front = true;
		} else {
			ret.dist = x2;
			ret.front = false;
		} 
	} else 
		return ret;

	ret.C = ray_O + ray_V * ret.dist;
	ret.N = ( ret.C - O ).GetUnitVector();
	if ( ret.front == false ) ret.N = -ret.N;
	ret.isCollide = true;
	ret.collide_primitive = this;
	return ret;
}

Color Sphere::GetTexture(Vector3 crash_C) {
	Vector3 I = ( crash_C - O ).GetUnitVector();
	double a = acos( -I.Dot( De ) );
	double b = acos( std::min( std::max( I.Dot( Dc ) / sin( a ) , -1.0 ) , 1.0 ) );
	double u = a / PI , v = b / 2 / PI;
	if ( I.Dot( Dc * De ) < 0 ) v = 1 - v;
	return material->texture->GetSmoothColor( u , v );
}


void Plane::Input( std::string var , std::stringstream& fin ) {
	if ( var == "N=" ) N.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

CollidePrimitive Plane::Collide( Vector3 ray_O , Vector3 ray_V ) {
	ray_V = ray_V.GetUnitVector();
	N = N.GetUnitVector();
	double d = N.Dot( ray_V );
	CollidePrimitive ret;
	if ( fabs( d ) < EPS ) return ret;
	double l = ( N * R - ray_O ).Dot( N ) / d;
	if ( l < EPS ) return ret;

	ret.dist = l;
	ret.front = ( d < 0 );
	ret.C = ray_O + ray_V * ret.dist;
	ret.N = ( ret.front ) ? N : -N;
	ret.isCollide = true;
	ret.collide_primitive = this;
	return ret;
}

Color Plane::GetTexture(Vector3 crash_C) {
	double u = crash_C.Dot( Dx ) / Dx.Module2();
	double v = crash_C.Dot( Dy ) / Dy.Module2();
	return material->texture->GetSmoothColor( u , v );
}

void Square::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "Dx=" ) Dx.Input( fin ); // what is Dx
	if ( var == "Dy=" ) Dy.Input( fin ); // what is Dy
	Primitive::Input( var , fin );
}

CollidePrimitive Square::Collide( Vector3 ray_O , Vector3 ray_V ) {
	CollidePrimitive ret;
	// To get the norm, we need to count (Dx - O) x (Dy - O)
	//NEED TO IMPLEMENT
	ray_V = ray_V.GetUnitVector();
	Vector3 N = (Dx - O) * (Dy - O);
	N = N.GetUnitVector();
	double d = N.Dot(ray_V);
	if (fabs(d) < EPS) return ret;
	double l = N.Dot(O - ray_O) / d;
	if (l < EPS) return ret;

	Vector3 P = ray_O + ray_V * l;
	if (((P - O).Dot(Dx - O) / (Dx - O).Dot(Dx - O) * (Dx - O)).Module2() > (Dx - O).Module2()) {
		return ret;
	}
	if (((P - O).Dot(Dy - O) / (Dy - O).Dot(Dy - O) * (Dy - O)).Module2() > (Dy - O).Module2()) {
		return ret;
	}

	ret.dist = l;
	ret.front = (d < 0);
	ret.C = P;
	ret.N = (ret.front) ? N : -N;
	ret.isCollide = true;
	ret.collide_primitive = this;
	return ret;
}

Color Square::GetTexture(Vector3 crash_C) {
	double u = (crash_C - O).Dot( Dx ) / Dx.Module2() / 2 + 0.5;
	double v = (crash_C - O).Dot( Dy ) / Dy.Module2() / 2 + 0.5;
	return material->texture->GetSmoothColor( u , v );
}

void Cylinder::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O1=" ) O1.Input( fin );
	if ( var == "O2=" ) O2.Input( fin );
	if ( var == "R=" ) fin>>R; 
	Primitive::Input( var , fin );
}

CollidePrimitive Cylinder::Collide( Vector3 ray_O , Vector3 ray_V ) {
	CollidePrimitive ret;

	//NEED TO IMPLEMENT
	//Only half of cylinder is rendered/// FIX IT
	ray_V = ray_V.GetUnitVector();
	Vector3 P = ray_O - O1;
	Vector3 L = (O2 - O1).GetUnitVector();
	double A = (ray_V - L * ray_V.Dot(L)).Module2();
	double B = 2 * (ray_V - L * ray_V.Dot(L)).Dot(P - L * P.Dot(L));
	double C = (P - L * P.Dot(L)).Module2() - R * R;
	//double b = -P.Dot(ray_V);
	//double det = b * b - P.Module2() + R * R;
	double t1 = 1e9;
	double t2 = 1e9;
	double t3 = 1e9;
	double t4 = 1e9;
	double d = 0;

	double det = B * B - 4 * A * C;
	if (det > EPS) {
		det = sqrt(det);
		double x1 = (-B - det) / (2 * A), x2 = (-B + det) / (2 * A);
		if (x2 > EPS) {
			if (x1 > EPS) {
				if ((L.Dot(ray_O + ray_V * x1 - O1) > EPS) && (L.Dot(ray_O + ray_V * x1 - O2) < EPS)) {
					t1 = x1;
				}
			}
			else {
				if ((L.Dot(ray_O + ray_V * x2 - O1) > EPS) && (L.Dot(ray_O + ray_V * x2 - O2) < EPS)) {
					t2 = x2;
				}
			}
		}

		d = L.Dot(ray_V);
		if (fabs(d) > EPS) {
			double l1 = L.Dot(O1 - ray_O) / d;
			Vector3 P1 = ray_O + ray_V * l1;
			double l2 = L.Dot(O2 - ray_O) / d;
			Vector3 P2 = ray_O + ray_V * l2;
			if ((R * R - (P1 - O1).Module2() > EPS) && (l1 > EPS)) {
				t3 = l1;
			}
			if ((R * R - (P2 - O2).Module2() > EPS) && (l2 > EPS)) {
				t4 = l2;
			}
		}
	} else
		return ret;

	if ((t1 < t3) && (t1 < t4)) {
		ret.dist = t1;
		ret.front = true;
		ret.C = ray_O + ray_V * ret.dist;
		Vector3 Ph = (ret.C - O1).Dot(O2 - O1) / (O2 - O1).Module2() * (O2 - O1) + O1;
		ret.N = (ret.C - Ph).GetUnitVector();
		if (ret.front == false) ret.N = -ret.N;
	}
	else if ((t2 < t3) && (t2 < t4)) {

		ret.dist = t2;
		ret.front = false;
		ret.C = ray_O + ray_V * ret.dist;
		Vector3 Ph = (ret.C - O1).Dot(O2 - O1) / (O2 - O1).Module2() * (O2 - O1) + O1;
		ret.N = (ret.C - Ph).GetUnitVector();
		if (ret.front == false) ret.N = -ret.N;
	}
	else if ((t3 < t4)) {
		ret.dist = t3;
		ret.C = ray_O + ray_V * ret.dist;
		ret.front = (d < 0);
		ret.N = (ret.front) ? L : -L;
	}
	else if (t4 < 1e8) {
		ret.dist = t4;
		ret.C = ray_O + ray_V * ret.dist;
		ret.front = (d < 0);
		ret.N = (ret.front) ? L : -L;
		//if (ret.front)
		//{
		//	ret.N = (ret.front) ? L : -L;
		//}
	}
	else {
		return ret;
	}

	ret.isCollide = true;
	ret.collide_primitive = this;
	return ret;
}

Color Cylinder::GetTexture(Vector3 crash_C) {
	//double u = 0.5 ,v = 0.5;
	//NEED TO IMPLEMENT
	double u = 0;
	double v = 0;
	if (((crash_C - O1).Dot(O2 - O1) < EPS && (crash_C - O1).Dot(O2 - O1) > -EPS) 
		|| ((crash_C - O2).Dot(O2 - O1) < EPS && (crash_C - O2).Dot(O2 - O1) > -EPS)) {

		u = crash_C.x - O2.x;
		v = crash_C.y - O2.y;
	}
	else {
		double fi = atan((crash_C.x) / (crash_C.z));
		u = std::fmod(fi, 2.0 * PI);
		v = (crash_C.y - O1.y) / (O2 - O1).Module();
	}

	return material->texture->GetSmoothColor( u , v );
}

void Bezier_slice::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O1=" ) O1.Input( fin );
	if ( var == "O2=" ) O2.Input( fin );
	if ( var == "P=" ) {
		degree++;
		double newR, newZ;
		fin>>newZ>>newR;
		R.push_back(newR);
		Z.push_back(newZ);
	}
	if ( var == "Cylinder" ) {
		double maxR = 0;
		for(int i=0;i<R.size();i++)
			if(R[i] > maxR)
				maxR = R[i];
		boundingCylinder = new Cylinder(O1, O2, maxR);
		N = (O1 - O2).GetUnitVector();
		Nx = N.GetAnVerticalVector();
		Ny = N * Nx;
	}
	Primitive::Input( var , fin );
}

CollidePrimitive Bezier_slice::Collide( Vector3 ray_O , Vector3 ray_V ) {
	CollidePrimitive ret;
	//NEED TO IMPLEMENT
	CollidePrimitive ret1 = boundingCylinder->Collide(ray_O, ray_V);

	if (!ret1.isCollide) return ret;
	//double bez_len = (ret1.C - O1).Dot(O2 - O1) / (O2 - O1).Module2(); // u
	Vector3 z_cur, z_next;
	double r_cur;
	//double R1 = (1 - proj_len) * (1 - proj_len) * Z[0] * R[0] + (1 - proj_len) * Z[1] * R[1] + proj_len * Z[2] * R[2];
	//ret1.C = ret1.C - ret1.N * (R1 / boundingCylinder->R);
	//ret1.dist = ret1.dist - (ret1.N * (R1 / boundingCylinder->R)).Dot(ray_V);
	
	double min_dist = 1e9;
	double step_height = 0.05;
	double tmp_dist;
	double min_u = 0;
	for (double u = 0; u - (1 - step_height) < EPS;) {
		z_cur = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
		r_cur = (1 - u) * (1 - u) * R[0] + 2 * u * (1 - u) * R[1] + u * u * R[2];
		u += step_height;
		z_next = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
		Cylinder* curCylinder = new Cylinder(z_cur, z_next, r_cur);
		tmp_dist = curCylinder->Collide(ray_O, ray_V).dist;
		if (tmp_dist < min_dist) {
			min_dist = tmp_dist;
			min_u = u - step_height;
		}
		delete curCylinder;
	}
	double u = min_u;
	z_cur = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
	r_cur = (1 - u) * (1 - u) * R[0] + 2 * u * (1 - u) * R[1] + u * u * R[2];
	u += step_height;
	z_next = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
	Cylinder* curCylinder = new Cylinder(z_cur, z_next, r_cur);
	ret1 = curCylinder->Collide(ray_O, ray_V);
	delete curCylinder;
	if (ret1.isCollide == false) return ret;
	ret1.collide_primitive = this;
	return ret1;
}

Color Bezier_slice::GetTexture(Vector3 crash_C) {
	// chabuduo ...
	double u = 0;
	double v = 0;
	if (((crash_C - O1).Dot(O2 - O1) < EPS && (crash_C - O1).Dot(O2 - O1) > -EPS)
		|| ((crash_C - O2).Dot(O2 - O1) < EPS && (crash_C - O2).Dot(O2 - O1) > -EPS)) {

		u = (1 + crash_C.x - O1.x) / 2;
		v = (1 + crash_C.y - O1.y) / 2;
	}
	else {
		double fi = atan((crash_C.x) / (crash_C.z));
		u = fi / 2.0 / PI;

		v = (crash_C.y - O1.y + 1) / 2;
	}

	return material->texture->GetSmoothColor(u, v);
}


void Bezier::Input(std::string var, std::stringstream& fin) {
	if (var == "O1=") O1.Input(fin);
	if (var == "O2=") O2.Input(fin);
	if (var == "P=") {
		degree++;
		double newR, newZ;
		fin >> newZ >> newR;
		R.push_back(newR);
		Z.push_back(newZ);
	}
	if (var == "Cylinder") {
		double maxR = 0;
		for (int i = 0; i < R.size(); i++)
			if (R[i] > maxR)
				maxR = R[i];
		boundingCylinder = new Cylinder(O1, O2, maxR);
		N = (O1 - O2).GetUnitVector();
		Nx = N.GetAnVerticalVector();
		Ny = N * Nx;
	}
	Primitive::Input(var, fin);
}

CollidePrimitive Bezier::Collide(Vector3 ray_O, Vector3 ray_V) {
	CollidePrimitive ret;
	//NEED TO IMPLEMENT
	CollidePrimitive ret1 = boundingCylinder->Collide(ray_O, ray_V);

	if (!ret1.isCollide) return ret;
	//double bez_len = (ret1.C - O1).Dot(O2 - O1) / (O2 - O1).Module2(); // u
	Vector3 z_cur, z_next;
	double r_cur;
	//double R1 = (1 - proj_len) * (1 - proj_len) * Z[0] * R[0] + (1 - proj_len) * Z[1] * R[1] + proj_len * Z[2] * R[2];
	//ret1.C = ret1.C - ret1.N * (R1 / boundingCylinder->R);
	//ret1.dist = ret1.dist - (ret1.N * (R1 / boundingCylinder->R)).Dot(ray_V);

	double min_dist = 1e9;
	double step_height = 0.05;
	double tmp_dist;
	double min_u = 0;
	for (double u = 0; u - (1 - step_height) < EPS;) {
		z_cur = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
		r_cur = (1 - u) * (1 - u) * R[0] + 2 * u * (1 - u) * R[1] + u * u * R[2];
		u += step_height;
		z_next = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
		Cylinder* curCylinder = new Cylinder(z_cur, z_next, r_cur);
		tmp_dist = curCylinder->Collide(ray_O, ray_V).dist;
		if (tmp_dist < min_dist) {
			min_dist = tmp_dist;
			min_u = u - step_height;
		}
		delete curCylinder;
	}
	double u = min_u;
	z_cur = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
	r_cur = (1 - u) * (1 - u) * R[0] + 2 * u * (1 - u) * R[1] + u * u * R[2];
	u += step_height;
	z_next = O1 + (O2 - O1) * ((1 - u) * (1 - u) * Z[0] + 2 * u * (1 - u) * Z[1] + u * u * Z[2]);
	Cylinder* curCylinder = new Cylinder(z_cur, z_next, r_cur);
	ret1 = curCylinder->Collide(ray_O, ray_V);
	delete curCylinder;
	if (ret1.isCollide == false) return ret;
	ret1.collide_primitive = this;
	return ret1;
}

Color Bezier::GetTexture(Vector3 crash_C) {
	// chabuduo ...
	double u = 0;
	double v = 0;
	if (((crash_C - O1).Dot(O2 - O1) < EPS && (crash_C - O1).Dot(O2 - O1) > -EPS)
		|| ((crash_C - O2).Dot(O2 - O1) < EPS && (crash_C - O2).Dot(O2 - O1) > -EPS)) {

		u = (1 + crash_C.x - O1.x) / 2;
		v = (1 + crash_C.y - O1.y) / 2;
	}
	else {
		double fi = atan((crash_C.x) / (crash_C.z));
		u = fi / 2.0 / PI;

		v = (crash_C.y - O1.y + 1) / 2;
	}

	return material->texture->GetSmoothColor(u, v);
}