#include <GLFW\glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))
#include <OpenGLHeaders/Camera.h>
#include <OpenGLHeaders/Texture.h>
#include <OpenGLHeaders/Shader.h>
#include <string>
#include <vector>
#include <algorithm>
///VBOs Vertex Buffer Objects contain vertex data that is sent to memory in the GPU, vertex attrib calls config bound VBO
///VAOs Vertex Array Objects when bound, any vertex attribute calls and attribute configs are stored in VAO
///Having multiple VAOs allow storage of multiple VBO configs, before drawing, binding VAO with right config applies to draw
///Vertex attributes are simply inputs of the vertex shader, configured so vertex shader knows how to interpret data
///EBOs Element Buffer Objects stores indices used for OpenGL to know which vertices to draw
///EBOs used when needing multiple triangles to render but triangles have overlapping points
///E.G. rectangle made with 2 triangles have 2 overlapping vertices
///Vertex shader changes 3D coords of vertices
///Fragment shader sets color of pixels
///OpenGL objects are used to reference pieces of OpenGL state machine
///E.G. when creating a shader program,  a program is created in the state machine and its ID
///is passed to the program object
///Similarly, with the VBO, the real VBO is stored in the background state machine, the
///object holds the ID of the real object and its value is bound to the real object
///With ALL (or most) OpenGL objects, they must be bound so that any function calls for that object type configures the
///ID you created
///Depth information stored in Z buffer, depth testing done automatically, must be enabled
///Depth buffer must also be cleared in the clear function

#define NUMBER_OF_LAYERS 6
#define MIN_STEP 0.001f
#define STEP_STEP 0.02f
#define SCROLL_MULTIPLIER 2

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool restrictY = true;

float step = MIN_STEP;

struct Layer {
	unsigned int texture;
	glm::vec3 position;
	glm::vec3 offset;

	Layer(unsigned int _texture) {
		texture = _texture;
	}

	Layer() {}
};

//Define offset variables
float lastX = W / 2; float lastY = H / 2;
float yaw = -90; float pitch = 0;
bool firstMouse = true;
float fov = 45.0f;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool blackBackground = true;
bool rHeld = false;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch);

bool sortLayers(Layer a, Layer b) {
	return a.position.z < b.position.z;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	if (step-STEP_STEP*yoffset*SCROLL_MULTIPLIER > MIN_STEP)
		step -= STEP_STEP*yoffset*SCROLL_MULTIPLIER;
	else
		step = MIN_STEP;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if(glfwGetKey(window, GLFW_KEY_E)) {
		step += STEP_STEP;
	}
	if(glfwGetKey(window, GLFW_KEY_Q)) {
		if(step-STEP_STEP > MIN_STEP){
			step -= STEP_STEP;
		}
		else {
			step = MIN_STEP;
		}
	}

	if(glfwGetKey(window, GLFW_KEY_SPACE)) {
		camera.ProcessMouseScroll(window, 1);
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		camera.ProcessMouseScroll(window, -1);
	}

	if(glfwGetKey(window, GLFW_KEY_R) && !rHeld) {
		blackBackground = !blackBackground;
		rHeld = true;
	}
	else
		rHeld = false;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (clicked) {
		camera.ProcessArcBall(xpos, ypos);
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			clicked = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			camera.SetRadius(xpos, ypos);
		}
		else
			clicked = false;
	}
}

std::vector<glm::vec3> v3Insert(std::vector<glm::vec3> v, glm::vec3 in, int index) {
	v.push_back(glm::vec3(0.0f));
	for(int i = v.size()-1; i > index-1; i++) {
		if(i == index) {
			v[i] = in;
		}
		else {
			v[i] = v[i-1]; 
		}
	}
	return v;
}

std::vector<unsigned int> uiInsert(std::vector<unsigned int> v, unsigned int in, int index) {
	v.push_back(0);
	for(int i = v.size()-1; i > index-1; i++) {
		if(i == index) {
			v[i] = in;
		}
		else {
			v[i] = v[i]-1; 
		}
	}
	return v;
}


