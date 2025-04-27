///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

	bool perspective = true;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 9.0f, 18.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.8f, -3.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
	g_pCamera->MovementSpeed = 50;
	g_pCamera->MouseSensitivity = 0.5f;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// callback for scroll wheel to change the camera speed.
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		if (yoffset > 0) { // Scrolling up
			g_pCamera->MovementSpeed++; // Increase speed
		}
		else if (yoffset < 0) { // Scrolling down
			g_pCamera->MovementSpeed--; // Decrease speed
		}
		// Ensure cameraSpeed doesn't go below a certain threshold
		if (g_pCamera->MovementSpeed < 1) {
			g_pCamera->MovementSpeed = 1;
		}
		std::cout << "Camera Speed: " << g_pCamera->MovementSpeed << std::endl;
		});

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// when the first mouse move event is received, this needs to be recorded so that
// all subsequent mouse moves can correctly calculate the X position offset and Y
// position offset for proper operation
	if (perspective) {
		if (gFirstMouse)
		{
			gLastX = xMousePos;
			gLastY = yMousePos;
			gFirstMouse = false;
		}

		// calculate the X offset and Y offset values for moving the 3D camera accordingly
		float xOffset = xMousePos - gLastX;
		float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

		// set the current positions into the last position variables
		gLastX = xMousePos;
		gLastY = yMousePos;

		// move the 3D camera according to the calculated offsets
		g_pCamera->ProcessMouseMovement(xOffset, yOffset);
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// if the camera object is null, then exit this method
	if (NULL == g_pCamera)
	{
		return;
	}
	if (perspective) {
		// process camera zooming in and out
		if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
		}

		// process camera panning left and right
		if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
		}

		// Camera upwards and downwards movement
		if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(UP, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
		}
	}

	// Key bindings for changing between orthographic and perspective view
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		perspective = false;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		perspective = true;
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;

	//Initializes both orthographic and perspective matrices
	glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	glm::mat4 orthoMatrix = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{

		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering

		/* Checks boolean variable 'perspective' for user input, if the user selects perspective mode
		*  the perspective matrix is loaded to the shaders, if orthographic mode is selected, the
		*  orthographic matrix is loaded to the shaders.
		*/
		if (perspective) {
			m_pShaderManager->setMat4Value(g_ProjectionName, perspectiveMatrix);
		}
		else {
			g_pCamera->Position = glm::vec3(0.0f, 9.0f, 18.0f);
			g_pCamera->Front = glm::vec3(0.0f, -0.8f, -3.0f);
			g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
			m_pShaderManager->setMat4Value(g_ProjectionName, orthoMatrix);
		}
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}