/* Base class for two types of camera*/
#pragma once
#include "Vec3.h"
#include "Matrix4.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
using namespace std;

class Camera{
public:
	Vec3 eye_position;
	Vec3 gaze_direction;
	Vec3 view_up;
	Vec3 u, v, w;

	// #TODO: You can declare some additional variables for computation in here

	Camera(Vec3 _eye_position, Vec3 _gaze_direction, Vec3 _view_up): eye_position(_eye_position), gaze_direction(_gaze_direction), view_up(_view_up) {
	}

	// #TODO: Compute u, v, w for camera
	void setCameraFrame() {
		w = gaze_direction.normalize().negate();
		u = view_up.crossProduct(w).normalize();
		v = w.crossProduct(u).normalize();
	}

	// #TODO: Compute View matrix for Camera transformation: M_cam
	Matrix4 calculateViewMatrix() {
		Matrix4 m_cam;

		return m_cam;
	}

	// #TODO: Compute orthographic projection matrix for Projection transformation: M_ortho
	Matrix4 calculateOrthographicMatrix(double l, double r, double b, double t, double n, double f) {
		Matrix4 m_orth;

		return m_orth;
	}

	// #TODO: Compute perspective projection matrix for Projection transformation: M_per
	// Note: To make sure that there is no distortion of shape in the image, using Field-of-View to compute perspective projection matrix
	Matrix4 calculatePerspectiveMatrix(double fovy, double aspect, double n, double f) {
		Matrix4 m_per;
		return m_per;
	}

	// #TODO: Compute perspective projection matrix for Projection transformation using left, right, bottom, top: M_per
	Matrix4 calculatePerspectiveMatrix(double l, double r, double b, double t, double n, double f) {
		Matrix4 m_per;
		return m_per;
	}
};



