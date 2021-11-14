#pragma once
#include <vector>
using std::vector;
#include "Vec3.h"
#include "Matrix4.h"
#include "Color.h"

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <fstream>
using std::ifstream;
#include <sstream>
using std::istringstream;

// Material for object
class Material
{
public:
    Color Ka; // Ambient coefficient
    Color Kd; // Diffuse coefficient
    Color Ks; // Specular coefficient
    double shininess; // Phong exponent

    Material(Color _ka, Color _kd, Color _ks, double _shininess):
        Ka(_ka), Kd(_kd), Ks(_ks), shininess(_shininess){}  
};

// Each object is a set of triangle
class Triangle
{
public:
    Vec3 v0, v1, v2;                    // Three vertices of triangle
    Vec3 faceNormal;                    // Face normal of triangle
    Vec3 v0Normal, v1Normal, v2Normal;  // Three normals for vertices of triangle
    Color c0, c1, c2;                   // Three color for vertices
    int boudingBox[4];                  //Bouding box of triangle: xmin, ymin, xmax, ymax

    Triangle() {};
    Triangle(Vec3 _v0, Vec3 _v1, Vec3 _v2, Vec3 _faceNormal, Vec3 _v0Normal, Vec3 _v1Normal, Vec3 _v2Normal)
        : v0(_v0), v1(_v1), v2(_v2), faceNormal(_faceNormal), v0Normal(_v0Normal), v1Normal(_v1Normal), v2Normal(_v2Normal)
    {
        boudingBox[0] = 0;
        boudingBox[1] = 0;
        boudingBox[2] = 0;
        boudingBox[3] = 0;
        c0 = Color(1.0f, 0.0f, 0.0f);
        c1 = Color(0.0f, 1.0f, 0.0f);
        c2 = Color(0.0f, 0.0f, 1.0f);
    }

    float min3(const double& a, const double& b, const double& c)
    {
        return std::min(a, std::min(b, c));
    }

    float max3(const double& a, const double& b, const double& c)
    {
        return std::max(a, std::max(b, c));
    } 

    // Use this function for compute bounding box of the triangle
    void calculateBoundingBox()
    {
        boudingBox[0] = std::floor(min3(v0.x, v1.x, v2.x));
        boudingBox[1] = std::ceil(min3(v0.y, v1.y, v2.y));
        boudingBox[2] = std::floor(max3(v0.x, v1.x, v2.x));
        boudingBox[3] = std::ceil(max3(v0.y, v1.y, v2.y));
    }
};

// Each object is represented by a mesh
class Mesh
{
public:
    // Each mesh has these attributes:
    vector<Vec3> points;        // list of vertices
    vector<Vec3> normals;       // list of normal for each vertex
    vector<int> faces;          // list face index
    vector<Vec3> faceNormals;   // list of face normal for each face
    vector<Triangle> triangles; // list of triangles

    // #TODO: Implement function for calculate vertex normal if the mesh does not contain normal for each vertex
    // Reference:  http://web.missouri.edu/~duanye/course/cs4610-spring-2017/assignment/ComputeVertexNormal.pdf
    void generateVertexNormals(
        const vector<Vec3>& points,
        vector<Vec3>& normals,
        const vector<int>& faces)
    {
    }

    // #TODO: Implement function for calculate face normal for each face (triangle) of the mesh
    void generateFaceNormals(
        const vector<Vec3>& points,
        const vector<int>& faces,
        vector<Vec3>& faceNormals)
    {
       
    }

    Mesh()
    {
    }
    Mesh(const char* fileName)
    {
        loadOBJ(fileName);
        
    }

    // #TODO: Implement function to read the data for the mesh from an OBJ file
    void loadOBJ(const char* fileName)
    {

    }
};

