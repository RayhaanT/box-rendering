#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <vector>

#define smoothingConstant 0.2f
#define PI 3.14159265358979323846264338f

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

#define C_PI 3.14159265359

// Default camera values
float W = 800;
float H = 600;
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVTY = 0.08f;
const float ZOOM = 45.0f;
const float CAMERA_DISTANCE = 20.0f;
const float CIRCUMFERENCE = CAMERA_DISTANCE * 2 * PI;
bool clicked;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void arcballScroll(double yOffset);

//Processes input and calculates euler values and view matrix
class Camera
{
public:
	//Camera properties
	glm::vec3 Position;
	glm::vec3 ballPos;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//Euler angles
	float Yaw;
	float Pitch;
	//Options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	float RotationSpeed = 0;
	const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	glm::mat4 GetArcMatrix()
	{
		return arcMatrix;
	}

	glm::mat4 GetReverseArcMatrix()
	{
		return reverseArcMatrix;
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(GLFWwindow *window, float deltaTime, bool restrictY)
	{
		float camspeed = 7 * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, true);
		}
		if (glfwGetKey(window, GLFW_KEY_E))
		{
			RotationSpeed += 1 * smoothingConstant * camspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			RotationSpeed -= 1 * smoothingConstant * camspeed;
		}
		//Move without flying: along x and z only
		if (restrictY)
		{
			if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
				Position += camspeed * glm::vec3(Front.x, 0.0f, Front.z);
			if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
				Position -= camspeed * glm::vec3(Front.x, 0.0f, Front.z);
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
				Position += camspeed * Front;
			if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
				Position -= camspeed * Front;
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateCameraVectors();
	}

	void SetRadius(double xPos, double yPos)
	{
		// Adjust for origin at top-left corner
		xPos -= W / 2;
		yPos -= H / 2;
		// Convert to screen coords
		xPos /= W;
		xPos *= -2;
		yPos /= H;
		yPos *= -2;
		sphereRadius = sqrt((xPos * xPos) + (yPos * yPos));

		lastPos2D = glm::vec2(xPos, yPos);
	}

	void ProcessArcBall(float xPos, float yPos)
	{
		// Adjust for origin at top-left corner
		xPos -= W / 2;
		yPos -= H / 2;
		// Convert to screen coords
		xPos /= W;
		xPos *= -2;
		yPos /= H;
		yPos *= -2;

		float xoffset = xPos - lastPos2D.x;
		float yoffset = yPos - lastPos2D.y;

		glm::quat xRotation = glm::angleAxis(-(float)(xoffset * C_PI) / (sphereRadius * 2), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat yRotation = glm::angleAxis(-(float)(yoffset * C_PI) / (sphereRadius * 2), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 xMatrix = glm::toMat4(xRotation);
		glm::mat4 yMatrix = glm::toMat4(yRotation);

		glm::quat xRotationReverse = glm::angleAxis((float)(-xoffset * C_PI) / (sphereRadius * 2), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat yRotationReverse = glm::angleAxis((float)(-yoffset * C_PI) / (sphereRadius * 2), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 xMatrixReverse = glm::toMat4(xRotationReverse);
		glm::mat4 yMatrixReverse = glm::toMat4(yRotationReverse);

		yMatrixReverse *= xMatrixReverse;
		yMatrixReverse *= reverseArcMatrix;
		reverseArcMatrix = yMatrixReverse;
		yMatrix *= xMatrix;
		yMatrix *= arcMatrix;
		arcMatrix = yMatrix;
		lastPos2D = glm::vec2(xPos, yPos);
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLFWwindow *window, float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;

		Position += zoomSense * Front * yoffset;
		//ProcessArcBall(lastPos2D.x, lastPos2D.y);
	}

private:
	float sphereRadius = 1.0f;
	glm::mat4 arcMatrix;
	glm::mat4 reverseArcMatrix;
	glm::vec2 lastPos2D = glm::vec2(0.0f);
	const float zoomSense = 0.4f;

	// Code used to find point on a sphere from a given point (pythagoras method)
	glm::vec3 GetSurfacePoint(float x, float y)
	{
		glm::vec3 newPos;
		float xySquare = x * x + y * y;

		glm::vec2 tempPoint(x, y);
		float tempMagnitude = GetMagnitude(glm::vec3(tempPoint, 0.0f));
		int zone = floor(tempMagnitude / sphereRadius);

		if (zone % 4 == 1 || zone % 4 == 2)
		{
			float desiredLength = GetModulus(tempMagnitude, sphereRadius) + sphereRadius * (zone % 4 == 1 ? 1 : 2);
			if (tempMagnitude != 0)
			{
				tempPoint = (desiredLength / tempMagnitude) * tempPoint;
			}

			newPos = glm::vec3(-tempPoint + (2.0f * sphereRadius * glm::normalize(tempPoint)), 0.0f);
			xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
			newPos.z = -sqrt((sphereRadius * sphereRadius) - xySquare);
		}
		// Code for 3rd quadrant if computed on its own:
		/*else if(zone % 4 == 2) {
			float desiredLength = GetModulus(tempMagnitude, sphereRadius);
			if(tempMagnitude != 0) {
				tempPoint = (desiredLength / tempMagnitude) * tempPoint;
			}

			newPos = -glm::vec3(tempPoint, 0.0f);
			xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
			newPos.z = -sqrt((sphereRadius * sphereRadius) - xySquare);
		}*/
		else if (zone % 4 == 3)
		{
			float desiredLength = GetModulus(tempMagnitude, sphereRadius) + sphereRadius;
			if (tempMagnitude != 0)
			{
				tempPoint = (desiredLength / tempMagnitude) * tempPoint;
			}

			newPos = glm::vec3(tempPoint - (2.0f * sphereRadius * glm::normalize(tempPoint)), 0.0f);
			xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
			newPos.z = sqrt((sphereRadius * sphereRadius) - xySquare);
		}
		else
		{
			float desiredLength = GetModulus(tempMagnitude, sphereRadius);
			if (tempMagnitude != 0)
			{
				tempPoint = (desiredLength / tempMagnitude) * tempPoint;
			}

			newPos = glm::vec3(tempPoint, 0.0f);
			xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
			newPos.z = sqrt((sphereRadius * sphereRadius) - xySquare);
		}

		return newPos;
	}

	float GetMagnitude(glm::vec3 v)
	{
		return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	}

	//Modulus of floating points
	float GetModulus(float base, float divisor)
	{
		float mod;
		// Handle negatives
		if (base < 0)
			mod = -base;
		else
			mod = base;
		if (divisor < 0)
			divisor = -divisor;

		while (mod >= divisor)
			mod = mod - divisor;

		if (base < 0)
			return -mod;

		return mod;
	}

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void updateArcVectors()
	{
		Right = glm::normalize(glm::cross(glm::normalize(Position - origin), WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		//float simpleAngle = abs(GetModulus(lat*180/C_PI, 360));
		float simpleAngle = 0;

		//Check which half of the sphere the angle is in
		if (simpleAngle < 90 || simpleAngle > 270)
		{
			//Calculate up with global up of (0, 1, 0)
			Up = glm::normalize(glm::cross(Right, glm::normalize(Position - origin)));
		}
		else
		{
			//Calculate up with global up of (0, -1, 0) -- Reversing order of cross results in this
			Up = glm::normalize(glm::cross(glm::normalize(Position - origin), Right));
		}
	}
};
#endif