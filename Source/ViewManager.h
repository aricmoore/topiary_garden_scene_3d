///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"

// GLFW library
#include "GLFW/glfw3.h" 

class ViewManager
{
public:
	// Constructor
	ViewManager(
		ShaderManager* pShaderManager);
	// Destructor
	~ViewManager();

	// Camera object pointer:
	// The camera was added here (instead of being managed as a global) 
	// so that ViewManager can directly handle input and update camera 
	// state (position, orientation, projection mode) as part of managing 
	// the view. This encapsulates scene interaction logic in one place.
	Camera* m_pCamera;

	// Mouse position callback for mouse interaction with the 3D scene
	static void MouseCallback(GLFWwindow* window, double xMousePos, double yMousePos);

	// Scroll callback for zooming in and out
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
	// For mouse movement
	float lastX;
	float lastY;
	bool firstMouse;
	float movementSpeedFactor;     // speed factor adjustable with scroll
	bool bOrthographicProjection;  // true = orthographic, false = perspective

	// Pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// Active OpenGL display window
	GLFWwindow* m_pWindow;

public:
	// Create the initial OpenGL display window
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);
	
	// Prepare the conversion from 3D object display to 2D scene display
	void PrepareSceneView();

	// Process keyboard events for interaction with the 3D scene
	void ProcessKeyboardEvents(float deltaTime);

	// Returns true if orthographic projection is enabled, false if perspective
	bool IsOrthographicProjection() const { return bOrthographicProjection; }
};