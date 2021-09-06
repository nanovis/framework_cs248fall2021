#pragma once
#include "Vec3.h"
#include "Color.h"
#include "Ray.h"
#include "GeometricObject.h"
#include<cmath>
#include<iostream>

using namespace std;

class Plane : public GeometricObject {
public:
	double A, B, C, D; // Plane equation: Ax + By + Cz + D = 0
	Vec3 Pn; // Normal vector of the plane
	
	// #TODO: You can declare some additional variables for computation in here

	Plane(double _A, double _B, double _C, double _D, Color _color) : GeometricObject(_color), A(_A), B(_B), C(_C), D(_D)
	{
		Pn = Vec3(_A, _B, _C).normalize();
	}
    
    Plane(Vec3 _Pn, double _D, Color _color) : GeometricObject(_color), A(_Pn.x), B(_Pn.y), C(_Pn.z), D(_D), Pn(_Pn.normalize()) {}

	double testIntersection(Ray ray)
	{
		// #TODO: Implement function to check ray intersects with sphere or not, return t.
	}

	Vec3 computeIntersectionPoint(Ray ray, double t)
	{
		// #TODO: Implement function to find intesection point, return intersection point
	}

	Vec3 computeNormalIntersection(Ray ray, double t)
	{
		// #TODO: Implement function to find normal vector at intesection point, return normal vector
	}
};
