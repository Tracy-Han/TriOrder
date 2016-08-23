#include "GLWindow.h"
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>

GLuint numIndices;
using glm::mat4;
const int NUM_FLOATS_PER_VERTEX = 3;
const int VERTEX_BYTE_SIZ = NUM_FLOATS_PER_VERTEX*sizeof(float);

MeGLWindow::MeGLWindow(int numVertices, int numFaces, int numViews, const vector<Vector>& viewPos)
	:_width(400)
	,_height(400)
	,_numSlices(4)
	,_offScreen(false)
	,_fbo(0)
	,_gColor(0)
	,_rboDepth(0)
	,_VAO(0)
	,_VBO(0)
	,_transformMatrixBufferID(0)
	,_indexBufferID(0)
	,_programID(0)
	, _atomicBufferID(0)
	,_window(NULL)
{
	_numFaces = numFaces;
	_numVertices = numVertices;
	_numViews = numViews;
	_pfCameraPositions.resize(numViews);
	_pfCameraPositions.assign(viewPos.begin(), viewPos.end());
}

void MeGLWindow::initializeGL()
{
	/* initialize window */
	if (!glfwInit())
		return;
	//	GLFWwindow* window;
	if (_offScreen)
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		printf("width: %d height: %d \n", _width, _height);
		_window = glfwCreateWindow(_width, _height, "Hello World", NULL, NULL);
		glfwHideWindow(_window);
	}
	else
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		_window = glfwCreateWindow(_width, _height, "Hello World", NULL, NULL);

	}
	if (!_window)
	{
		glfwTerminate();
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.3?");
	}
	glfwMakeContextCurrent(_window);
	/* initialize glew */
	glewInit();

	paintParameter();
	setBufferObject();
	iniAtomicBuffer();
}

void MeGLWindow::setOffScreen(bool off)
{
	_offScreen = off;
}
void MeGLWindow::setWindowProperty(int x, int y)
{
	int xSlice = ceil(sqrt(_numViews));
	_width = x * xSlice;
	_height = y * xSlice;
	_numSlices = _numViews;
}

