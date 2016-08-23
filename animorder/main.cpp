#include <iostream>

#include "ViewPoints.h"
#include "ReOrder.h"
void main()
{
	// random pick nCluster viewpoints
	vector<int> viewIds = { 148, 54, 17, 92, 45 };

	// reOrder(lamda, cacheSize, numClusters, viewIds)
	ReOrder* reorder = new ReOrder(0.85,20,5,viewIds);
	// initModels(animationLength, animation folder)
	reorder->initModels(75,"../PLY/test3");
	reorder->vertexCacheOpti();
	reorder->ovrOpti();
	getchar();

}

