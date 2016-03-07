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
	rotAngles = glm::vec3(0, 0, 0);
	globalAngles = glm::vec3(0, 0, 0);
}

Bone::Bone(int ID, glm::vec3 T, glm::vec3 S, glm::vec3 O)
{
	parent = NULL;
	child = NULL;
	boneID = ID;
	isRoot = false;
	//pivotPoint = T;
	endEffector = T + glm::vec3(0,2,0);
	rotAngles = glm::vec3(0, 0, 0);
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
	updateRotAngles(rotation, axis);
	updateGlobalAngles(rotation, axis);
	//std::cout << "updateBone " << boneID << std::endl;
	//std::cout << "pivotPoint.x " << pivotPoint.x << "\tpivotPoint.y " << pivotPoint.y << std::endl;
	//std::cout << "endEffector.x " << endEffector.x << "\tendEffector.y " << endEffector.y << std::endl << std::endl;
	TranslationMatrix = translate(TranslationMatrix, translation);
	//TranslationMatrix = translate(glm::mat4(), pivotPoint);
	RotationMatrix = glm::rotate(RotationMatrix, rotation, axis);
	//RotationMatrix = glm::rotate(glm::mat4(), getRotAngle(axis), axis);
	modelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	if (hasParent())
	{
		glm::mat4 tempTranslationMatrix = translate(TranslationMatrix, parent->negOffset);
		glm::mat4 tempModelMatrix = tempTranslationMatrix * RotationMatrix * ScalingMatrix;
		modelMatrix = parent->modelMatrix * tempModelMatrix;
	}

	pivotPoint = glm::vec3(glm::column(modelMatrix, 3).x, glm::column(modelMatrix, 3).y, glm::column(modelMatrix, 3).z);
	updateEndEffector(translation, rotation, axis);
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
	updateEndEffector(translation, rotation, axis);
}

void Bone::updateEndEffector(glm::vec3 translation, float rotation, glm::vec3 axis)
{
	endEffector += translation;
	if (rotation != 0)
	{
		if (axis.x == 1)
		{

		}
		else if (axis.y == 1)
		{

		}
		else
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
void Bone::updateRotAngles(float rotation, glm::vec3 axis)
{
	if (axis.x == 1)
		rotAngles.x += rotation;
	else if (axis.y == 1)
		rotAngles.y += rotation;
	else rotAngles.z += rotation;
	//std::cout << "rotAngles.z " << rotAngles.z << std::endl;
}

//updated if bone is specifically moved and if parent has moved
void Bone::updateGlobalAngles(float rotation, glm::vec3 axis)
{
	if (axis.x == 1)
		globalAngles.x += rotation;
	else if (axis.y == 1)
		globalAngles.y += rotation;
	else globalAngles.z += rotation;
	//std::cout << "globalAngles.z " << globalAngles.z << std::endl;
}

float Bone::getRotAngle(glm::vec3 axis)
{
	if (axis.x == 1)
		return rotAngles.x;
	else if (axis.y == 1)
		return rotAngles.y;
	else return rotAngles.z;
}

Bone::~Bone() {}