///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Fix to ensure that OpenGL does not assume 4-byte alignment (images whose row size is not multiple of 4)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// Set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// Set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// Set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

 /***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	//std::cout << "LoadSceneTextures() called" << std::endl;
	//CreateGLTexture("Textures/leaves1.jpg", "Leaves1");

	bool bLoaded1 = CreateGLTexture("Textures/leaves1.jpg", "Leaves1");
	if (!bLoaded1)
	{
		std::cout << "Failed to load Leaves1" << std::endl;
		return;  // abort if texture failed to load
	}

	bool bLoaded2 = CreateGLTexture("Textures/leaves2.jpg", "Leaves2");
	if (!bLoaded2)
	{
		std::cout << "Failed to load Leaves2" << std::endl;
		return;  // abort if texture failed to load
	}

	bool bLoaded3 = CreateGLTexture("Textures/gravel1.jpg", "Gravel1");
	if (!bLoaded3)
	{
		std::cout << "Failed to load Gravel1" << std::endl;
		return;  // abort if texture failed to load
	}

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method configures material properties for objects
 *  within the 3D scene. These determine how surfaces react
 *  to the lighting defined in SetupSceneLights().
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	if (!m_pShaderManager)
		return;

	// Foliage material (default)
	OBJECT_MATERIAL foliageMaterial;
	foliageMaterial.tag = "Foliage";
	foliageMaterial.ambientColor = glm::vec3(0.2f, 0.4f, 0.2f);
	foliageMaterial.ambientStrength = 0.5f;
	foliageMaterial.diffuseColor = glm::vec3(0.3f, 0.7f, 0.3f);
	foliageMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f);
	foliageMaterial.shininess = 16.0f;
	m_objectMaterials.push_back(foliageMaterial);

	// Ground material
	OBJECT_MATERIAL groundMaterial;
	groundMaterial.tag = "Ground";
	groundMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	groundMaterial.ambientStrength = 0.4f;
	groundMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	groundMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);
	groundMaterial.shininess = 8.0f;
	m_objectMaterials.push_back(groundMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method sets up up to two light sources for the scene.
 *  These lights are applied via the existing ShaderManager
 *  uniforms. The first is the primary light, the second is a
 *  softer fill light to reduce harsh shadows.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	if (!m_pShaderManager)
		return;

	// Enable lighting in shaders
	m_pShaderManager->setBoolValue("bUseLighting", true);

	// ----------------------------
	// Sunlight (directional, warm white)
	// ----------------------------
	// The "position" here is treated as a *direction vector* in the shader.
	glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));
	glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f); // warm sunlight tone

	m_pShaderManager->setVec3Value("lightSources[0].position", sunDirection);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", sunColor * 0.4f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", sunColor);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(1.0f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 1.0f);
	m_pShaderManager->setBoolValue("lightSources[0].isDirectional", true);

	// ----------------------------
	// Fill light (cool tint, point light)
	// ----------------------------
	glm::vec3 lightPos1 = glm::vec3(-8.0f, 6.0f, -8.0f);
	glm::vec3 lightColor1 = glm::vec3(0.3f, 0.4f, 0.6f); // bluish tone

	m_pShaderManager->setVec3Value("lightSources[1].position", lightPos1);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", lightColor1 * 0.15f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", lightColor1 * 0.6f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", lightColor1 * 0.8f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.5f);
	m_pShaderManager->setBoolValue("lightSources[1].isDirectional", false);

	// ----------------------------
	// Ground bounce (soft warm fill)
	// ----------------------------
	glm::vec3 bouncePos = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 bounceColor = glm::vec3(0.8f, 0.7f, 0.6f);

	m_pShaderManager->setVec3Value("lightSources[2].position", bouncePos);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", bounceColor * 0.05f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", bounceColor * 0.3f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.4f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 8.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.3f);
	m_pShaderManager->setBoolValue("lightSources[2].isDirectional", false);

	// ----------------------------
	// Disable unused light slots if shader expects four
	// ----------------------------
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", glm::vec3(0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.0f));
}


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();
	// Setup lights for the scene
	SetupSceneLights();
	// setup object materials
    DefineObjectMaterials();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadTaperedCylinderTreeTierMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadBoxMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes.
 * 
 *  This method orchestrates the layout of our entire garden 
 *  scene. It applies geometric transformations (scaling, 
 *  rotation, translation) to base meshes, assigns colours via 
 *  shaders, and calls helper methods to assemble compound 
 *  structures (hedges, bushes, decorative shapes). The scene 
 *  is rendered differently depending on whether an 
 *  orthographic or perspective view is selected.
 ***********************************************************/
