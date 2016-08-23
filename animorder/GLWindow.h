#ifndef MEGLWINDOW_H
#define MEGLWINDOW_H

#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "Vector.h"
#include <vector>
#include <glm\gtc\matrix_transform.hpp>
#include <stdexcept>
using namespace std;
class MeGLWindow
{
protected:
	int _width;
	int _height;
	int _numSlices;// numViews
	bool _offScreen;
	GLuint _fbo;
	GLuint _gColor;
	GLuint _rboDepth;
	GLuint _VAO;
	GLuint _VBO;
	GLuint _transformMatrixBufferID;
	GLuint _indexBufferID;
	GLuint _programID;
	Camera _gCamera;
	GLFWwindow* _window;
	GLuint _atomicBufferID;
	vector<Vector> _pfCameraPositions;
	int _numFaces;
	int _numVertices;
	int _numViews;

private:
	void setBufferObject();
	void installShaders();
	int paintParameter();
	void iniAtomicBuffer();
public:
	MeGLWindow(int numVertices,int numFaces,int numViews, const vector<Vector>& viewPos);
	void initializeGL();
	void setWindowProperty(int x, int y);
	void setOffScreen(bool off);
	void setCamera();
	void setClusterCamera(const vector<vector<int>>& assignments, int frameId, int clusterId);
	void subLoadGeo(const vector<Vector>& pfVertexPositions, const vector<int>& piIndexBuffer);
	void overdrawRatio(vector<float>& sliceRatio);
	void render(int numViews);
	void showGL();
	void teminateGL();
	void setZeroAtomicBuffer();
	GLuint readAtomicBuffer();
};


#endif

