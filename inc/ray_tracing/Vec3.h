#pragma once
#include "Color.h"
#include <iostream>
#include <cmath>

class Vec3 {	
public:
	double x;
	double y;
	double z;
	Vec3(): x(0), y(0), z(0){}
	Vec3(double v) : x(v), y(v), z(v) {}
	Vec3(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
	Vec3(const Vec3 &v) : x(v.x), y(v.y), z(v.z) {}

	
	double vecLength(){
		// #TODO: Implement function to calculate length of vector
	}

	double dotProduct(Vec3 b){
		// #TODO: Implement function to calculate dot product of two vectors: a.b
	}

	Vec3 crossProduct(Vec3 b){
		// #TODO: Implement function to calculate cross product of two vectors: axb
	}

	Vec3 operator*(double b){
		// #TODO: Implement function to calculate vector a multiple with scalar b
	}
	Vec3 operator/(double b) {
		// #TODO: Implement function to calculate vector a divide by scalar b
	}
	
	Vec3 operator+(double b){
		// #TODO: Implement function to calculate element wise addition of vector a and scalar b
	}
	Vec3 operator-(double a){
		// #TODO: Implement function to calculate element wise subtraction of vector a and scalar b
	}
	Vec3 operator+(Vec3 b) {
		// #TODO: Implement function to calculate sum of two vectors: a + b
	}
	Vec3 operator-(Vec3 b){
		// #TODO: Implement function to calculate sum of two vectors: a - b
	}
	Vec3 normalize(){
		// #TODO: Implement function to normalize vector
	}
	Vec3 negate() {
		// #TODO: Implement function to negate vector
	}
};

std::ostream& operator<< (std::ostream& os, const Vec3& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

