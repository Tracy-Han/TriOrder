#include "ReOrder.h"
#define MAXIteration 10

class patchSort
{
public:
	float dist;// distance
	int id;//index
};
class clusterAssign
{
public:
	int frameId;
	int viewId;
};
bool sortPatch(const patchSort &a, const patchSort &b)
{
	return a.dist < b.dist;
}
ReOrder::ReOrder(float lamda, int CacheSize, int numClusters, vector<int>& viewIds)
	: _model(NULL)
	, _lamda(lamda)
	, _CacheSize(CacheSize)
	, _numClusters(numClusters)
	, _patchNums(0)
	, _modelManager(NULL)
{
	_viewIds.resize(numClusters);
	_viewIds.assign(viewIds.begin(), viewIds.end());
}


ReOrder::~ReOrder()
{
	_linearIndices.clear();
	_patchesOut.clear();
	_assignments.clear();
	_minRatios.clear();
	_ratios.clear();
	_means.clear();

	if (_modelManager)
		delete _modelManager;
	if (_viewPoints)
		delete _viewPoints;
	if (_GLWindow)
		delete _GLWindow;
	_model = NULL;
}
void ReOrder::loadCameraPos()
{
	FILE *myFile;
	string cameraPath = "viewpoints.txt";
	const char* cCameraPath = cameraPath.c_str();
	myFile = fopen(cCameraPath, "r");
	for (int i = 0; i < 162; i++){
		for (int j = 0; j < 3; j++)
			fscanf(myFile, "%f \n", &(_viewPos[i].v[j]));
	}
	fclose(myFile);
}
void ReOrder::initModels(int aniLen, const string &fileName)
{
	_numFrames = aniLen;
	_modelManager = new ModelManager(aniLen,fileName);
	double radius = 0;
	for (auto i = 0; i < _numFrames; i++){
		_model = _modelManager->getModel(i);
		double tempRadius = _model->getRadius();
		if (radius < tempRadius)
			radius = tempRadius;
	}
	
	_viewPoints = new ViewPoints( );
	_viewPoints->init(radius, 2);
	_viewPos = _viewPoints->getViewPoints();
	//loadCameraPos();

	int iNumVertices = _model->getNumberofVertices();
	int iNumFaces = _model->getNumberofTrianlges();
	_numViews = _viewPoints->getNumberofVertices();
	_GLWindow = new MeGLWindow(iNumVertices,iNumFaces,_numViews,_viewPos);
	_GLWindow->setOffScreen(true);
	_GLWindow->setWindowProperty(400,400 );
	_GLWindow->initializeGL();
	
	_assignments.resize(_numFrames, vector<int>(_numViews, 0));
	_minRatios.resize(_numFrames, vector<float>(_numViews, 0.0f));
	_ratios.resize(_numFrames);
	for (auto i = 0; i < _numFrames; i++)
		_ratios[i].resize(_numClusters, vector<float>(_numViews, 0.0f));
}
void ReOrder::vertexCacheOpti()
{
	int iNumVertices = _model->getNumberofVertices();
	int iNumFaces = _model->getNumberofTrianlges();
	vector<int> IndiceBuffer = _model->getIndices();
	//set indexBufferIn
	int *piIndexBufferIn = & IndiceBuffer[0];

	int *piIndexBufferOut = new int[iNumFaces*3];
	int *piClustersOut = new int[iNumFaces+1];
	int *piClustersTmp = new int[iNumFaces+1];
	int  *piScratch = NULL;
	FanVertOptimizeVCacheOnly(piIndexBufferIn,piIndexBufferOut,iNumVertices,iNumFaces,_CacheSize,piScratch,piClustersTmp,&_patchNums);
	FanVertOptimizeClusterOnly(piIndexBufferOut,iNumVertices,iNumFaces,_CacheSize,_lamda,piClustersTmp,_patchNums,piClustersOut,&_patchNums,piScratch);
	cout << "lamda set " << _patchNums << " patches"<<endl;

	// save array to vector 
	_linearIndices.resize(iNumFaces*3);
	_linearIndices.assign(piIndexBufferOut, piIndexBufferOut + iNumFaces*3);
	int patchLen = _patchNums + 1;
	_patchesOut.resize(patchLen);
	_patchesOut.assign(piClustersOut,piClustersOut+patchLen);
	
	delete[] piIndexBufferOut;
	delete[] piClustersOut;
	delete[] piClustersTmp;
}

