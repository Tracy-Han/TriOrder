#ifndef MODEL_H
#define MODEL_H

#include "rply.h"
#include "Vector.h"
#include <vector>
#include <iostream>
#include <string>
using namespace std;

// load model with root
// get center, radius --> viewpoints
// get vertices positions, face indices
class Model
{
protected:

private:
	enum {
		PLY_PX, PLY_PY, PLY_PZ,
	};

	// call back functions
	static int vertex_cb(p_ply_argument argument);

	static int index_cb(p_ply_argument argument);

	void bounds(Vector* center, float* radius) const;
public:
	Model();
	Model(string fileName,int id);
	~Model();
	void loadPly(const char* filename);
public:
	int getNumberofIndices() const;
	int getNumberofVertices() const;
	int getNumberofTrianlges() const;
	const vector<Vector>& getVertices() const;
	const vector<int>& getIndices() const;
	const vector<Vector>& getPatchesPos() const;

	Vector getCenter() const;
	float getRadius() const;
	void moveToOrigin();
	

	void printSummary() const;
	int getId() const;

	void genPatchPos(int numPatches,const vector<int>& linearFace, const vector<int>& patchIndices);
	bool backPatch(const Vector& viewpoint, const vector<int>& linearFace,const vector<int>& patchIndices, int patchId);
private:
	int m_nTriangles;
	vector<Vector> m_patchBuffer;
	vector<Vector> m_vertexBuffer;
	vector<int> m_indexBuffer;
	vector<Vector> m_linearFaceNormal;
	Vector m_center;
	float m_radius;
	char m_modelFullPath[256];
	int _Id;
};

inline int Model::getNumberofIndices() const
{
	return  m_indexBuffer.size();
}

inline int Model::getNumberofVertices() const
{
	return m_vertexBuffer.size();
}

inline int Model::getNumberofTrianlges() const
{
	return m_nTriangles;
}

inline Vector Model::getCenter() const
{
	return m_center;
}

inline float Model::getRadius() const
{
	return m_radius;
}

inline int Model::getId() const
{
	return _Id;
}

#endif

