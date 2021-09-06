#pragma once
#include "Vec3.h"
#include "Color.h"
#include "Ray.h"
#include "GeometricObject.h"
#include<cmath>
#include<iostream>

using namespace std;
	
class Ellipsoid : public GeometricObject {
public:
	double aRadius; // radius along x axis
	double bRadius; // radius along y axis
	double cRadius; // radius along z axis
	Vec3 center; // center of ellipsoid

	// #TODO: You can declare some additional variables for computation in here
	
	Ellipsoid(Vec3 _center, double _aRadius, double _bRadius, double _cRadius, Color _color) : GeometricObject(_color), center(_center), aRadius(_aRadius), bRadius(_bRadius), cRadius(_cRadius) {}
	
	double testIntersection(Ray ray)
	{
		// #TODO: Implement function to check ray intersects with sphere or not, return t
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

