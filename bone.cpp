#include "bone.hpp"
#include <iostream>
#include <vector>


Bone::Bone() {
	parent = NULL;
	child = NULL;
	boneID = 0;
	modelMatrix = glm::mat4(1.0);
	isRoot = false;
	TranslationMatrix = glm::mat4(1.0);
	ScalingMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	negOffset = glm::vec3(0, 0, 0);
	posOffset = glm::vec3(0, 0, 0);
	pivotPoint = glm::vec3(0, 0, 0);
	endEffector = glm::vec3(0, 0, 0);
	localAngles = glm::vec3(0, 0, 0);
	globalAngles = glm::vec3(0, 0, 0);
}

Bone::Bone(int ID, glm::vec3 T, glm::vec3 S, glm::vec3 O)
{
	parent = NULL;
	child = NULL;
	boneID = ID;
	isRoot = false;
	endEffector = T + glm::vec3(0,2,0);
	localAngles = glm::vec3(0, 0, 0);
	globalAngles = glm::vec3(0, 0, 0);
	std::cout << "endEffector.x " << endEffector.x << "\tendEffector.y " << endEffector.y << std::endl << std::endl;
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

void Bone::updateBone(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	pivotPoint += translation;
	updateLocalAngles(rotation, axis);
	updateGlobalAngles(rotation, axis);
	//std::cout << "updateBone " << boneID << std::endl;
	//std::cout << "pivotPoint.x " << pivotPoint.x << "\tpivotPoint.y " << pivotPoint.y << std::endl;
	//std::cout << "endEffector.x " << endEffector.x << "\tendEffector.y " << endEffector.y << std::endl << std::endl;
	TranslationMatrix = translate(TranslationMatrix, translation);
	//TranslationMatrix = translate(glm::mat4(), pivotPoint);
	RotationMatrix = glm::rotate(RotationMatrix, rotation, axis);
	//RotationMatrix = glm::rotate(glm::mat4(), getlocalAngle(axis), axis);
	modelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	if (hasParent())
	{
		glm::mat4 tempTranslationMatrix = translate(TranslationMatrix, parent->negOffset);
		glm::mat4 tempModelMatrix = tempTranslationMatrix * RotationMatrix * ScalingMatrix;
		modelMatrix = parent->modelMatrix * tempModelMatrix;
	}

	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
	//updateEndEffector(translation, rotation, axis);
	//if (hasParent())
		//modelMatrix = parent->modelMatrix * modelMatrix;
}


// if notified that the parent has moved update model matrix
void Bone::parentHasMoved(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	//pivotPoint = parent->endEffector;
	updateGlobalAngles(rotation, axis);
	//std::cout << "Bone " << boneID << std::endl;
	//std::cout << "pivotPoint.x " << pivotPoint.x << "\tpivotPoint.y " << pivotPoint.y << std::endl;
	//std::cout << "endEffector.x " << endEffector.x << "\tendEffector.y " << endEffector.y << std::endl << std::endl;
	modelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	glm::mat4 tempTranslationMatrix = translate(TranslationMatrix, parent->negOffset);
	glm::mat4 tempModelMatrix = tempTranslationMatrix * RotationMatrix * ScalingMatrix;
	modelMatrix = parent->modelMatrix * tempModelMatrix;
	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
	//updateEndEffector(translation, rotation, axis);
}

void Bone::updateEndEffector(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	endEffector += translation;
	if (rotation != 0)
	{
		if (axis.x != 0)
		{
			endEffector.z = pivotPoint.z + 2 * sin(globalAngles.x);
			endEffector.y = pivotPoint.y + 2 * cos(globalAngles.x);
		}
		if (axis.y != 0)
		{
			endEffector.z = pivotPoint.z + 2 * cos(globalAngles.y);
			endEffector.x = pivotPoint.x + 2 * sin(globalAngles.y) * -1;
		}
		if (axis.z != 0)
		{
			endEffector.x = pivotPoint.x + 2 * sin(globalAngles.z) * -1;
			endEffector.y = pivotPoint.y + 2 * cos(globalAngles.z);
			//std::cout << "Bone " << boneID << std::endl;
			//std::cout << "pivotPoint.x " << pivotPoint.x << "\tpivotPoint.y " << pivotPoint.y << "\tpivotPoint.z " << pivotPoint.z << std::endl;
			//std::cout << "x " << 2 * sin(globalAngles.z) * -1 << std::endl;
			//std::cout << "y " << 2 * cos(globalAngles.z) << std::endl;
			//std::cout << "endEffector.x " << endEffector.x << "\tendEffector.y " << endEffector.y << std::endl << std::endl;
		}
	}
}

//updated when bone is specifically moved
void Bone::updateLocalAngles(float rotation, glm::vec3 axis)
{
	if (axis.x != 0)
		localAngles.x += (rotation * axis.x);
	if (axis.y != 0)
		localAngles.y += (rotation * axis.y);
	if (axis.z != 0)
		localAngles.z += (rotation * axis.z);
	//std::cout << "localAngles.z " << localAngles.z << std::endl;
}

//updated if bone is specifically moved and if parent has moved
void Bone::updateGlobalAngles(float rotation, glm::vec3 axis)
{
	if (axis.x != 0)
		globalAngles.x += (rotation * axis.x);
	if (axis.y != 0)
		globalAngles.y += (rotation * axis.y);
	if (axis.z != 0)
		globalAngles.z += (rotation * axis.z);
	//std::cout << "globalAngles.z " << globalAngles.z << std::endl;
}

/*
float Bone::getlocalAngle(glm::vec3 axis)
{
	if (axis.x == 1)
		return localAngles.x;
	else if (axis.y == 1)
		return localAngles.y;
	else return localAngles.z;
}*/

Bone::~Bone() {}