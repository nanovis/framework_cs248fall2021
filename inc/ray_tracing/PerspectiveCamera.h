#pragma once
#include "Ray.h"
#include "Camera.h"
#include<iostream>
using namespace std;
class PerspectiveCamera : public Camera {
public:
	Vec3 w, u, v;
	double distance = 1.0f; 
	using Camera::Camera;
	
	void setCameraFrame() {
		// #TODO: Set up 3 basis vector for camera
	}

	void getRay(Ray& outRay, double ui, double vj) {
		// #TODO: Get view ray from camera to the pixel
	}
};

std::ostream& operator<< (std::ostream& os, const PerspectiveCamera& camera)
{
	os << "Perspective basis vectors: u = " << camera.u << "; v = " << camera.v << "; w = " << camera.w << "\n";
	return os;
}
