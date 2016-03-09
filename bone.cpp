#include "bone.hpp"
#include <iostream>
#include <vector>


Bone::Bone() {
	parent = NULL;
	child = NULL;
	boneID = 0;
	isRoot = false;
	
	negOffset = glm::vec3(0, 0, 0);
	posOffset = glm::vec3(0, 0, 0);
	pivotPoint = glm::vec3(0, 0, 0);

	TranslationMatrix = glm::mat4(1.0);
	ScalingMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	modelMatrix = glm::mat4(1.0);
}

Bone::Bone(int ID, glm::vec3 T, glm::vec3 S, glm::vec3 O)
{
	parent = NULL;
	child = NULL;
	boneID = ID;
	isRoot = false;
	posOffset = T;
	negOffset = O;

	ScalingMatrix = scale(glm::mat4(1.0), S);
	TranslationMatrix = translate(glm::mat4(), T);
	modelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
}

void Bone::addParent(Bone *bone)
{
	this->parent = bone;
}

void Bone::addChild(Bone *bone)
{
	this->child = bone;
	children.push_back(bone);
}

bool Bone::hasParent()
{
	if (this->parent)
		return true;
	else return false;
}

bool Bone::hasChild()
{
	if (this->child)
		return true;
	else return false;
}

// if bone is updated via skeleton
void Bone::updateBone(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	TranslationMatrix = translate(TranslationMatrix, translation);
	RotationMatrix = glm::rotate(RotationMatrix, rotation, axis);
	modelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

	if (hasParent())
	{
		glm::mat4 tempTranslationMatrix = translate(TranslationMatrix, parent->negOffset);
		glm::mat4 tempModelMatrix = tempTranslationMatrix * RotationMatrix * ScalingMatrix;
		modelMatrix = parent->modelMatrix * tempModelMatrix;
	}

	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
}


// if parent is updated
void Bone::parentHasMoved(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	glm::mat4 tempTranslationMatrix = translate(TranslationMatrix, parent->negOffset);
	glm::mat4 tempModelMatrix = tempTranslationMatrix * RotationMatrix * ScalingMatrix;

	modelMatrix = parent->modelMatrix * tempModelMatrix;

	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
}

Bone::~Bone() {}