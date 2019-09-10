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
glm::mat4 ModelMatrix_ball;	// 球的模型


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

// 初始的摄像机位置 : on +Z
glm::vec3 position = glm::vec3(0, 0, 8);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.003f;


// 读取键盘和鼠标操作，计算阶梯和塔的变化
void computeMatricesFromInputs() {

	// 开始调用函数
	static double lastTime = glfwGetTime();

	// 用两帧之间的间隔控制移动步长
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// 获取鼠标朝向
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// 把光标放到屏幕中心，否则放到屏幕外会失去响应
	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);	// 暂时设置屏幕1024*768 4:3，可用glwfGetWindowSize设定窗口大小

	// float(1024/2 - xpos ) 鼠标离窗口的距离，值越大，转动角越大
	// mouseSpeed 加速或减慢旋转
	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	// 世界空间下计算代表视线方向的向量
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// 摄像机移动观察
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// 用右边和视线方向计算上方向
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