void ReOrder::ovrOpti()
{
	initMeans();
	vector<float> updateRatios;
	for (auto iter = 0; iter < MAXIteration; iter++){
		_GLWindow->setCamera();
		for (auto i = 0; i < _numFrames; i++){
			Model* model = _modelManager->getModel(i);
			for (auto j = 0; j < _numClusters; j++){
				_GLWindow->subLoadGeo(model->getVertices(), _means[j]);
				_GLWindow->render(_numViews);
				_GLWindow->overdrawRatio(_ratios[i][j]);
				_GLWindow->showGL();
			}
		}
		updateRatios.push_back(makeAssignments());
		cout << updateRatios[iter] << endl;
		if (updateRatios.size() > 1){
			if (updateRatios[iter - 1] - updateRatios[iter] < 0.001){
				printf("updateRatio: %f\n", updateRatios[iter]);
				printf("update ratio is too small, finish iteration! \n");
				break;
			}
		}

		bool move = false;
		for (auto clusterId = 0; clusterId < _numClusters; clusterId++){
			bool tempMove = newClusterMean(clusterId);
			if (tempMove == true)
				move = true;
		}
		if (move == false){
			cout << "no cluster index buffer changes, finish iteration!" << endl;
			break;
		}
	}
	_GLWindow->teminateGL();
}

void ReOrder::setLamda(float lamda)
{
	_lamda = lamda;
}

void ReOrder::depthSortPatch(vector<int> &piIndexBufferTmp, const Vector &viewpoint, const vector<Vector> &avgPatchPos)
{
	int i, j;
	patchSort *viewToPatch = new patchSort[_patchNums];
	memset(viewToPatch, 0, _patchNums*sizeof(patchSort));
	for (i = 0; i < _patchNums; i++)
	{
		viewToPatch[i].id = i;
		viewToPatch[i].dist = dist(viewpoint, avgPatchPos[i]);
	}
	std::sort(viewToPatch, viewToPatch + _patchNums, sortPatch);
	
	int jj = 0;
	for (i = 0; i < _patchNums; i++)
	{
		for (j = _patchesOut[viewToPatch[i].id] * 3; j < _patchesOut[viewToPatch[i].id + 1] * 3; j++)
			piIndexBufferTmp[jj++] = _linearIndices[j];
	}

	delete[] viewToPatch;
}

void ReOrder::initMeans()
{
	int numFaces3 = _model->getNumberofIndices();
	_means.resize(_numClusters, vector<int>(numFaces3, 0));

	vector<Vector> avgPatchesPos(_patchNums, Vector(0, 0, 0));

	for (auto i = 0; i < _numFrames; i++){
		Model* model = _modelManager->getModel(i);
		model->genPatchPos(_patchNums,_linearIndices,_patchesOut);
		const vector<Vector> &framePatchesPositions = model->getPatchesPos();
		for (auto j = 0; j < _patchNums; j++)
			avgPatchesPos[j] += framePatchesPositions[j];
	}
	for (auto j = 0; j < _patchNums; j++)
		avgPatchesPos[j] /= _numFrames;

	for (auto i = 0; i < _numClusters; i++){
		Vector viewpoint(_viewPos[_viewIds[i]]);
		depthSortPatch(_means[i], viewpoint, avgPatchesPos);
	}
}

float ReOrder::makeAssignments()
{
	float averageRatio = 0.0f;
	patchSort *tempRatio = new patchSort[_numClusters];
	memset(tempRatio, 0, _numClusters*sizeof(patchSort));
	for (auto x = 0; x < _numFrames; x++){
		for (auto z = 0; z < _numViews; z++){
			for (auto y = 0; y < _numClusters; y++){
				tempRatio[y].id = y;
				tempRatio[y].dist = _ratios[x][y][z];
			}
			std::sort(tempRatio, tempRatio + _numClusters, sortPatch);
			_minRatios[x][z] = tempRatio[0].dist;
			_assignments[x][z] = tempRatio[0].id;
			averageRatio += _minRatios[x][z];
		}
	}
	averageRatio /= (float)(_numFrames*_numViews);
	delete[] tempRatio;
	return averageRatio;
}

