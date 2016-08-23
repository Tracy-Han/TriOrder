#ifndef REORDER_H
#define REORDER_H

#include "Tipsify.h"
#include "Model.h"
#include "ModelManager.h"
#include "ViewPoints.h"
#include "GLWindow.h"
#include <fstream>
#include <vector>
using namespace std;


class ReOrder
{
private:
	Model*          _model;
	ModelManager*   _modelManager;
	ViewPoints*     _viewPoints;
	MeGLWindow*     _GLWindow;
private:
	float          _lamda;
	int            _CacheSize;
	int            _patchNums;
	int            _numClusters;
	int            _numFrames;
	int            _numViews;
	vector<int>    _linearIndices;
	vector<int>    _patchesOut;
	vector<vector<int>> _assignments;
	vector<vector<float>> _minRatios;
	vector<vector<int>> _means;
	vector<vector<vector<float>>> _ratios;
	vector<Vector> _viewPos;
	vector<int>    _viewIds;
	
private:
	bool compareClusterMean(const vector<int> &tempMean, int clusterId);
	void depthSortPatch(vector<int> &piIndexBufferTmp, const Vector &viewpoint, const vector<Vector> &avgPatchPos);
	void initMeans();
	float makeAssignments();
	bool newClusterMean(int clusterId);
	void loadCameraPos();
public:
	ReOrder(float lamda, int CacheSize, int numClusters, vector<int>& viewIds);
	~ReOrder();
	void initModels(int aniLen, const string &fileName);
	void setLamda(float lamda);
	void vertexCacheOpti();
	void ovrOpti();
	
	
	
};

#endif

