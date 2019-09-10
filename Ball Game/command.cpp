// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "command.h"
extern float Angle;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::mat4 ModelMatrix_tower;
glm::mat4 ModelMatrix_ball;	// ���ģ��


glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}
glm::mat4 getModelMatrix_tower() {
	return ModelMatrix_tower;
}
glm::mat4 getModelMatrix_ball() {
	return ModelMatrix_ball;
}

// ��ʼ�������λ�� : on +Z
glm::vec3 position = glm::vec3(0, 0, 8);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.003f;


// ��ȡ���̺���������������ݺ����ı仯
void computeMatricesFromInputs() {

	// ��ʼ���ú���
	static double lastTime = glfwGetTime();

	// ����֮֡��ļ�������ƶ�����
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// ��ȡ��곯��
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// �ѹ��ŵ���Ļ���ģ�����ŵ���Ļ���ʧȥ��Ӧ
	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);	// ��ʱ������Ļ1024*768 4:3������glwfGetWindowSize�趨���ڴ�С

	// float(1024/2 - xpos ) ����봰�ڵľ��룬ֵԽ��ת����Խ��
	// mouseSpeed ���ٻ������ת
	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	// ����ռ��¼���������߷��������
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// ������ƶ��۲�
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// ���ұߺ����߷�������Ϸ���
	glm::vec3 up = glm::cross(right, direction);

	// move the camara to Zoom In/Out

	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}

	// Left / Right 
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position += right * deltaTime * speed;
		
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
		
	}

	// up
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		position -= up * deltaTime * speed;
	}
	// down
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position += up * deltaTime * speed;
	}

	// Rotate
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		ModelMatrix_tower = glm::rotate(ModelMatrix_tower, glm::radians(deltaTime * 200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Angle += deltaTime * 200.0f;
		while (Angle >= 360) {
			Angle -= 360;
		}
		
		while (Angle <= 0) {
			Angle += 360;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		ModelMatrix_tower = glm::rotate(ModelMatrix_tower, glm::radians(deltaTime * 200.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		Angle -= deltaTime * 200.0f;

		while (Angle >= 360) {
			Angle -= 360;
		}
		
		while (Angle <= 0) {
			Angle += 360;
		}
	}

	float FoV = initialFoV;// -

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
