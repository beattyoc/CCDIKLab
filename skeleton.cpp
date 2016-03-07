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
	
	//else
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
	std::cout << "update root\n";
	root->updateBone(translation, rotation, axis);

	update(myBone[1], translation, rotation, axis);

	// lab 3 update below
	// tell each finger root
	/*
	update(myBone[1], translation, rotation, axis);
	update(myBone[4], translation, rotation, axis);
	update(myBone[7], translation, rotation, axis);
	update(myBone[10], translation, rotation, axis);
	update(myBone[13], translation, rotation, axis);
	*/
	/*
	tellChild(myBone[1]);
	tellChild(myBone[4]);
	tellChild(myBone[7]);
	tellChild(myBone[10]);
	tellChild(myBone[13]);
	*/

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
