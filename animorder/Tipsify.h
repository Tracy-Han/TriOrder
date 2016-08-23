#ifndef TIPSIFY_H
#define TIPSIFY_H

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <functional>
#include <algorithm>
#include <memory>
#include <limits>
#include <cstddef>
#include "Vector.h"

/*******************************************************************************************************
This file has the code from "Fast Triangle Reordering for Vertex Locality and Reduced Overdraw"
as well as additional utility classes and alternative access point functions to facilitate its use.

This work is Copyright 2007 by AMD.
It provided for use by researchers and game developers.

The main function to take note of is shown below. It performs the entire optimization as described by the paper.

void FanVertOptimize(	float *pfVertexPositionsIn,   //vertex buffer positions, 3 floats per vertex
int *piIndexBufferIn,         //index buffer positions, 3 ints per vertex *in CCW order*
int *piIndexBufferOut,        //updated index buffer (the output of the algorithm)
int iNumVertices,             //# of vertices in the vertex buffer
int iNumFaces,                //# of faces in the index buffer
int iCacheSize,               //hardware cache size (usually 12 though 24 are good options)

float alpha,                  //constant parameter to compute lambda term from algorithm
float beta,                   //linear parameter to compute lambda term from algorithm
//lambda = alpha + beta * ACMR_OF_TIPSIFY
//refer to the paper for a description of these parameters
//(usually alpha=0.75f and beta=0.0f are good options)

int *piScratch = NULL,        //optional temp buffer for computations; its size in bytes should be:
//FanVertScratchSize(iNumVertices, iNumFaces)
//if NULL is passed, function will allocate and free this data

int *piClustersOut = NULL,    //optional buffer for the output cluster position (in faces) of each cluster
int *piNumClustersOut = NULL) //the number of putput clusters

If you don't care about overdraw and just want vertex cache optimization, you may use:

void FanVertOptimizeVCacheOnly(	int *piIndexBufferIn,
int *piIndexBufferOut,
int iNumVertices,
int iNumFaces,
int iCacheSize,
int *piScratch = NULL,
int *piClustersOut = NULL,
int *iNumClusters = NULL)

The function below just clusters the mesh. It assumes it is already sorted and pre-clustered
with "hard boundaries" during vertex cache optimization using the above function.
(please see paper for more details)

void FanVertOptimizeClusterOnly( int *piIndexBufferIn,
int iNumVertices,
int iNumFaces,
int iCacheSize,
float lambda,
int *piClustersIn,
int iNumClusters,
int *piClustersOut,
int *iNumClustersOut,
int *piScratch = NULL )

The function below just optimizes for overdraw and returns a "remap" array which maps the new cluster IDs to
the old ones. It is particularly useful for characters composed of multiple draw calls, as this will give an ordering
of draw calls to attempt to reduce overdraw.

void FanVertOptimizeOverdrawOnly(float *pfVertexPositionsIn,
int *piIndexBufferIn,
int *piIndexBufferOut,
int iNumVertices,
int iNumFaces,
int iCacheSize,
float lambda,
int *piClustersIn,
int iNumClusters,
int *piScratch = NULL,
int *piRemap = NULL)


*******************************************************************************************************/

void FanVertOptimize(float *pfVertexPositionsIn,   //vertex buffer positions, 3 floats per vertex
	int *piIndexBufferIn,         //index buffer positions, 3 ints per vertex *in CCW order*
	int *piIndexBufferOut,        //updated index buffer (the output of the algorithm)
	int iNumVertices,             //# of vertices in the vertex buffer
	int iNumFaces,                //# of faces in the index buffer
	int iCacheSize,               //hardware cache size (usually 12 though 24 are good options)

	float alpha,                  //constant parameter to compute lambda term from algorithm
	float beta,                   //linear parameter to compute lambda term from algorithm
	//lambda = alpha + beta * ACMR_OF_TIPSIFY
	//refer to the paper for a description of these parameters
	//(usually alpha=0.75f and beta=0.0f are good options)

	int *piScratch = NULL,        //optional temp buffer for computations; its size in bytes should be:
	//FanVertScratchSize(iNumVertices, iNumFaces)
	//if NULL is passed, function will allocate and free this data

	int *piClustersOut = NULL,    //optional buffer for the output cluster position (in faces) of each cluster
	int *piNumClustersOut = NULL);//the number of output clusters

void FanVertOptimizeVCacheOnly(int *piIndexBufferIn,
	int *piIndexBufferOut,
	int iNumVertices,
	int iNumFaces,
	int iCacheSize,
	int *piScratch = NULL,
	int *piClustersOut = NULL,
	int *iNumClusters = NULL);

void FanVertOptimizeClusterOnly(int *piIndexBufferIn,
	int iNumVertices,
	int iNumFaces,
	int iCacheSize,
	float lambda,
	int *piClustersIn,
	int iNumClusters,
	int *piClustersOut,
	int *iNumClustersOut,
	int *piScratch = NULL);

void FanVertOptimizeOverdrawOnly(float *pfVertexPositionsIn,
	int *piIndexBufferIn,
	int *piIndexBufferOut,
	int iNumVertices,
	int iNumFaces,
	int iCacheSize,
	float lambda,
	int *piClustersIn,
	int iNumClusters,
	int *piScratch = NULL,
	int *piRemap = NULL);



#endif