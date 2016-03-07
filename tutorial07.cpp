// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <Windows.h>
#include <cmath>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/bone.hpp>
#include <common/skeleton.hpp>

//#define NUMBONES 16
#define NUMBONES 4

glm::mat4 ViewMatrix, ModelMatrix, ProjectionMatrix, TranslationMatrix, RotationMatrix, ScalingMatrix, MVP, targetTranslationMatrix, targetModelMatrix;
GLuint MatrixID, Texture, TextureID, vertexbuffer, uvbuffer, VertexArrayID, programID;
std::vector<glm::vec3> vertices;
bool updated = true;

void drawBone(Bone bone, mat4 ProjectionMatrix, mat4 ViewMatrix);
void drawSkeleton(Skeleton skeleton, mat4 ProjectionMatrix, mat4 ViewMatrix);
void checkKeys(mat4 ProjectionMatrix, mat4 ViewMatrix, float deltaTime);
void CCD();
void drawTarget(mat4 ProjectionMatrix, mat4 ViewMatrix);

Bone bone[NUMBONES];
Skeleton skeleton = Skeleton(NUMBONES);

Bone lowerArm, upperArm, hand, tip;


//----- Camera Variables --------
glm::vec3 view_angles;
float ViewRotationAngle = 0.2f;
quat ViewOrientation;
glm::mat4 ViewRotationMatrix;
bool camMoved = true;

//------ Transformations -------
float rotAngle = 0.005f;
glm::vec3 nullVec(0, 0, 0);
glm::vec3 left(-0.01, 0, 0);
glm::vec3 right(0.01, 0, 0);
glm::vec3 up(0, 0.01, 0);
glm::vec3 down(0, -0.01, 0);
glm::vec3 back(0, 0, -0.01);
glm::vec3 forth(0, 0, 0.01);
glm::vec3 axisX(1, 0, 0);
glm::vec3 axisY(0, 1, 0);
glm::vec3 axisZ(0, 0, 1);

