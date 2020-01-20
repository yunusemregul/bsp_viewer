// Source:
// 	learnopengl.com

#include <GLFW/glfw3.h>
extern GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{
	return ProjectionMatrix;
}

glm::vec3 position = glm::vec3(0, 0, 5); 
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float initialFoV = 75.0f;

float speed = 25.0f; // 3 units / second
float mouseSpeed = 0.005f;

void computeMatricesFromInputs(){
	// disable input if we dont have focus
	int focused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
	if(!focused)
		return;

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	float curSpeed = speed;

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024/2 - xpos);
	verticalAngle   += mouseSpeed * float(768/2 - ypos);
	verticalAngle = glm::clamp<float>(verticalAngle,-1.75,1.75);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// speed up by pressing shift
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS)
		curSpeed = speed * 4;

	// slow down by pressing ctrl
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS)
		curSpeed = speed / 4;	

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS)
		position += direction * deltaTime * curSpeed;
	
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS)
		position -= direction * deltaTime * curSpeed;

	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS)
		position += right * deltaTime * curSpeed;

	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS)
		position -= right * deltaTime * curSpeed;

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 1000.0f);
	
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position+direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}