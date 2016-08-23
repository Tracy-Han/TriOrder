#include "Model.h"


Model::Model()
	:_Id(0)
	, m_nTriangles(0)
	, m_radius(0.0)
{

}
Model::Model(string fileName,int id)
	:_Id(id)
	, m_nTriangles(0)
	, m_radius(0.0)
{
	const char* cfileName = fileName.c_str();
	this->loadPly(cfileName);
}


Model::~Model()
{
	m_indexBuffer.clear();
	m_vertexBuffer.clear();
	m_patchBuffer.clear();
	m_linearFaceNormal.clear();
}

void Model::loadPly(const char * filename)
{
	strcpy(m_modelFullPath, filename);
	int nVertices, nIndices;
	p_ply ply = ply_open(filename, NULL);
	ply_read_header(ply);
	nVertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, (void*)&m_vertexBuffer, PLY_PX);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, (void*)&m_vertexBuffer, PLY_PY);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, (void*)&m_vertexBuffer, PLY_PZ);
	m_nTriangles = ply_set_read_cb(ply, "face", "vertex_indices", index_cb, (void*)&m_indexBuffer, 0);
	nIndices = 3 * m_nTriangles;
	m_vertexBuffer.resize(nVertices);
	m_indexBuffer.reserve(nIndices);
	ply_read(ply);
	ply_close(ply);

	bounds(&m_center, &m_radius);
}

int Model::vertex_cb(p_ply_argument argument)
{
	long i, j;
	vector<Vector>* ver;
	ply_get_argument_element(argument, NULL, &i);
	ply_get_argument_user_data(argument, (void**)&ver, &j);
	j -= PLY_PX;
	(*ver)[i].v[j] = (double)ply_get_argument_value(argument);
	return 1;
}

int Model::index_cb(p_ply_argument argument)
{
	long i, j;
	vector<int>* ind;
	ply_get_argument_element(argument, NULL, &i);
	ply_get_argument_property(argument, NULL, NULL, &j);
	ply_get_argument_user_data(argument, (void**)&ind, NULL);
	if (j < 0) return 1;
	ind->push_back((int)ply_get_argument_value(argument));
	return 1;
}

void Model::bounds(Vector* center, float* radius) const
{
	double xMax = -std::numeric_limits<double>::max();
	double yMax = -std::numeric_limits<double>::max();
	double zMax = -std::numeric_limits<double>::max();

	double xMin = std::numeric_limits<double>::max();
	double yMin = std::numeric_limits<double>::max();
	double zMin = std::numeric_limits<double>::max();

	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	int numVerts = static_cast<int>(m_vertexBuffer.size());

	for (int i = 0; i < numVerts; ++i)
	{
		x = m_vertexBuffer[i].v[0];
		y = m_vertexBuffer[i].v[1];
		z = m_vertexBuffer[i].v[2];

		if (x < xMin)
			xMin = x;

		if (x > xMax)
			xMax = x;

		if (y < yMin)
			yMin = y;

		if (y > yMax)
			yMax = y;

		if (z < zMin)
			zMin = z;

		if (z > zMax)
			zMax = z;
	}
	double width = xMax - xMin;
	double height = yMax - yMin;
	double length = zMax - zMin;
	
	(*center).v[0] = (xMin + xMax) / 2.0;
	(*center).v[1] = (yMin + yMax) / 2.0;
	(*center).v[2] = (zMin + zMax) / 2.0;
	*radius = sqrt(pow(width, 2.0) + pow(height, 2.0) + pow(length, 2.0)) / 2.0;
}

void Model::printSummary() const
{
	cout << "nVertices=" << getNumberofVertices() << " nFaces=" << getNumberofTrianlges() << endl;
	cout << "center: " << m_center.v[0] << " "<<m_center.v[1]<<" " << m_center.v[2]<<endl;
	cout << "radius: " << m_radius << endl;
}

void Model::moveToOrigin()
{
	// move every frame to center
	for (auto &v : m_vertexBuffer)
		v -= m_center;
	bounds(&m_center, &m_radius);
}

const vector<int>& Model::getIndices() const
{
	return m_indexBuffer;
}

const vector<Vector>& Model::getVertices() const
{
	return m_vertexBuffer;
}

const vector<Vector>& Model::getPatchesPos() const
{
	return m_patchBuffer;
}

void Model::genPatchPos(int numPatches,const vector<int>& linearFace, const vector<int>& patchIndices)
{
	m_patchBuffer.resize(numPatches);
	m_linearFaceNormal.resize(m_nTriangles);
	
	for (auto &v : m_patchBuffer)
		v = Vector(0, 0, 0);

	int cstart = 0, c =0;
	int cnext = patchIndices[1];
	/* compute patch positions */
	float fPArea = 0.0f;
	for (auto i = 0; i <= m_nTriangles; i++){
		if (i == cnext){
			m_patchBuffer[c] /= fPArea * 3.0f;
			c++;
			if (c == numPatches)
				break;
			cstart = i;
			cnext = patchIndices[c + 1];
			fPArea = 0.0f;
		}
		Vector vNormal = cross(m_vertexBuffer[linearFace[i * 3 + 1]]-m_vertexBuffer[linearFace[i * 3 + 0]],
			m_vertexBuffer[linearFace[i * 3 + 2]] - m_vertexBuffer[linearFace[i * 3 + 0]]);
		float fArea = vNormal.length();
		m_linearFaceNormal[i] = vNormal;
		m_linearFaceNormal[i].normalize();
		for (auto j = 0; j < 3; j++)
			m_patchBuffer[c] += m_vertexBuffer[linearFace[i * 3 + j]] * fArea;
		fPArea += fArea;
	}
	// save patchBuffer to compare
}

bool Model::backPatch(const Vector& viewpoint, const vector<int>& linearFace, const vector<int>& patchIndices, int patchId)
{
	bool backPatch = true;
	for (auto i = patchIndices[patchId]; i < patchIndices[patchId + 1]; i++){
		int vid = linearFace[i * 3];
		Vector tempView = viewpoint;
		if (dot(m_linearFaceNormal[i], (tempView - m_vertexBuffer[vid])) > 0){
			backPatch = false;
			break;
		}
	}
	return backPatch;
}