glm::vec3 targetPos(8.0f, 8.0f, 8.0f);
glm::vec3 targetScale(0.25f, 0.25f, 0.25f);

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 07 - Model Loading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);  
    glfwPollEvents();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture
	Texture = loadDDS("Grey.DDS");
	//Texture = loadDDS("SopCamel1.DDS");

	// Get a handle for our "myTextureSampler" uniform
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	//std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ("cubeBase.obj", vertices, uvs, normals);

	//bool tip = loadOBJ("cubeTip.obj", vertices, uvs, normals);

	// Load it into a VBO

	vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// For speed computation
	double lastTime = glfwGetTime();
	double lastFrameTime = lastTime;
	int nbFrames = 0;

	//------------------ Initialize Matrices ----------------
	//--------------- not using controls.cpp ---------------------------
	ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	//ViewMatrix = glm::lookAt(vec3(0,-2,-10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	//RotationMatrix = glm::mat4(1.0);
	//TranslationMatrix = translate(mat4(), vec3(0.0f, 5.0f,-30.0f));
	//ScalingMatrix = scale(mat4(), vec3(1.0f, 1.0f, 1.0f));
	//ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	//MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	targetTranslationMatrix = translate(mat4(), targetPos);
	targetModelMatrix = targetTranslationMatrix * mat4(1.0) * scale(mat4(), targetScale);

	//------------------ Create Bones --------------------------
	// lab 3 create bones
	/*
	float x = 0.0f;
	float increment = 2.0f;
	float initialY = 3.0f;
	float y = initialY;
	glm::vec3 fingerScale(0.5, 1.0, 1.0);
	glm::vec3 palmScale(2.0, 2.0, 1.0);
	//glm::vec3 fingerScale(1.0, 1.0, 1.0);
	//glm::vec3 palmScale(1.0, 1.0, 1.0);

	//palm//root
	bone[0] = Bone(0, vec3(0, 0, 0), palmScale);

	//thumb
	y = 0.0f;
	for (int i = 1; i < 4; i++)
	{
		x = -2.5f;
		bone[i] = Bone(i, vec3(x, y, 0), fingerScale);
		y += increment;
	}
	y = initialY;

	//index
	for (int i = 4; i < 7; i++)
	{
		x = -1.5f;
		bone[i] = Bone(i, vec3(x, y, 0), fingerScale);
		y += increment;
	}
	y = initialY;

	//middle
	for (int i = 7; i < 10; i++)
	{
		x = -0.5f;
		bone[i] = Bone(i, vec3(x, y, 0), fingerScale);
		y += increment;
	}
	y = initialY;

	//ring
	for (int i = 10; i < 13; i++)
	{
		x = 0.5f;
		bone[i] = Bone(i, vec3(x, y, 0), fingerScale);
		y += increment;
	}
	y = initialY;

	//baby
	for (int i = 13; i < 16; i++)
	{
		x = 1.5f;
		bone[i] = Bone(i, vec3(x, y, 0), fingerScale);
		y += increment;
	}

	/*
	for (int i = 0; i < NUMBONES; i++)
	{
		bone[i] = Bone(i, vec3(0, y, 0), false);
		y += 5.0f;
	}
	*/
	
	// lab 4 bones
	vec3 scale = vec3(1, 1, 1);
	upperArm = Bone(0, vec3(0, 0, 0), scale, vec3(0,0,0));
	lowerArm = Bone(1, vec3(0, 2, 0), scale, vec3(0, -2, 0));
	hand = Bone(2, vec3(0, 4, 0), scale, vec3(0, -4, 0));
	tip = Bone(3, vec3(0, 6, 0), vec3(0.25, 0.25, 0.25), vec3(0, -6, 0));

	//----------------- Make relationships ----------------
	// lab 3 bone relationships
	/*
	//palm//root
	bone[0].isRoot = true;
	for (int i = 1; i < NUMBONES; i++)
		bone[0].addChild(&bone[i]);

	for (int i = 1; i < (NUMBONES - 2); i+=3)
		bone[i].addParent(&bone[0]);

	//thumb
	for (int i = 3; i > 1; i--)
	{
		bone[1].addChild(&bone[i]);
		bone[i].addParent(&bone[i - 1]);
	}
	bone[2].addChild(&bone[3]);

	//index
	for (int i = 6; i > 4; i--)
	{
		bone[4].addChild(&bone[i]);
		bone[i].addParent(&bone[i - 1]);
	}
	bone[5].addChild(&bone[6]);

	//middle
	for (int i = 9; i > 7; i--)
	{
		bone[7].addChild(&bone[i]);
		bone[i].addParent(&bone[i - 1]);
	}
	bone[8].addChild(&bone[9]);

	//ring
	for (int i = 12; i > 10; i--)
	{
		bone[10].addChild(&bone[i]);
		bone[i].addParent(&bone[i - 1]);
	}
	bone[11].addChild(&bone[12]);

	//baby
	for (int i = 15; i > 13; i--)
	{
		bone[13].addChild(&bone[i]);
		bone[i].addParent(&bone[i - 1]);
	}
	bone[14].addChild(&bone[15]);

	/*
	// Initialize with respect to parent
	for (int i = 0; i < NUMBONES; i++)
	{
		bone[i].updateBone(vec3(0, 0, 0), 0);
	}
	*/

	// lab 4 relationships
	//upperArm.isRoot = true;
	upperArm.addChild(&tip);
	upperArm.addChild(&hand);
	upperArm.addChild(&lowerArm);

	lowerArm.addParent(&upperArm);
	lowerArm.addChild(&tip);
	lowerArm.addChild(&hand);

	hand.addParent(&lowerArm);
	hand.addChild(&tip);

	tip.addParent(&hand);

	//------------------ Load Skeleton ------------------------
	// lab 3 skeleton
	/*
	//skeleton = Skeleton(NUMBONES);
	for (int i = 0; i < NUMBONES; i++)
	{
		skeleton.loadBone(&bone[i]);
	}
	*/

	// lab 4 skeleton
	skeleton.loadBone(&upperArm);
	skeleton.loadBone(&lowerArm);
	skeleton.loadBone(&hand);
	skeleton.loadBone(&tip);

	// ---------------- loop -------------------
	do{

		// Measure speed
		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			// printf and reset
			//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		
		//--------------- to use controls.cpp from plane lab -----------------------
		/*
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelMatrix = getModelMatrix();
		*/
		//TranslationMatrix = translate(mat4(), vec3(0.0f,10.0f,-30.0f));
		//ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
		//MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//------------- Update Camera --------------------
		//camMoved = false; // this line disables camera
		if (camMoved)
		{
			//ViewMatrix = glm::lookAt(vec3(5,-10, 45), glm::vec3(0, 20, 0), glm::vec3(0, 1, 0)); // lab 3 initial view matrix
			ViewMatrix = glm::lookAt(vec3(10, 5, 30), glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)); // lab 4 initial view matrix
			glm::quat view_rotation(radians(view_angles));
			ViewOrientation = ViewOrientation * view_rotation;
			ViewRotationMatrix = toMat4(ViewOrientation);
			ViewMatrix = ViewMatrix * ViewRotationMatrix; // switch multiplication to switch to first person
			camMoved = false;
		}

		//update target
		targetTranslationMatrix = translate(mat4(1.0), targetPos);

		//------------- Let's Draw! -----------------------
		drawSkeleton(skeleton, ProjectionMatrix, ViewMatrix);
		drawTarget(ProjectionMatrix, ViewMatrix);


		checkKeys(ProjectionMatrix, ViewMatrix, deltaTime);

		//if (updated)
		if (targetPos != tip.pivotPoint)
			CCD();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