int main()
{
	//Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFW Window

	// Regular small window
	GLFWwindow  *window = glfwCreateWindow(W, H, "Exploded Rendering", NULL, NULL);
	glfwMakeContextCurrent(window);

	// Borderless fulscreen window
	// const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	// glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	// glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	// glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	// glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	// W = mode->width;
	// H = mode->height;
	// GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Exploded Rendering", glfwGetPrimaryMonitor(), NULL);
	// glfwMakeContextCurrent(window);

	//glad init: intializes all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Set size of rendering window
	glViewport(0, 0, W, H);

	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	float vertices[] = {
		 //Positions          //Normals            //Texture coords
		-0.9f, -1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		 0.9f, -1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.9f,  1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.9f,  1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		-0.9f,  1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		-0.9f, -1.2f, -0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	};
	//Create a Vertex Array Object
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Create a Vertex Buffer Object
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	//Bind the VBO to the object type and copy its data to the state
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Configure vertex data so readable by vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightingShader = 0;
	Shader("Shaders/VeShMap.vs", "Shaders/FrShMap.fs", lightingShader);
	glUseProgram(lightingShader);
	setInt(lightingShader, "material.diffuse", 0);
	setInt(lightingShader, "material.specular", 1);
	setFloat(lightingShader, "material.shininess", 32.0f);
	setVec3(lightingShader, "light.ambient", glm::vec3(1.0, 1.0, 1.0));
	setVec3(lightingShader, "light.diffuse", glm::vec3(0.0, 0.0, 0.0));
	setVec3(lightingShader, "light.specular", glm::vec3(0.0, 0.0, 0.0));

	//std::string temp1 = "D:/Libraries/Libraries/GitHub/layered-image-rendering/main/images/layer";
	std::string temp1 = "images/layer";
	std::string temp2 = ".png";
	std::vector<Layer> layers;
	glActiveTexture(GL_TEXTURE0);
	// Load layer textures
	for(int i = 0; i < NUMBER_OF_LAYERS; i++) {
		std::string numStr = std::to_string(i);
		std::string path = temp1 + numStr + temp2;
		unsigned int newTexture;
		std::cout << "Loading " << path.c_str() << std::endl;
		loadTexture(newTexture, path.c_str());
		layers.push_back(Layer(newTexture));
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Set mouse input callback function
	void mouse_callback(GLFWwindow *window, double xpos, double ypos);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Render Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		camera.ProcessKeyboard(window, deltaTime, true);
		glm::mat4 rotationModel = camera.GetArcMatrix();

		if(blackBackground)
			glClearColor(0, 0, 0, 1);
		else
			glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(lightingShader);
		glBindVertexArray(VAO);

		glm::mat4 model;
		//model = glm::rotate(model, (float)(glfwGetTime()) * 2, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), W / H, 0.1f, 100.0f);

		//Pass our matrices to the shader through a uniform
		setMat4(lightingShader, "view", view);
		setMat4(lightingShader, "projection", projection);
		setVec3(lightingShader, "lightPos", glm::vec3(glm::vec3(sin((float)(glfwGetTime())), 0.0f, cos((float)(glfwGetTime())))));
		//setVec3(lightingShader, "lightPos", lightPos);

		std::vector<Layer> sortedLayers = layers;
		//Sort layers
		for(int i = 0; i < NUMBER_OF_LAYERS; i++) {
			int shiftedI = i - NUMBER_OF_LAYERS / 2;
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, step * shiftedI);
			sortedLayers[i].offset = pos;
			pos = glm::vec3(glm::vec4(pos, 0.0f) * rotationModel);
			sortedLayers[i].position = pos;
		}
		std::sort(sortedLayers.begin(), sortedLayers.end(), sortLayers);

		for(int i = 0; i < NUMBER_OF_LAYERS; i++) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glActiveTexture(GL_TEXTURE0);
			model = glm::mat4();
			glBindTexture(GL_TEXTURE_2D, sortedLayers[i].texture);
			int shiftedI = i - NUMBER_OF_LAYERS/2;
			glm::vec3 v = sortedLayers[i].offset;
			model = glm::translate(model, v);
			model = rotationModel * model;
			setMat4(lightingShader, "model", model);
			//Draw layer
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));
		}

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}