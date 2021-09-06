#pragma once
#include "Vec3.h"
#include "Color.h"
#include "Ray.h"
#include "GeometricObject.h"
#include<cmath>
#include<iostream>

using namespace std;
	
class Sphere: public GeometricObject {
public:
	Vec3 center; // center point of the sphere
	double radius; // radius of the sphere
	
	// #TODO: You can declare some additional variables for computation in here

	Sphere(Vec3 _center, double _radius, Color _color) : GeometricObject(_color), center(_center), radius(_radius){}

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