void CCD()
{
	vec3 targetVector, endEffector, endEffectorVector, rotAxis;
	float angle;
	endEffector = tip.pivotPoint;
		
	// round 1
	if (targetPos != endEffector)
	{
		targetVector = vec3(vec3(targetPos.x, targetPos.y, targetPos.z) - vec3(hand.pivotPoint.x, hand.pivotPoint.y, hand.pivotPoint.z)); // direction B
		endEffectorVector = vec3(vec3(endEffector.x, endEffector.y, endEffector.z) - vec3(hand.pivotPoint.x, hand.pivotPoint.y, hand.pivotPoint.z)); // direction A
		angle = acos(dot(normalize(targetVector), normalize(endEffectorVector))); // angle between
		//std::cout << "angle " << angle << std::endl;
		rotAxis = vec3(cross(normalize(targetVector), normalize(endEffectorVector))); // rotation axis
		//std::cout << "rotAxis " << rotAxis.x << " " << rotAxis.y << " " << rotAxis.z << std::endl;
		if (angle > 0)
			skeleton.update(&hand, nullVec, -angle, rotAxis);
	}

	// round 2 // should probably iterate through these in main do loop
	endEffector = tip.pivotPoint;
	if (targetPos != endEffector)
	{
		targetVector = vec3(vec3(targetPos.x, targetPos.y, targetPos.z) - vec3(lowerArm.pivotPoint.x, lowerArm.pivotPoint.y, lowerArm.pivotPoint.z));
		endEffectorVector = vec3(vec3(endEffector.x, endEffector.y, endEffector.z) - vec3(lowerArm.pivotPoint.x, lowerArm.pivotPoint.y, lowerArm.pivotPoint.z));
		angle = acos(dot(normalize(targetVector), normalize(endEffectorVector)));
		rotAxis = vec3(cross(normalize(targetVector), normalize(endEffectorVector)));
		if (angle > 0)
			skeleton.update(&lowerArm, nullVec, -angle, rotAxis);
	}

	endEffector = tip.pivotPoint;
	if (targetPos != endEffector)
	{
		targetVector = vec3(vec3(targetPos.x, targetPos.y, targetPos.z) - vec3(upperArm.pivotPoint.x, upperArm.pivotPoint.y, upperArm.pivotPoint.z));
		endEffectorVector = vec3(vec3(endEffector.x, endEffector.y, endEffector.z) - vec3(upperArm.pivotPoint.x, upperArm.pivotPoint.y, upperArm.pivotPoint.z));
		angle = acos(dot(normalize(targetVector), normalize(endEffectorVector)));
		rotAxis = vec3(cross(normalize(targetVector), normalize(endEffectorVector)));
		if (angle > 0)
			skeleton.update(&upperArm, nullVec, -angle, rotAxis);
	}
	

	updated = false;
	// x axis 
	/*
	targetVector = vec3(vec3(0, targetPos.y, targetPos.z) - vec3(0, hand.pivotPoint.y, hand.pivotPoint.z));
	endEffectorVector = vec3(vec3(0, endEffector.y, endEffector.z) - vec3(0, hand.pivotPoint.y, hand.pivotPoint.z));
	angle = acos(dot(normalize(targetVector), normalize(endEffectorVector)));
	if (angle != 0 && angle > 0)
	{
		std::cout << "angle X " << angle << std::endl;
		updated = false;
		skeleton.update(&hand, nullVec, angle, axisX);
	}
	
	
	// y axis
	
	targetVector = vec3(vec3(targetPos.x, 0, targetPos.z) - vec3(hand.pivotPoint.x, 0, hand.pivotPoint.z));
	endEffectorVector = vec3(vec3(endEffector.x, 0, endEffector.z) - vec3(hand.pivotPoint.x, 0, hand.pivotPoint.z));
	angle = acos(dot(normalize(targetVector), normalize(endEffectorVector)));
	if (angle != 0 && angle > 0)
	{
		std::cout << "angle Y " << angle << std::endl;
		updated = false;
		skeleton.update(&hand, nullVec, angle, axisY);
	}
	
	
	// z axis
	
	targetVector = vec3(vec3(targetPos.x, targetPos.y, 0) - vec3(hand.pivotPoint.x, hand.pivotPoint.y, 0));
	endEffectorVector = vec3(vec3(endEffector.x, endEffector.y, 0) - vec3(hand.pivotPoint.x, hand.pivotPoint.y, 0));
	angle = acos(dot(normalize(targetVector), normalize(endEffectorVector)));
	if (angle != 0 && angle > 0)
	{
		std::cout << "angle Z " << angle << std::endl;
		updated = false;
		skeleton.update(&hand, nullVec, angle, axisZ);
	}
	*/
	
}

