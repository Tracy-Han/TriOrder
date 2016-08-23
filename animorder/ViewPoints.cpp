#include "ViewPoints.h"
// coordinates of one of the icosahedron vertex
#define X 0.525731112119133696
#define Z 0.850650808352039932

// icosahedron  vertices
static double icoVerts[12][3] = {
	{ -X, 0.0, Z }, { X, 0.0, Z }, { -X, 0.0, -Z }, { X, 0.0, -Z },
	{ 0.0, Z, X }, { 0.0, Z, -X }, { 0.0, -Z, X }, { 0.0, -Z, -X },
	{ Z, X, 0.0 }, { -Z, X, 0.0 }, { Z, -X, 0.0 }, { -Z, -X, 0.0 }
};

// icosehedron faces
static int icoTri[20][3] = {
	{ 1, 4, 0 }, { 4, 9, 0 }, { 4, 5, 9 }, { 8, 5, 4 }, { 1, 8, 4 },
	{ 1, 10, 8 }, { 10, 3, 8 }, { 8, 3, 5 }, { 3, 2, 5 }, { 3, 7, 2 },
	{ 3, 10, 7 }, { 10, 6, 7 }, { 6, 11, 7 }, { 6, 0, 11 }, { 6, 1, 0 },
	{ 10, 1, 6 }, { 11, 0, 9 }, { 2, 11, 9 }, { 5, 2, 9 }, { 11, 2, 7 }
};


ViewPoints::ViewPoints()
	:_radius(0)
	, _nSub(0)
{
}


ViewPoints::~ViewPoints()
{
	_vertices.clear();
	_faces.clear();
}

void ViewPoints::init(double radius, int nSubdivision)
{
	_radius = radius;
	_nSub = nSubdivision;

	initIco();
	subdivide(nSubdivision);
}

void ViewPoints::initIco()
{
	_vertices.resize(12);
	_faces.resize(20);

	// initialize a fixed icosahefron
	double scale = _radius * 3;
	for (auto i = 0; i < 12; i++){
		_vertices[i] = Vector(icoVerts[i][0], icoVerts[i][1], icoVerts[i][2]) *scale;
	}
	for (auto i = 0; i < 20; i++){
		_faces[i][0] = icoTri[i][0];
		_faces[i][1] = icoTri[i][1];
		_faces[i][2] = icoTri[i][2];
	}
}

void ViewPoints::subdivide(int n)
{
	/*
	* Each Triangle will generate 4 faces
	* 1. use each midpoint of the edges to subdivide
	* 2. because each edge belongs to faces, we'll generate new vertices once (vertexmap will check if that vertex is generated or not)
	*/
	if (n == 0)
		return;

	vector<IcoFace> newFaces;
	newFaces.reserve(_faces.size() * 4);

	// return index of the midpoint vertext index given an edge
	map<Edge, int> vertexmap; //vertex mapping of new

	for (int i = 0; i < _faces.size(); ++i)
	{
		// new vertices
		Vector v[3];
		int vidx[3];
		const IcoFace & face = _faces[i];
		for (int j = 0; j < 3; ++j)
		{
			int v1 = face[j];
			int v2 = face[(j + 1) % 3];
			Edge e = v1<v2 ? Edge(v1, v2) : Edge(v2, v1);
			map<Edge, int>::iterator it = vertexmap.find(e);
			if (it != vertexmap.end()){
				// created before
				v[j] = _vertices[it->second];
				vidx[j] = it->second;
			}
			else {
				Vector midPoint((_vertices[v1] + _vertices[v2]) / 2.0);
				midPoint.normalize();
				v[j] = midPoint * _radius * 3;
				_vertices.push_back(v[j]);
				vidx[j] = _vertices.size() - 1;
				vertexmap[e] = vidx[j];
			}
		}

		// new faces
		for (int j = 0; j < 3; ++j)
		{
			IcoFace newface;
			newface[0] = face[j];
			newface[1] = vidx[j];
			newface[2] = vidx[(j + 2) % 3];
			newFaces.push_back(newface);
		}
		// add center faces
		{
			IcoFace newface;
			newface[0] = vidx[0];
			newface[1] = vidx[1];
			newface[2] = vidx[2];
			newFaces.push_back(newface);
		}
	}

	_faces.assign(newFaces.begin(), newFaces.end());

	subdivide(n - 1);
}

void plyHead(p_ply oply, int numVertices, int numFaces)
{
	ply_add_element(oply, "vertex", numVertices);
	ply_add_scalar_property(oply, "x", PLY_FLOAT);
	ply_add_scalar_property(oply, "y", PLY_FLOAT);
	ply_add_scalar_property(oply, "z", PLY_FLOAT);
	ply_add_element(oply, "face", numFaces);
	ply_add_list_property(oply, "vertex_indices", PLY_UCHAR, PLY_INT);
	ply_write_header(oply);
}
void ViewPoints::saveViewMesh(std::string fileName) const
{
	//string fileName = "viewMesh_" + suffix +".ply";
	const char* cFileName = fileName.c_str();
	p_ply oply;
	oply = ply_create(cFileName, PLY_ASCII, NULL);
	ply_add_comment(oply, "VCGLIB generated");
	if (!oply)
		cout << "Could not create file" << endl;
	auto numVertices = _vertices.size();
	auto numFaces = _faces.size();
	plyHead(oply, numVertices, numFaces);

	//output data
	for (const Vector &vertice : _vertices){
		for (auto i = 0; i < 3; i++)
			ply_write(oply, vertice[i]);
	}
	for (const IcoFace &face : _faces){
		ply_write(oply, 3);
		for (auto i = 0; i < 3; i++)
			ply_write(oply, face[i]);
	}
}

const vector<Vector>& ViewPoints::getViewPoints() const
{
	return _vertices;
}