bool ReOrder::newClusterMean(int clusterId)
{
	int numFaces3 = _model->getNumberofIndices();
	vector<int> tempMean(numFaces3,0);

	clusterAssign *cluster = new clusterAssign[_numFrames*_numViews];
	memset(cluster, 0, _numFrames*_numViews*sizeof(clusterAssign));
	patchSort *viewToPatch = new patchSort[_patchNums];
	memset(viewToPatch, 0, _patchNums*sizeof(patchSort));
	int count = 0;
	for (auto i = 0; i < _numFrames; i++){
		for (auto j = 0; j < _numViews; j++){
			if (_assignments[i][j] == clusterId){
				cluster[count].frameId = i;
				cluster[count].viewId = j;
				count++;
			}
		}
	}

	int frameId, viewId;
	vector<int> frontPatchNums(_patchNums,0);
	for (auto i = 0; i < _patchNums; i++)
		viewToPatch[i].id = i;
	for (auto i = 0; i < count; i++){
		frameId = cluster[i].frameId;
		viewId = cluster[i].viewId;
		Model* model = _modelManager->getModel(frameId);
		const vector<Vector> patchPos = model->getPatchesPos();
		for (auto j = 0; j < _patchNums; j++){
			Vector viewpoint = _viewPos[viewId];
			if (! model->backPatch(viewpoint, _linearIndices, _patchesOut, j))
			{
				viewToPatch[j].dist += dist(_viewPos[viewId], patchPos[j]);
				frontPatchNums[j] ++;
			}
		}
	}
	for (auto j = 0; j < _patchNums; j++){
		viewToPatch[j].dist /= (float)frontPatchNums[j];
	}
	std::sort(viewToPatch, viewToPatch + _patchNums, sortPatch);
	int jj = 0;
	for (auto i = 0; i < _patchNums; i++){
		for (auto j = _patchesOut[viewToPatch[i].id] * 3; j < _patchesOut[viewToPatch[i].id + 1] * 3; j++)
			tempMean[jj++] = _linearIndices[j];
	}
	bool moveMean = compareClusterMean(tempMean, clusterId);

	delete[] cluster;
	delete[] viewToPatch;
	return moveMean;
}

bool ReOrder::compareClusterMean(const vector<int> &tempMean, int clusterId)
{
	bool clusterMove = false;
	GLuint64EXT oldDrawn = 0;
	_GLWindow->setZeroAtomicBuffer();
	oldDrawn = _GLWindow->readAtomicBuffer();
	for (int frameId = 0; frameId < _numFrames; frameId++)
	{
		_GLWindow->setClusterCamera(_assignments,frameId,clusterId);
		Model* model = _modelManager->getModel(frameId);
		_GLWindow->subLoadGeo(model->getVertices(), _means[clusterId]);

		_GLWindow->render(_numViews);
		_GLWindow->showGL();
	}
	oldDrawn = _GLWindow->readAtomicBuffer();
	printf("old drawn pixels: %u\n", oldDrawn);


	_GLWindow->setZeroAtomicBuffer();
	GLuint64EXT newDrawn = 0;
	for (int frameId = 0; frameId < _numFrames; frameId++)
	{
		Model* model = _modelManager->getModel(frameId);
		_GLWindow->subLoadGeo(model->getVertices(), tempMean);
		_GLWindow->setClusterCamera(_assignments, frameId, clusterId);

		_GLWindow->render(_numViews);
		_GLWindow->showGL();
	}
	newDrawn = _GLWindow->readAtomicBuffer();
	printf("new drawn pixels: %u\n", newDrawn);


	if (newDrawn < oldDrawn)
	{
		printf("replace old index buffer with new index buffer \n");
		_means[clusterId].assign(tempMean.begin(), tempMean.end());
		clusterMove = true;
	}
	else
	{
		printf("cluster index buffer remain the same \n");
	}
	return clusterMove;
}