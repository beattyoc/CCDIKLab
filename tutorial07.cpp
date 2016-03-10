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

#define NUMBONES 7

glm::mat4 ViewMatrix, ModelMatrix, ProjectionMatrix, TranslationMatrix, RotationMatrix, ScalingMatrix, MVP, targetTranslationMatrix, targetModelMatrix;
GLuint MatrixID, Texture, TextureID, vertexbuffer, uvbuffer, VertexArrayID, programID;
std::vector<glm::vec3> vertices;
bool updated = true;

void drawBone(Bone bone, mat4 ProjectionMatrix, mat4 ViewMatrix);
void drawSkeleton(Skeleton skeleton, mat4 ProjectionMatrix, mat4 ViewMatrix);
void checkKeys(mat4 ProjectionMatrix, mat4 ViewMatrix, float deltaTime);
void CCD();
void drawTarget(mat4 ProjectionMatrix, mat4 ViewMatrix);
vec3 bezierCurve(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3);

Bone bone[NUMBONES];
Skeleton skeleton = Skeleton(NUMBONES);

//----- Camera Variables --------
glm::vec3 view_angles;
float ViewRotationAngle = 0.2f;
quat ViewOrientation;
glm::mat4 ViewRotationMatrix;
bool camMoved = true;

//------ Transformations -------
float rotAngle = 0.005f;
glm::vec3 nullVec(0, 0, 0);
glm::vec3 left(-0.05, 0, 0);
glm::vec3 right(0.05, 0, 0);
glm::vec3 up(0, 0.05, 0);
glm::vec3 down(0, -0.05, 0);
glm::vec3 back(0, 0, -0.05);
glm::vec3 forth(0, 0, 0.05);
glm::vec3 axisX(1, 0, 0);
glm::vec3 axisY(0, 1, 0);
glm::vec3 axisZ(0, 0, 1);

// --------- Target info ---------
glm::vec3 targetPos(6.0f, 10.0f, 8.0f);
glm::vec3 targetScale(0.1f, 0.1f, 0.1f);

//------- Bezier Curve Points ------
vec3 p0(10, 10, 0);
vec3 p1(0, 7, -10);
vec3 p2(-12, 6, -5);
vec3 p3(-8, 8, 5);
bool followCurve = false;

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
	Texture = loadDDS("skin.DDS");

	// Get a handle for our "myTextureSampler" uniform
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ("sphere.obj", vertices, uvs, normals);

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
	ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	targetTranslationMatrix = translate(mat4(), targetPos);
	targetModelMatrix = targetTranslationMatrix * mat4(1.0) * scale(mat4(), targetScale);




	//------------------ Create Bones --------------------------
	vec3 scale = vec3(1, 1, 1);
	
	
	int Y = 0;
	for (int i = 0; i < 6; i++)
	{
		bone[i] = Bone(i, vec3(0, Y, 0), scale, vec3(0, -Y, 0));
		Y += 2;
	}
	bone[NUMBONES - 1] = Bone(6, vec3(0, (NUMBONES - 1) * 2, 0), vec3(0.1, 0.1, 0.1), vec3(0, (NUMBONES - 1) * -2, 0));
	


	//----------------- Make relationships ----------------
	bone[0].isRoot = true;
	bone[0].addChild(&bone[1]);
	for (int i = 1; i < NUMBONES - 1; i++)
	{
		bone[i].addParent(&bone[i - 1]);
		bone[i].addChild(&bone[i + 1]);
	}
	bone[NUMBONES - 1].addParent(&bone[NUMBONES - 2]);
	



	//------------------ Load Skeleton ------------------------
	for (int i = 0; i < NUMBONES; i++)
		skeleton.loadBone(&bone[i]);



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
			ViewMatrix = glm::lookAt(vec3(10, 5, 30), glm::vec3(0, 8, 0), glm::vec3(0, 1, 0)); // lab 4 initial view matrix
			glm::quat view_rotation(radians(view_angles));
			ViewOrientation = ViewOrientation * view_rotation;
			ViewRotationMatrix = toMat4(ViewOrientation);
			ViewMatrix = ViewMatrix * ViewRotationMatrix; // switch multiplication to switch to first person
			camMoved = false;
		}



		//update target
		targetTranslationMatrix = translate(mat4(1.0), targetPos);



		//------------- Draw -----------------------
		drawSkeleton(skeleton, ProjectionMatrix, ViewMatrix);
		drawTarget(ProjectionMatrix, ViewMatrix);



		// check for user input
		checkKeys(ProjectionMatrix, ViewMatrix, deltaTime);


		// ------ follow curve ----------------
		float t = glm::mod((float)currentTime, 5.0f) / (5.0f);

		if (followCurve)
			targetPos = bezierCurve(t, p0, p1, p2, p3);
	
	
		// if end effector is not at target, calculate CCD
		if (targetPos != bone[NUMBONES - 1].pivotPoint)
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


// ----- Cyclic Coordinate Descent ---------------
// Updates all bones in skeleton according to CCD algorithm
void CCD()
{
	vec3 targetVector, endEffector, endEffectorVector, rotAxis;
	float angleBetween;
	endEffector = bone[NUMBONES - 1].pivotPoint;
	for (int i = NUMBONES - 2; i > -1; i--)
	{
		if (targetPos != endEffector) // if end effector has reached target stop
		{
			targetVector = vec3(vec3(targetPos.x, targetPos.y, targetPos.z) - vec3(bone[i].pivotPoint.x, bone[i].pivotPoint.y, bone[i].pivotPoint.z));
			endEffectorVector = vec3(vec3(endEffector.x, endEffector.y, endEffector.z) - vec3(bone[i].pivotPoint.x, bone[i].pivotPoint.y, bone[i].pivotPoint.z));
			angleBetween = acos(dot(normalize(targetVector), normalize(endEffectorVector))); // angle between the EEvector and Tvector
			rotAxis = vec3(cross(normalize(targetVector), normalize(endEffectorVector))); // the normal of the plane that EEvector and Tvector sit on

			if (angleBetween >= 0 || angleBetween < 0) // to counteract if the angle is NAN 
				skeleton.update(&bone[i], nullVec, angleBetween, rotAxis);

			else std::cout << angleBetween << std::endl;
		}
		endEffector = bone[NUMBONES - 1].pivotPoint;
	}
}

// returns a position on the curve relative to t
vec3 bezierCurve(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	return (pow((1 - t), 3) * p0) + (3 * t*pow((1 - t), 2) * p1) + (3 * t*pow((1 - t), 2) * p2) + (pow(t, 3) * p3);
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
	// ---------- Curve controls ---------------------------
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		followCurve = true;

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		followCurve = false;

	// ------------ target controls --------------------------
	if ((glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)){
		targetPos += right;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)){
		targetPos += left;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)){
		targetPos += up;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)){
		targetPos += down;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)){
		targetPos += back;
		updated = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)){
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