void MeGLWindow::setCamera()
{
	/*Camera _gCamera;*/
	_gCamera.setViewportAspectRatio(1.0f*_width / _height);
	_gCamera.setFieldOfView(45.0f);
	_gCamera.setNearAndFarPlanes(0.1f, 2000.0f);

	int NumSlice = ceil(sqrt(_numViews));
	int sliceX, sliceY;
	printf("NumSlice: %u\n", NumSlice);
	float *translatePos = new float[NumSlice];
	memset(translatePos, 0.0f, NumSlice*sizeof(float));
	for (int i = 0; i < NumSlice; i++){
		translatePos[i] = -1 + 1.0 / NumSlice + (2.0 / NumSlice)*i; // original offset plus slot*i 
	}

	mat4 *fullTransformMatrix = new mat4[_numViews];
	memset(fullTransformMatrix, 0.0f, _numViews*sizeof(mat4));
	for (int i = 0; i < _numViews; i++)
	{
		sliceX = i % (int)NumSlice;
		sliceY = i / (int)NumSlice;
		_gCamera.setPosition(glm::vec3(_pfCameraPositions[i].v[0], _pfCameraPositions[i].v[1], _pfCameraPositions[i].v[2]));
		_gCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		fullTransformMatrix[i] =
			glm::translate(glm::mat4(), glm::vec3(translatePos[sliceX], translatePos[sliceY], 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(1.0 / NumSlice, 1.0 / NumSlice, 1.0)) *
			_gCamera.matrix();
	}
	delete[] translatePos;

	/* upload camera*/
	glBindBuffer(GL_ARRAY_BUFFER, _transformMatrixBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _numViews* sizeof(mat4), fullTransformMatrix);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] fullTransformMatrix;
}
void MeGLWindow::setClusterCamera(const vector<vector<int>>& assignments, int frameId, int clusterId)
{
	/*Camera gCamera;*/
	_numSlices = _numViews;
	int NumSlice = ceil(sqrt(_numViews));
	int sliceX, sliceY;

	float *translatePos = new float[NumSlice];
	memset(translatePos, 0.0f, NumSlice*sizeof(float));
	for (int i = 0; i < NumSlice; i++){
		translatePos[i] = -1 + 1.0 / NumSlice + (2.0 / NumSlice)*i; // original offset plus slot*i 
	}

	mat4 *fullTransformMatrix = new mat4[_numViews];
	memset(fullTransformMatrix, 0.0f, _numViews*sizeof(mat4));

	for (int i = 0; i < _numViews; i++)
	{
		sliceX = i % (int)NumSlice;
		sliceY = i / (int)NumSlice;
		if (assignments[frameId][i] == clusterId)
		{
			_gCamera.setPosition(glm::vec3(_pfCameraPositions[i].v[0], _pfCameraPositions[i].v[1], _pfCameraPositions[i].v[2]));
			_gCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
			fullTransformMatrix[i] =
				glm::translate(glm::mat4(), glm::vec3(translatePos[sliceX], translatePos[sliceY], 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(1.0 / NumSlice, 1.0 / NumSlice, 1.0)) *
				_gCamera.matrix();
		}
	}
	delete[] translatePos;

	glBindBuffer(GL_ARRAY_BUFFER, _transformMatrixBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _numViews* sizeof(mat4), fullTransformMatrix);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] fullTransformMatrix;
}
void MeGLWindow::setBufferObject()
{
	/* vertex object */
	glGenBuffers(1, &_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, _numVertices * 3 * sizeof(float), NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZ, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* camera object */
	glGenBuffers(1, &_transformMatrixBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, _transformMatrixBufferID);
	GLuint transformMatrixLocation = glGetAttribLocation(_programID, "fullTransformMatrix");
	GLuint pos0 = transformMatrixLocation + 0;
	GLuint pos1 = transformMatrixLocation + 1;
	GLuint pos2 = transformMatrixLocation + 2;
	GLuint pos3 = transformMatrixLocation + 3;

	glBufferData(GL_ARRAY_BUFFER, _numViews* sizeof(mat4), NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pos0);
	glVertexAttribPointer(pos0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 0));
	glEnableVertexAttribArray(pos1);
	glVertexAttribPointer(pos1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 4));
	glEnableVertexAttribArray(pos2);
	glVertexAttribPointer(pos2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 8));
	glEnableVertexAttribArray(pos3);
	glVertexAttribPointer(pos3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 12));

	glVertexAttribDivisor(pos0, 1);
	glVertexAttribDivisor(pos1, 1);
	glVertexAttribDivisor(pos2, 1);
	glVertexAttribDivisor(pos3, 1);

	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glBindVertexArray(0);

	/* index buffer object */
	glGenBuffers(1, &_indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numFaces * 3 * sizeof(int), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	numIndices = _numFaces * 3;

}
void MeGLWindow::subLoadGeo(const vector<Vector>& pfVertexPositions, const vector<int>& piIndexBuffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _numVertices * 3 * sizeof(float), &pfVertexPositions[0].v[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _numFaces * 3 * sizeof(int), &piIndexBuffer[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeGLWindow::iniAtomicBuffer()
{
	glGenBuffers(1, &_atomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _atomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _atomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}
void MeGLWindow::setZeroAtomicBuffer()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _atomicBufferID);
	GLuint a = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}
GLuint MeGLWindow::readAtomicBuffer()
{
	GLuint userCounter;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _atomicBufferID);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &userCounter);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	//	printf("redPixels: %u\n", userCounter);
	return userCounter;
}
bool checkShaderStatus(GLuint shaderID)
{
	GLint compileStatus;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus != GL_TRUE)
	{
		GLint infoLogLens;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLens);
		GLchar* buffer = new GLchar[infoLogLens];
		GLsizei bufferSize;
		glGetShaderInfoLog(shaderID, infoLogLens, &bufferSize, buffer);
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}
std::string readShaderCode(const char* fileName)
{
	std::ifstream meInput(fileName);
	if (!meInput.good())
	{
		exit(1);
	}

	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>());

}
void MeGLWindow::installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const char* adpater[1];
	std::string temp = readShaderCode("vertexShaderCode.glsl");
	adpater[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adpater, 0);
	temp = readShaderCode("fragmentShaderCode.glsl");
	adpater[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adpater, 0);

	/* how to get error message */
	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);
	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	_programID = glCreateProgram();
	glAttachShader(_programID, vertexShaderID);
	glAttachShader(_programID, fragmentShaderID);
	glLinkProgram(_programID);



	glUseProgram(_programID);
}
void MeGLWindow::overdrawRatio(vector<float>& sliceRatio)
{
	//printf("_width: %u,_height %u\n", _width, _height);

	unsigned char* pixelBuffer = new unsigned char[_width*_height];
	memset(pixelBuffer, 0, _width*_height*sizeof(unsigned char));

	if (_offScreen)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
	}
	else
	{
		glReadBuffer(GL_BACK);
	}
	glReadPixels(0, 0, _width, _height, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);

	// slice
	int xSlice = ceil(sqrt(_numSlices));
	int sliceWidth = _width / xSlice;

	int x, y;
	int drawnPixel; int showedPixel = 0;
	for (int cameraId = 0; cameraId < _numSlices; cameraId++)
	{
		x = cameraId % xSlice; //width
		y = cameraId / xSlice; //height
		drawnPixel = 0; showedPixel = 0;

		for (int i = sliceWidth * y; i < sliceWidth*(y + 1); i++) //height = width
		{
			for (int j = sliceWidth * x; j < sliceWidth*(x + 1); j++)//width
			{
				if ((int)pixelBuffer[i * _width + j] > 1)
				{
					drawnPixel += round((float)pixelBuffer[i * _width + j] / 51.0f);
					showedPixel++;
				}
			}
		}
		sliceRatio[cameraId] = (float)drawnPixel / (float)showedPixel;
		//printf("x : %u ,y : %u , ratio: %f \n", x,y, sliceRatio[cameraId]);
	}

	delete[] pixelBuffer;
}


void MeGLWindow::render(int _numViews)
{
	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
	if (_offScreen)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	}

	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL, _numViews);

}
int MeGLWindow::paintParameter()
{

	if (_offScreen)
	{
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

		glGenRenderbuffers(1, &_gColor);
		glBindRenderbuffer(GL_RENDERBUFFER, _gColor);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RED, _width, _height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _gColor);

		glGenRenderbuffers(1, &_rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, _rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _width, _height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rboDepth);

		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "frame buffer error:" << std::endl;
			exit(EXIT_FAILURE);
		}

	}
	/*load shader*/
	installShaders();

	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	///* load geometry*/
	//loadGeo(pfVertexPositions, _numVertices, piIndexBuffer, _numFaces);

	///* set uniform*/
	//setCamera(_pfCameraPositions,_numViews);

	glViewport(0, 0, _width, _height);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glBindVertexArray(_VAO); // set it in render

	return 1;
}
void MeGLWindow::showGL()
{
	glfwSwapBuffers(_window);
	//glfwPollEvents();
}
void MeGLWindow::teminateGL()
{
	/* finish drawing*/
	glfwTerminate();
	printf("terminate window! ");

}