void SceneManager::RenderScene(bool bOrthographic)
{
	// Declare the variables used to scale, rotate, and position meshes
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/****************************************************************/


	// ================================
	// 1) Ground Plane (the party starts here)
	// ================================
	// Plane provides a base surface for the scene. It's skipped in 
	// orthographic mode to test perspective changes.

	// Only draw plane if not orthographic
	if (!bOrthographic) 
	{
		scaleXYZ = glm::vec3(60.0f, 1.0f, 30.0f);  // widen plane in X and Z, keep Y flat

		// Set the XYZ rotation for the mesh
		XrotationDegrees = 0.0f;
		YrotationDegrees = 0.0f;
		ZrotationDegrees = 0.0f;

		// Set the XYZ position for the mesh
		positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);  // keep plane centred

		// Set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		// Plane: coloured surface only with lighting
		m_pShaderManager->use();
		SetShaderMaterial("Ground");
		SetShaderTexture("Gravel1");
		m_pShaderManager->setBoolValue("bUseLighting", true);
		m_pShaderManager->setBoolValue("bUseTexture", true);

		// Light grey shader colour ensures visibility when perspective is active
		//SetShaderColor(0.7f, 0.7f, 0.7f, 1.0f);

		float tileX = 20.0f;  // number of times texture repeats along X
		float tileZ = 20.0f;  // number of times texture repeats along Z
		m_pShaderManager->setVec2Value("UVscale", tileX, tileZ);

		// Draw the mesh with transformation values
		m_basicMeshes->DrawPlaneMesh();
	}

	/****************************************************************/


	// ================================
	// 2) Cylinders with sphere tips (topiary bushes)
	// ================================
	// Use helper function to draw decorative, bush-like structures.
	DrawCylinderWithSphereTip(glm::vec3(0.0f, 0.0f, 3.0f),
		7.0f,   // height
		2.5f);  // radius

	DrawCylinderWithSphereTip(glm::vec3(-12.0f, 0.0f, -2.0f),
		6.0f,
		2.0f);

	/****************************************************************/


	// ================================
	// 3) Torus (ring hedge around base)
	// ================================
	// Torus mesh is scaled and rotated to act as a ring hedge surrounding the main bushes.
	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);						   // large, flat ring
	positionXYZ = glm::vec3(0.0f, 0.5f, 3.0f);					   // position at ground level
	SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);  // rotate so torus is horizontal

	// Set texture and lighting
	m_pShaderManager->use();
	SetShaderMaterial("Foliage");
	SetShaderTexture("Leaves2");
	m_pShaderManager->setBoolValue("bUseLighting", true);
	m_pShaderManager->setBoolValue("bUseTexture", true);

	// Tone down the shininess
	m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(0.3f, 0.3f, 0.3f));
	m_pShaderManager->setFloatValue("material.shininess", 8.0f);

	// Adjust UV scaling so texture repeats instead of stretches.
	// Use a factor based on torus size; larger number = more repeats
	float torusTiling = 5.0f;
	m_pShaderManager->setVec2Value("UVscale", torusTiling, torusTiling);

	m_basicMeshes->DrawTorusMesh();

	/****************************************************************/


	// =====================================
	// 4) Rectangular Hedge Left (flat on plane)
	// =====================================
	// Draws a rectangular hedge aligned with the left bush.
	glm::vec3 secondComboPos = glm::vec3(-12.0f, 0.0f, -2.0f);
	DrawRectangularHedge(
		secondComboPos,						 // centre it around the cylinder
		10.0f,								 // length in X
		6.0f,								 // width in Z
		2.0f);								 // height in Y

	/****************************************************************/


	// ================================
	// 5) Outer Rectangular Hedge in front of torus
	// ================================
	// Creates a hedge boundary enclosing the torus.
	glm::vec3 outerHedgeCenter = glm::vec3(0.0f, 0.0f, 18.0f);  // in front of the torus, along Z
	float outerLength = 8.0f;
	float outerWidth = 10.0f;
	float hedgeHeight = 2.0f;

	DrawRectangularHedge(outerHedgeCenter, outerLength, outerWidth, hedgeHeight);

	/****************************************************************/


	// ================================
	// 6) Inner X-shaped hedges inside of the outer hedge
	// ================================
	// Adds decorative criss-cross hedges using rotated box meshes.
	float innerWidth = 1.0f;  // thickness of the hedge wall

	// innerLength: diagonal span of the X-shaped hedge across the inner rectangle,
	// accounting for hedge wall thickness (using the Pythagorean theorem)
	// Note: no need for a helper since it'll only be used once
	float innerLength = sqrt(
		(outerLength - 2 * innerWidth) * (outerLength - 2 * innerWidth) +
		(outerWidth - 2 * innerWidth) * (outerWidth - 2 * innerWidth)
	);

	// Diagonal 1 (from front-left to back-right)
	SetTransformations(glm::vec3(innerLength, hedgeHeight, innerWidth),
		0.0f, 45.0f, 0.0f, outerHedgeCenter);
	m_basicMeshes->DrawBoxMesh();

	// Diagonal 2 (from back-left to front-right)
	SetTransformations(glm::vec3(innerLength, hedgeHeight, innerWidth),
		0.0f, -45.0f, 0.0f, outerHedgeCenter);
	m_basicMeshes->DrawBoxMesh();
}

