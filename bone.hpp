#ifndef BONE_HPP
#define BONE_HPP

#include <GL/glew.h>
#include <glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

class Bone
{
public:
	Bone *parent;
	Bone *child;
	std::vector<Bone*> children;
	int boneID;
	glm::mat4 modelMatrix;
	glm::mat4 TranslationMatrix;
	glm::vec3 negOffset;
	glm::vec3 posOffset;
	glm::vec3 pivotPoint;
	glm::vec3 endEffector;
	glm::vec3 localAngles;
	glm::vec3 globalAngles;
	glm::mat4 RotationMatrix;
	glm::mat4 ScalingMatrix;
	bool isRoot;

	Bone();
	Bone(int ID, glm::vec3 T, glm::vec3 S, glm::vec3 O);
	~Bone();

	void addParent(Bone *bone);
	void addChild(Bone *bone);
	bool hasParent();
	bool hasChild();
	void updateBone(glm::vec3 translation, float rotation, glm::vec3 axis);
	void parentHasMoved(glm::vec3 translation, float rotation, glm::vec3 axis);
	void updateEndEffector(glm::vec3 translation, float rotation, glm::vec3 axis);
	void updateLocalAngles(float rotation, glm::vec3 axis);
	void updateGlobalAngles(float rotation, glm::vec3 axis);
	//float getlocalAngle(glm::vec3 axis);

};

#endif