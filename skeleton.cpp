#include "skeleton.hpp"
#include <iostream>

Skeleton::Skeleton()
{
	numBones = 0;
	//root = Bone();
}

Skeleton::Skeleton(int nBones)
{
	//root = Bone();
	numBones = nBones;
	std::cout << "Skeleton with " << numBones << " bones made.\n";
}

void Skeleton::loadBone(Bone *bone)
{
	int boneID = bone->boneID;
	myBone[boneID] = bone;
	if (bone->isRoot)
		root = bone;
}

void Skeleton::update(Bone *bone, glm::vec3 translation, float rotation, glm::vec3 axis)
{
	int ID = bone->boneID;

	if (myBone[ID]->isRoot)
		updateRoot(translation, rotation, axis);
	
	myBone[ID]->updateBone(translation, rotation, axis);

	// Recursively update children
	if (myBone[ID]->hasChild())
	{
		//update(myBone[ID]->child, translation, rotation, axis);
		tellChild(myBone[ID]->child, translation, rotation, axis);
	}
}

void Skeleton::updateRoot(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	root->updateBone(translation, rotation, axis);
	//update(myBone[1], translation, rotation, axis);
	tellChild(myBone[1], translation, rotation, axis);

	return;
}

// inform children that their parent has moved
void Skeleton::tellChild(Bone *bone, glm::vec3 translation, float rotation, glm::vec3 axis)
{
	int ID = bone->boneID;
	myBone[ID]->parentHasMoved(translation, rotation, axis);
	if (myBone[ID]->hasChild())
		tellChild(myBone[ID]->child, translation, rotation, axis);
}

Skeleton::~Skeleton(){}