// ----------------------------------------------
// Helper Functions for Drawing Complex Objects
// ----------------------------------------------
void SceneManager::DrawCylinderWithSphereTip(glm::vec3 basePos, float cylinderHeight, float cylinderRadius)
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --------------------------
	// Cylinder body
	// --------------------------
	scaleXYZ = glm::vec3(cylinderRadius, cylinderHeight, cylinderRadius);
	positionXYZ = basePos;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// Activate shader and set material/texture BEFORE drawing
	m_pShaderManager->use();
	SetShaderMaterial("Foliage");
	SetShaderTexture("Leaves1");
	m_pShaderManager->setBoolValue("bUseLighting", true);
	m_pShaderManager->setBoolValue("bUseTexture", true);

	// Draw only the sides
	m_basicMeshes->DrawTaperedCylinderTreeTierMesh(false, false, true);

	// --------------------------
	// Sphere tip
	// --------------------------
	float cylinderTopY = positionXYZ.y + scaleXYZ.y;
	float topRadius = 0.05f * cylinderRadius;
	float sphereRadius = topRadius * 1.1f;
	float sphereCenterY = cylinderTopY - sphereRadius * 0.7f;

	scaleXYZ = glm::vec3(sphereRadius * 2.0f);
	positionXYZ = glm::vec3(basePos.x, sphereCenterY, basePos.z);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// Ensure shader and material are applied here again
	// Ensure shader and material are applied here again
	m_pShaderManager->use();
	SetShaderMaterial("Foliage");
	SetShaderTexture("Leaves1");
	m_pShaderManager->setBoolValue("bUseLighting", true);
	m_pShaderManager->setBoolValue("bUseTexture", true);

	m_basicMeshes->DrawSphereMesh();
}

void SceneManager::DrawRectangularHedge(glm::vec3 centerPos, float length, float width, float height)
{
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;
	float halfHeight = height * 0.5f;  // centre mesh vertically at half its height
	float wallThickness = 1.0f;		   // consistent thickness of each hedge wall

	// Left side (aligned along Z)
	DrawHedgeWall(
		glm::vec3(centerPos.x - (length - wallThickness) * 0.5f, halfHeight, centerPos.z),
		glm::vec3(wallThickness, height, width - wallThickness),
		"Leaves2",
		(width - wallThickness) * 0.5f,
		height * 0.5f
	);

	// Right side (opposite side along Z)
	DrawHedgeWall(
		glm::vec3(centerPos.x + (length - wallThickness) * 0.5f, halfHeight, centerPos.z),
		glm::vec3(wallThickness, height, width - wallThickness),
		"Leaves2",
		(width - wallThickness) * 0.5f,
		height * 0.5f
	);

	// Front side (aligned along X)
	DrawHedgeWall(
		glm::vec3(centerPos.x, halfHeight, centerPos.z - (width - wallThickness) * 0.5f),
		glm::vec3(length, height, wallThickness),
		"Leaves2",
		length * 0.5f,
		height * 0.5f
	);

	// Back side (opposite side along X)
	DrawHedgeWall(
		glm::vec3(centerPos.x, halfHeight, centerPos.z + (width - wallThickness) * 0.5f),
		glm::vec3(length, height, wallThickness),
		"Leaves2",
		length * 0.5f,
		height * 0.5f
	);
}

void SceneManager::DrawHedgeWall(glm::vec3 centerPos, glm::vec3 scaleXYZ, const char* textureName, float uvX, float uvY)
{
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, centerPos);
	SetShaderTexture(textureName);
	m_pShaderManager->setVec2Value("UVscale", uvX, uvY);
	m_basicMeshes->DrawBoxMesh();
}
