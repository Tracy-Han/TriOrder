#ifndef  VIEWPOINTS_H
#define VIEWPOINTS_H

#include "rply.h"
#include "Vector.h"
#include <vector>
#include <set>
#include <map>
#include <iostream>
using namespace std;

class ViewPoints
{
	struct IcoFace
	{
		int& operator[](int i) { return d_v[i]; }
		const int& operator[](int i) const { return d_v[i]; }
		int d_v[3];   // vertex
	};
	typedef std::pair<int, int> Edge;

private:
	void initIco();
	void subdivide(int n);
private:
	vector<Vector> _vertices;
	vector<IcoFace> _faces;
	double _radius;
	int _nSub;
public:
	ViewPoints();
	~ViewPoints();
	void init(double radius, int nSubdivision);
	const vector<Vector>& getViewPoints() const;
	void saveViewMesh(std::string fileName) const;
	int getNumberofVertices();
};

inline int ViewPoints::getNumberofVertices()
{
	return _vertices.size();
}
#endif