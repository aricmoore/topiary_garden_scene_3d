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
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the ViewManager class.
 *  This initialises the shader manager reference, camera,
 *  window pointer, and input-related state variables.
 * 
 *  It also sets up the camera with a zoomed-out starting
 *  position and orientation (compared to the default), 
 *  preparing for either a perspective or orthographic projection.
 * 
 *  IMPORTANT NOTE on design change:
 *  Previously, the camera was accessed via a global pointer
 *  (g_pCamera). This was replaced with a member variable
 *  (m_pCamera) so that each ViewManager instance owns and
 *  manages its own camera. This avoids reliance on global
 *  state, makes the class more self-contained, and improves
 *  flexibility (for example, supporting multiple view managers).
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// Initialise the member variables
	m_pShaderManager = pShaderManager;  // store shader manager reference
	m_pWindow = NULL;					// initialise window pointer
	m_pCamera = new Camera();			// create a new camera instance

	// Set initial zoomed-out camera (compared to default)
	m_pCamera->Position = glm::vec3(0.0f, 10.0f, 30.0f);  // back farther and higher up
	m_pCamera->Front = glm::vec3(0.0f, -0.3f, -1.0f);     // look slightly downward
	m_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);		  // keep world Y as up
	m_pCamera->Zoom = 90;                                 // wider field of view

	// Initialise mouse input state variables
	lastX = WINDOW_WIDTH / 2.0f;   // centre X position
	lastY = WINDOW_HEIGHT / 2.0f;  // centre Y position
	firstMouse = true;			   // flag for first mouse event
	movementSpeedFactor = 1.0f;    // default movement speed

	// Default to perspective projection
	bOrthographicProjection = false;
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
	if (NULL != m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Store 'this' pointer for static callbacks
	glfwSetWindowUserPointer(window, this);

	// This callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::MouseCallback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  MouseCallback() (renamed from Mouse_Position_Callback())
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 * 
 *  Design note:
 *  Camera control now uses the member variable m_pCamera
 *  instead of the old global g_pCamera. This makes the code
 *  object-oriented: each ViewManager manages its own camera
 *  rather than relying on a shared global.
 *
 *  Function:
 *  - Tracks the mouse position across frames
 *  - Calculates movement offsets
 *  - Passes offsets to the Camera to update its look direction
 ***********************************************************/
void ViewManager::MouseCallback(GLFWwindow * window, double xMousePos, double yMousePos)
{
	ViewManager* vm = static_cast<ViewManager*>(glfwGetWindowUserPointer(window));
	if (!vm) return;

	Camera* camera = vm->m_pCamera;

	// First time the mouse moves: just store its position
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// Calculate movement relative to last position
	float xoffset = xMousePos - gLastX;
	float yoffset = gLastY - yMousePos;  // inverted: screen Y grows downward
	gLastX = xMousePos;
	gLastY = yMousePos;

	// Adjust sensitivity (smaller factor equates to smoother motion)
	float sensitivity = 0.1f;  // tweak as needed?
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// Update the camera’s horizontal/vertical look direction
	camera->ProcessMouseMovement(xoffset, yoffset);
}

/***********************************************************
 *  ScrollCallback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is used within the active GLFW
 *  display window. It adjusts the camera's movement speed
 *  based on the scroll input.
 * 
 *  Design note:
 *  Uses m_pCamera instead of a global camera pointer.
 *
 *  Function:
 *  - Adjusts camera zoom / movement speed based on scroll input
 ***********************************************************/
void ViewManager::ScrollCallback(GLFWwindow * window, double xOffset, double yOffset)
{
	ViewManager* vm = static_cast<ViewManager*>(glfwGetWindowUserPointer(window));
	if (!vm) return;

	// Adjust camera speed directly in Camera class
	vm->m_pCamera->ProcessMouseScroll(static_cast<float>(yOffset));
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 * 
 *  Design note:
 *  Uses m_pCamera (per-instance) instead of a global camera
 *  pointer for cleaner encapsulation.
 *
 *  Function:
 *  - ESC closes the window
 *  - WASD + QE keys move the camera
 *  - O/P switch between Orthographic and Perspective modes
 *  - Numpad + / - adjust mouse sensitivity
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents(float deltaTime)
{
	// Close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// Movement speed is frame-rate independent and adjusted with scroll wheel
	float speed = deltaTime * movementSpeedFactor;  // movementSpeedFactor is updated via ScrollCallback

	// Camera movement controls
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(FORWARD, speed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(BACKWARD, speed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(LEFT, speed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(RIGHT, speed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(DOWN, speed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		m_pCamera->ProcessKeyboard(UP, speed);

	// Projection toggle keys: O = Orthographic, P = Perspective
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		if (!bOrthographicProjection) {
			bOrthographicProjection = true;
			// DEBUG: uncomment line below to see if switching occurs
			//std::cout << "Switched to Orthographic\n";
		}
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		if (bOrthographicProjection) {
			bOrthographicProjection = false;
			// DEBUG: uncomment line below to see if switching occurs
			//std::cout << "Switched to Perspective\n";
		}
	}

	// Adjust mouse sensitivity with Numpad + and - keys (my trackpad might just suck)
	if (glfwGetKey(m_pWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS)  // Numpad +
		m_pCamera->MouseSensitivity += 0.01f;

	if (glfwGetKey(m_pWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)  // Numpad -
		m_pCamera->MouseSensitivity -= 0.01f;

	// Limit sensitivity to avoid going too low or negative
	if (m_pCamera->MouseSensitivity < 0.01f)
		m_pCamera->MouseSensitivity = 0.01f;
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering.
 * 
 *  This method sets up the camera view and projection
 *  matrices that determine how the 3D world is rendered in
 *  each frame. It handles both orthographic and perspective
 *  projections, applies per-frame timing to ensure smooth
 *  camera movement, and sends the final matrices and camera
 *  position to the active shader program so geometry can be
 *  transformed correctly during rendering.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// Per-frame timing is used to normalize movement across 
	// machines with different frame rates (ensures time-based, 
	// not frame-based, updates).
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// Process any keyboard events that may be waiting in the
	// event queue, using frame time for smooth movement.
	// NOTE: WASD/QE camera movement is frame-rate independent.
	ProcessKeyboardEvents(gDeltaTime);

	// Camera setup: Orthographic vs Perspective projection
	// determines how 3D coordinates are mapped to the screen.
	if (bOrthographicProjection)
	{
		float orthoSize = 30.0f; // base “radius” for the scene

		// Near/far planes are chosen so the ground plane
		// doesn’t clip in the orthographic top-down view.
		float nearPlane = 20.0f;
		float farPlane = 100.0f;

		// Adjust ortho extents based on window aspect ratio to prevent stretching.
		float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
		float orthoWidth, orthoHeight;

		if (aspect >= 1.0f) {
			orthoWidth = orthoSize * aspect;
			orthoHeight = orthoSize;
		}
		else {
			orthoWidth = orthoSize;
			orthoHeight = orthoSize / aspect;
		}

		// Construct orthographic projection matrix.
		projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, nearPlane, farPlane);

		// Camera setup: positioned above scene, looking straight down.
		glm::vec3 orthoPos = glm::vec3(0.0f, 50.0f, 0.0f);    // elevated Y
		glm::vec3 orthoFront = glm::vec3(0.0f, -1.0f, 0.0f);  // facing downward
		glm::vec3 orthoUp = glm::vec3(0.0f, 0.0f, -1.0f);     // define "up" as -Z for correct orientation

		// Generate view matrix from orthographic camera.
		view = glm::lookAt(orthoPos, orthoPos + orthoFront, orthoUp);
	}
	else
	{
		// Perspective projection simulates human vision with
		// depth (objects shrink with distance).
		projection = glm::perspective(glm::radians(m_pCamera->Zoom),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
			0.1f,     // near plane (very close to camera)
			100.0f);  // far plane (scene cutoff)

		// View matrix derived from the active camera object.
		view = m_pCamera->GetViewMatrix();
	}

	// Send matrices and camera position to the shader program.
	// The shader uses these values to transform 3D coordinates
	// into screen space and apply lighting based on camera pos.
	if (m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", m_pCamera->Position);
	}
}