void drawSkeleton(Skeleton skeleton, mat4 ProjectionMatrix, mat4 ViewMatrix)
{
	for (int i = 0; i < skeleton.numBones; i++)
	{
		ModelMatrix = skeleton.myBone[i]->modelMatrix;
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}
}

void drawTarget(mat4 ProjectionMatrix, mat4 ViewMatrix)
{
	targetModelMatrix = targetTranslationMatrix * mat4(1.0) * scale(mat4(), targetScale);
	MVP = ProjectionMatrix * ViewMatrix * targetModelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}


void checkKeys(mat4 ProjectionMatrix, mat4 ViewMatrix, float deltaTime)
{
	// Lab 3 Hand controls
	/*
	// manually wave
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		skeleton.update(&bone[0], vec3(0, 0, 0), rotAngle, axisZ);
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		skeleton.update(&bone[0], vec3(0, 0, 0), -rotAngle, axisZ);
	}

	// auto wave 
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		float time = glfwGetTime();
		skeleton.update(&bone[0], vec3(0, 0, 0), sin(time * 15) / 100, axisZ);
		//playAnimation(ProjectionMatrix, ViewMatrix, deltaTime);
	}

	//------------ palm / root control --------------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		//bone[1].updateBone(vec3(0, 0, 0), rotAngle);
		skeleton.update(&bone[0], back, 0, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		//bone[0].updateBone(vec3(0, 0, 0), rotAngle);
		skeleton.update(&bone[0], forth, 0, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		//bone[0].updateBone(left, 0);
		skeleton.update(&bone[0], left, 0, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		//bone[0].updateBone(right, 0);
		skeleton.update(&bone[0], right, 0, axisX);
	}

	//-------------------- finger controls ---------------------
	//thumb root
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		skeleton.update(&bone[1], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		skeleton.update(&bone[1], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//index root
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		skeleton.update(&bone[4], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		skeleton.update(&bone[4], vec3(0, 0, 0), -rotAngle, axisX);
	} 

	//middle root
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		skeleton.update(&bone[7], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		skeleton.update(&bone[7], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//ring root
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		skeleton.update(&bone[10], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		skeleton.update(&bone[10], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//baby root
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		skeleton.update(&bone[13], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		skeleton.update(&bone[13], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//----------- finger centre bones ------------------------
	//thumb centre
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		skeleton.update(&bone[2], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		skeleton.update(&bone[2], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//index centre
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		skeleton.update(&bone[5], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		skeleton.update(&bone[5], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//middle centre
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		skeleton.update(&bone[8], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		skeleton.update(&bone[8], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//ring centre
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		skeleton.update(&bone[11], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		skeleton.update(&bone[11], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//baby centre
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		skeleton.update(&bone[14], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		skeleton.update(&bone[14], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//----------- finger tip bones ------------------------
	//thumb tip
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		skeleton.update(&bone[3], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		skeleton.update(&bone[3], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//index tip
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		skeleton.update(&bone[6], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		skeleton.update(&bone[6], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//middle tip
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		skeleton.update(&bone[9], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		skeleton.update(&bone[9], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//ring tip
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		skeleton.update(&bone[12], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		skeleton.update(&bone[12], vec3(0, 0, 0), -rotAngle, axisX);
	}

	//baby tip
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		skeleton.update(&bone[15], vec3(0, 0, 0), rotAngle, axisX);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		skeleton.update(&bone[15], vec3(0, 0, 0), -rotAngle, axisX);
	}
	*/

	// lab 4 controls
/*
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		skeleton.update(&upperArm, left, 0, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		skeleton.update(&upperArm, right, 0, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		skeleton.update(&lowerArm, left, 0, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		skeleton.update(&lowerArm, right, 0, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		skeleton.update(&upperArm, nullVec, rotAngle, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		skeleton.update(&upperArm, nullVec, -rotAngle, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		skeleton.update(&lowerArm, nullVec, rotAngle, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		skeleton.update(&lowerArm, nullVec, -rotAngle, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		skeleton.update(&hand, nullVec, rotAngle, axisZ);
		updated = true;
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		skeleton.update(&hand, nullVec, -rotAngle, axisZ);
		updated = true;
	}
	*/

	// target controls
	if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)){
		targetPos += right;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)){
		targetPos += left;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)){
		targetPos += up;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)){
		targetPos += down;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)){
		targetPos += back;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)){
		targetPos += forth;
		updated = true;
	}

	//---------- Camera Controls -----------------------
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		view_angles = vec3(0, ViewRotationAngle, 0);
		camMoved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		view_angles = vec3(0, -ViewRotationAngle, 0);
		camMoved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		view_angles = vec3(-ViewRotationAngle, 0, 0);
		camMoved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		view_angles = vec3(ViewRotationAngle, 0, 0);
		camMoved = true;
	}
}
