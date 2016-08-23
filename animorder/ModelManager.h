#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "Model.h"
#include <string>
class ModelManager
{
private:
	vector<Model*> _models;
	int            _aniLens;
	string         _aniFolder;
	
public:
	ModelManager(int aniLen, string aniFolder);
	~ModelManager();
	// load model for animation
	void initModels();
	void addModel(Model * pModel);
	Model* getModel(int id);
};

#endif