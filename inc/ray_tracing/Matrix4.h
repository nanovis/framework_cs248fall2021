#pragma once
#include "Vec3.h"
#include <iostream>
#include <cmath>

// Vec4 for homogeneous coordinate
class Vec4 {

public:
	double x;
	double y;
	double z;
	double w;
	Vec4() :x(0), y(0), z(0), w(0) {}
	Vec4(double _x, double _y, double _z, double _w) :x(_x), y(_y), z(_z), w(_w) {}
	Vec4(Vec3 a, double _w) :x(a.x), y(a.y), z(a.z), w(_w) {}
	static Vec3 toVec3(Vec4 a)
	{
		return Vec3(a.x, a.y, a.z);
	}
};

// Matrix4 for transformation
class Matrix4 {
public:
	double matrix[4][4];

	Matrix4() {
		identity();
	}
	void identity()
	{
		// #TODO: Implement function to create 4x4 identity matrix
	}
	Matrix4 operator *(Matrix4 a)
	{
		// #TODO: Implement function for matrix multiplication
	}
	Vec4 operator *(Vec4 a)
	{
		// #TODO: Implement function for matrix - vector multiplication
	}
	Matrix4 transpose()
	{
		// #TODO: Implement function to calculate transpose of a matrix
	}
	static Matrix4 Translate(Vec3 t)
	{
		// #TODO: Implement function to calculate translation matrix

	}
	static Matrix4 TranslateInv(Vec3 t)
	{
		// #TODO: Implement function to calculate inverse of translation matrix

	}
	static Matrix4 Scaling(Vec3 t)
	{
		// #TODO: Implement function to calculate scaling matrix

	}
	static Matrix4 ScalingInv(Vec3 t)
	{
		// #TODO: Implement function to calculate inverse of scaling matrix

	}
	static Matrix4 RotationX(double theta)
	{
		// #TODO: Implement function to calculate rotation matrix - X axis

	}
	static Matrix4 RotationXInv(double theta)
	{
		// #TODO: Implement function to calculate inverse of rotation matrix - X axis
	}
	static Matrix4 RotationY(double theta)
	{
		// #TODO: Implement function to calculate rotation matrix - Y axis

	}
	static Matrix4 RotationYInv(double theta)
	{
		// #TODO: Implement function to calculate inverse of rotation matrix - Y axis
	}
	static Matrix4 RotationZ(double theta)
	{
		// #TODO: Implement function to calculate rotation matrix - Z axis

	}
	static Matrix4 RotationZInv(double theta)
	{
		// #TODO: Implement function to calculate inverse of rotation matrix - Z axis
	}

	static Matrix4 Rotation(Vec3 theta)
	{
		// #TODO: Implement function to calculate rotation matrix for three axes
		// theta.x - rotate angle around X axis
		// theta.y - rotate angle around Y axis
		// theta.z - rotate angle around Z axis
	}
	static Matrix4 RotationInv(Vec3 theta)
	{
		// #TODO: Implement function to calculate inverse of rotation matrix for three axes
		// theta.x - rotate angle around X axis
		// theta.y - rotate angle around Y axis
		// theta.z - rotate angle around Z axis
	}
};

std::ostream& operator<< (std::ostream& os, const Matrix4& m)
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			os << m.matrix[i][j] << "\t";
		}
		os << "\n";
	}
	return os;
}