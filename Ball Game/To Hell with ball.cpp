#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "command.h"
#include "screenshot_bmp.h"

using namespace glm;
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
static double Time = glfwGetTime();
#define G 9.8/4
float Angle = 0;
float Height = 0;
double T = 0.1;
double t = 0.01;
float y = 0;
float maximumHeight = 0.9f;
float direction = -1;
float radius = 0.02f;

int Ball_status = 0;
void DisplayEnd();
GLuint count;
struct step {

	GLuint go_num, stop_num, end_num;
	float Y;
	float go[7];	//1~2 3~4 5~6
	float stop[7];	//1~2 3~4 5~6
	float end[3];	// 1~2
};
struct step S[17];

bool InitSteps();

int main(void){
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Bumping ball", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
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
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture[4];
	Texture[0] = loadDDS("uvmap.DDS");	// Tower texture
	Texture[1] = loadDDS("ball.DDS");	// Ball texture
	Texture[2] = loadDDS("Crack_normal.DDS");	// Normal steps texture
	Texture[3] = loadDDS("Crack_end.DDS");	// Special steps texture


	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our tower.obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res_tower = loadOBJ("tower.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Read our ball.obj,  steps_normal.obj, steps_end.obj file
	std::vector<glm::vec3> vertices2, vertices3, vertices4, vertices5;
	std::vector<glm::vec2> uvs2, uvs3, uvs4, uvs5;
	std::vector<glm::vec3> normals2, normals3, normals4, normals5;
	bool res_ball = loadOBJ("ball.obj", vertices2, uvs2, normals2);
	bool res_septs_normal = loadOBJ("steps_normal.obj", vertices3, uvs3, normals3);
	bool res_end = loadOBJ("steps_end.obj", vertices4, uvs4, normals4);
	bool res_over = loadOBJ("gameover.obj", vertices5, uvs5, normals5);

	std::vector<unsigned short> indices2, indices3, indices4, indices5;
	std::vector<glm::vec3> indexed_vertices2, indexed_vertices3, indexed_vertices4, indexed_vertices5;
	std::vector<glm::vec2> indexed_uvs2, indexed_uvs3, indexed_uvs4, indexed_uvs5;
	std::vector<glm::vec3> indexed_normals2, indexed_normals3, indexed_normals4, indexed_normals5;


	indexVBO(vertices2, uvs2, normals2, indices2, indexed_vertices2, indexed_uvs2, indexed_normals2);
	indexVBO(vertices3, uvs3, normals3, indices3, indexed_vertices3, indexed_uvs3, indexed_normals3);
	indexVBO(vertices4, uvs4, normals4, indices4, indexed_vertices4, indexed_uvs4, indexed_normals4);
	indexVBO(vertices5, uvs5, normals5, indices5, indexed_vertices5, indexed_uvs5, indexed_normals5);


	// Load Tower into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Load ball into a VBO
	GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices2.size() * sizeof(glm::vec3), &indexed_vertices2[0], GL_STATIC_DRAW);

	GLuint uvbuffer2;
	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs2.size() * sizeof(glm::vec2), &indexed_uvs2[0], GL_STATIC_DRAW);

	GLuint normalbuffer2;
	glGenBuffers(1, &normalbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer2);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals2.size() * sizeof(glm::vec3), &indexed_normals2[0], GL_STATIC_DRAW);


	// Load steps(normal) into a VBO
	GLuint vertexbuffer3;
	glGenBuffers(1, &vertexbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices3.size() * sizeof(glm::vec3), &indexed_vertices3[0], GL_STATIC_DRAW);

	GLuint uvbuffer3;
	glGenBuffers(1, &uvbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs3.size() * sizeof(glm::vec2), &indexed_uvs3[0], GL_STATIC_DRAW);

	GLuint normalbuffer3;
	glGenBuffers(1, &normalbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer3);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals3.size() * sizeof(glm::vec3), &indexed_normals3[0], GL_STATIC_DRAW);


	// Load steps(end) into a VBO 
	GLuint vertexbuffer4;
	glGenBuffers(1, &vertexbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices4.size() * sizeof(glm::vec3), &indexed_vertices4[0], GL_STATIC_DRAW);

	GLuint uvbuffer4;
	glGenBuffers(1, &uvbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs4.size() * sizeof(glm::vec2), &indexed_uvs4[0], GL_STATIC_DRAW);

	GLuint normalbuffer4;
	glGenBuffers(1, &normalbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer4);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals4.size() * sizeof(glm::vec3), &indexed_normals4[0], GL_STATIC_DRAW);


	// Load GAMEONER into a VBO 
	GLuint vertexbuffer5;
	glGenBuffers(1, &vertexbuffer5);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer5);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices5.size() * sizeof(glm::vec3), &indexed_vertices5[0], GL_STATIC_DRAW);

	GLuint uvbuffer5;
	glGenBuffers(1, &uvbuffer5);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer5);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs5.size() * sizeof(glm::vec2), &indexed_uvs5[0], GL_STATIC_DRAW);

	GLuint normalbuffer5;
	glGenBuffers(1, &normalbuffer5);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer5);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals5.size() * sizeof(glm::vec3), &indexed_normals5[0], GL_STATIC_DRAW);





	// Generate a buffer for the indices of Tower
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices of Ball
	GLuint elementbuffer2;
	glGenBuffers(1, &elementbuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2.size() * sizeof(unsigned short), &indices2[0], GL_STATIC_DRAW);


	// Generate a buffer for the indices of Normal Steps
	GLuint elementbuffer3;
	glGenBuffers(1, &elementbuffer3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices3.size() * sizeof(unsigned short), &indices3[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices of Special Steps
	GLuint elementbuffer4;
	glGenBuffers(1, &elementbuffer4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices4.size() * sizeof(unsigned short), &indices4[0], GL_STATIC_DRAW);


	// Generate a buffer for the indices of Special Steps
	GLuint elementbuffer5;
	glGenBuffers(1, &elementbuffer5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer5);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices5.size() * sizeof(unsigned short), &indices5[0], GL_STATIC_DRAW);


	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	int current_step = 1;

	InitSteps();
	do {

		// Measure speed
		double currentTime = glfwGetTime();
		double oldTime = currentTime;
		nbFrames++;
		//if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}

		double deltatime = currentTime - Time;
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();


		////// Start of the rendering of the first object //////

		// 调用着色器
		glUseProgram(programID);
		// set lights
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		// Calculate Tower's ModelMatrix and Draw it
		float deltaTime = currentTime - Time;
		float move_tower = 1.0f*(currentTime - Time);	// move_tower还没写好
		//move_tower+= 1.0f*(currentTime - Time);

		computeMatricesFromInputs();
				


		////// Start of the rendering of the Ball //////


		glUseProgram(programID);

		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		if(Ball_status != -1)
			glBindTexture(GL_TEXTURE_2D, Texture[1]);
		else
			glBindTexture(GL_TEXTURE_2D, Texture[4]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);


		float ball_move = 1.0f*(currentTime - Time); //速度*（当前时刻-游戏开始时刻）--- 碰撞检测补充
		// -0.75是第一层高
		float ball_Y = -0.75f;

		/*
		float dt = (currentTime - Time);
		float v0 = 0.2f;
		float Y = 0.0f;

		current_step = 1;
		y += direction * (v0 * t + 0.5*G*t*t);
		if (direction == 1) {
			v0 -= G * t;
		}
		else if (direction == -1) {
			v0 += G * t;
		}
		if (y > maximumHeight) {
			y = maximumHeight;
			direction = -1;

		}
		else if (y < radius) {
			y = radius;
			printf("%f\n", Angel);


			if ((Angel > S[current_step].go[1] && Angel < S[current_step].go[2]) || (Angel > S[current_step].go[1] && Angel < S[current_step].go[2]) || (Angel > S[current_step].go[5] && Angel < S[current_step].go[6])) {
				Ball_status = 1; //fall
				direction = -1;

			}else{
				Ball_status = 0; //bound
				direction = 1;

			}

		}*/

		//InitSteps();

		int flag56=0;
		if (S[current_step].go[5] < S[current_step].go[6])
		{
			if (Angle > S[current_step].go[5] && Angle < S[current_step].go[6])
			{
				flag56 = 1;
				printf("%f,%f,%f\n", Angle, S[current_step].go[5], S[current_step].go[6]);
			}
		}
		else if ((S[current_step].go[5] > S[current_step].go[6])&&(Angle > S[current_step].go[5] && Angle < 360) || (Angle > 0 && Angle < S[current_step].go[6]))
		{
			flag56 = 1;
			printf("%d,%f,%f,%f\n", Angle, S[current_step].go[5], S[current_step].go[6]);
		}

		int flag12 = 0;
		if (S[current_step].go[1] < S[current_step].go[2])
		{
			if (Angle > S[current_step].go[1] && Angle < S[current_step].go[2])
			{
				flag12 = 1;
				printf("%f,%f,%f\n", Angle, S[current_step].go[1], S[current_step].go[2]);
			}
		}
		else if ((S[current_step].go[1] > S[current_step].go[2])&&(Angle > S[current_step].go[1] && Angle < 360) || (Angle > 0 && Angle < S[current_step].go[2]))
		{
			flag12 = 1;
			printf("%f,%f,%f\n", Angle, S[current_step].go[1], S[current_step].go[2]);
		}

		int flag34 = 0;
		if (S[current_step].go[3] < S[current_step].go[4])
		{
			if (Angle > S[current_step].go[3] && Angle < S[current_step].go[4])
			{
				flag34 = 1;
				printf("%f,%f,%f\n", Angle, S[current_step].go[3], S[current_step].go[4]);
			}
		}
		else if ((S[current_step].go[3] > S[current_step].go[4])&&(Angle > S[current_step].go[3] && Angle < 360) || (Angle > 0 && Angle < S[current_step].go[4]))
		{
			flag34 = 1;
			printf("%f,%f,%f\n", Angle, S[current_step].go[3], S[current_step].go[4]);
		}

		//printf("%f~%f\n", S[current_step].go[1], S[current_step].go[2]);
		//if ((Angle > S[current_step].go[1] && Angle < S[current_step].go[2]) || (Angle > S[current_step].go[3] && Angle < S[current_step].go[4]) || (Angle > S[current_step].go[5] && Angle < S[current_step].go[6]))
		if (flag12 || flag34 || flag56)
		{
			Ball_status = 1; //fall
		}
		else if ((Angle > S[current_step].end[1] && Angle < S[current_step].end[2]) || (Angle > S[current_step].end[3] && Angle < S[current_step].end[4]))
		{
			Ball_status = -1; //end//test
		}
		else
		{
			Ball_status = 0; //stop
		}


		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0.0f, y - 0.8f, 0.0f));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer2);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices2.size(), GL_UNSIGNED_SHORT, (void*)0);


		////// End of rendering of the Ball //////




		ModelMatrix1 = getModelMatrix_tower();

		//读取键盘信息，改变塔和阶梯的ModelMatrix
		//if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		/*float move = 1.0f*(currentTime - Time);*/

		printf("level : %d\n", current_step);
	
		//float basetime = Time;
		//move_tower = 1.0f*(currentTime - basetime);	// move_tower还没写好
		float move;
		if (Ball_status == 1)
		{
			ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, (current_step-1)*1.2f + move_tower-0.04f, 0.0f));
			if (move_tower >=  1.2f)
			{
				move = move_tower;
				//move_tower = current_step * 1.2f;
				current_step++;
				Ball_status = 0;
			}
		}
		if (Ball_status == 0 || current_step == 16)
		{
			//printf("tower move : %f, current_step : %d\n", move_tower, current_step);
			//ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, -0.0f + 1.2f*(current_step - 1), 0.0f));
			Time = currentTime;
			ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, (current_step-1)*1.2f-0.04f, 0.0f));
		}

		if (Ball_status == -1)
		{

			// Draw the triangles !
			/// start of the rendering of the "GAME OVER"  ///


			mat4 ModelMatrix5 = glm::mat4(1.0);
			glm::translate(ModelMatrix5, glm::vec3(0.0f, 100.8f, 0.0f));
			if (Ball_status == -1) {
				glm::translate(ModelMatrix5, glm::vec3(0.0f, -100.8f, 0.0f));
			}
			glm::mat4 MVP5 = ProjectionMatrix * ViewMatrix * ModelMatrix5;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP5[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix5[0][0]);


			// The rest is exactly the same as the first object

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer5);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer5);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer5);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer5);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, indices5.size(), GL_UNSIGNED_SHORT, (void*)0);



			////// End of rendering of the "GAME OVER" //////

					   		
		}
		/*if (Ball_status == 2) //fall
;		{
			printf("\n status = fall!\n");
			y = 0;
			
			//ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, -0.75f + move_tower, 0.0f));
			printf("%f\n", move_tower);
			if (move_tower >= current_step * 1.2f)
			{
				//move_tower = current_step * 1.2f;
				current_step++;
				Ball_status = 0;
			}
		}
		else if (Ball_status == 0) //stop
		{
			y = 0;
			//move_tower =  1.2f*(current_step-1);
			printf("tower move : %f, current_step : %d\n", move_tower,current_step);
			ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, -0.75f + 1.2f*(current_step - 1), 0.0f));
		}*/
		printf("%f\n", Angle);

		//ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, -0.75f + move_tower, 0.0f));
		/*}
		if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			ModelMatrix4 = glm::rotate(ModelMatrix4, deltaTime  * 2.0f, glm::vec3(0.0f, -1.0f, 0.0f));
		}*/

		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture[0]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		////// End of rendering of the first object //////
	
		
		

		////// Start of the rendering of the Normal Steps //////

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture[2]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);


		float move3 = 1.0f*(currentTime - Time);
		//float move = currentTime * 0.002f;

		// Compute the MVP matrix from keyboard and mouse input
		//读取键盘信息，改变塔和阶梯的ModelMatrix
		
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			Screen_shot();
		}

		
		glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix1;
			 
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer3);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer3);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices3.size(), GL_UNSIGNED_SHORT, (void*)0);

		
		////// End of rendering of the Normal Steps //////


		////// Start of the rendering of the Special Steps //////

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture[3]);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer4);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer4);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices3.size(), GL_UNSIGNED_SHORT, (void*)0);

		////// end of the rendering of the Special Steps //////


		







		//Time = currentTime;
		////// End of rendering of the steps //////

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//system("pause");

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	
	return 0;
}
bool InitSteps() {

	for (GLuint i = 0; i < 17; i++) {
		S[i].go_num = S[i].end_num = S[i].stop_num = -1;
		for (int j = 1; j < 7; j++) {
			S[i].go[j] = -100000;
			//S[i].stop[j] = -100000;
		}
		for (int j = 1; j < 3; j++) {
			S[i].end[j] = -100000;
		}
		S[i].Y = i * 1.21f;
	}
	// 1
	S[1].go_num = 2;
	//S[1].stop_num = 2;
	S[1].end_num = 0;
	S[1].go[1] = 234; S[1].go[2] = 306;
	S[1].go[3] = 54; S[1].go[4] = 126;

	S[2].go_num = 1;
	S[2].end_num = 0;
	S[2].go[1] = 126; S[2].go[2] = 270;

	S[3].go_num = 2;
	S[3].end_num = 0;
	S[3].go[1] = 324;
	S[3].go[2] = 36;
	S[3].go[3] = 126;
	S[3].go[4] = 234;

	S[4].go_num = 3;
	S[4].end_num = 0;
	S[4].go[1] = 0;
	S[4].go[2] = 54;
	S[4].go[3] = 126;
	S[4].go[4] = 180;
	S[4].go[5] = 234;
	S[4].go[6] = 288;

	S[5].go_num = 2;
	S[5].end_num = 1;
	S[5].go[1] = 72;
	S[5].go[2] = 126;
	S[5].go[3] = 198;
	S[5].go[4] = 252;
	S[5].end[1] = 252;
	S[5].end[2] = 288;

	S[6].go_num = 2;
	S[6].end_num = 1;
	S[6].go[1] = 108;
	S[6].go[2] = 162;
	S[6].go[3] = 270;
	S[6].go[4] = 306;
	S[6].end[1] = 162;
	S[6].end[2] = 198;

	S[7].go_num = 2;
	S[7].end_num = 0;
	S[7].go[1] = 36;
	S[7].go[2] = 126;
	S[7].go[3] = 180;
	S[7].go[4] = 270;

	S[8].go_num = 3;
	S[8].end_num = 0;
	S[8].go[1] = 36;
	S[8].go[2] = 90;
	S[8].go[3] = 144;
	S[8].go[4] = 198;
	S[8].go[5] = 270;
	S[8].go[6] = 324;


	S[9].go_num = 2;//这个好像歪的有点厉害
	S[9].end_num = 1;
	S[9].go[1] = 126;
	S[9].go[2] = 162;
	S[9].go[3] = 270;
	S[9].go[4] = 0;
	S[9].end[1] = 162;
	S[9].end[2] = 198;

	S[10].go_num = 2;
	S[10].end_num = 0;
	S[10].go[1] = 126;
	S[10].go[2] = 180;
	S[10].go[3] = 288;
	S[10].go[4] = 0;

	S[11].go_num = 3;
	S[11].end_num = 0;
	S[11].go[1] = 108;
	S[11].go[2] = 162;
	S[11].go[3] = 216;
	S[11].go[4] = 270;
	S[11].go[5] = 342;
	S[11].go[6] = 36;

	S[12].go_num = 3;//歪得厉害+1
	S[12].end_num = 1;
	S[12].go[1] = 0;
	S[12].go[2] = 72;
	S[12].go[3] = 180;
	S[12].go[4] = 216;
	S[12].go[5] = 270;
	S[12].go[6] = 306;
	S[12].end[1] = 144;
	S[12].end[2] = 280;

	S[13].go_num = 2;
	S[13].end_num = 0;
	S[13].go[1] = 18;
	S[13].go[2] = 72;
	S[13].go[3] = 216;
	S[13].go[4] = 270;

	S[14].go_num = 3;
	S[14].end_num = 0;
	S[14].go[1] = 18;
	S[14].go[2] = 72;
	S[14].go[3] = 144;
	S[14].go[4] = 180;
	S[14].go[5] = 252;
	S[14].go[6] = 306;

	S[15].go_num = 3;
	S[15].end_num = 1;
	S[15].go[1] = 0;
	S[15].go[2] = 36;
	S[15].go[3] = 72;
	S[15].go[4] = 126;
	S[15].go[5] = 252;
	S[15].go[6] = 306;
	S[15].end[1] = 126;
	S[15].end[2] = 162;

	S[16].go_num = 1;
	S[16].end_num = 0;
	S[16].go[1] = 0;
	S[16].go[2] = 0;



	return true;
}