#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>

#include <common/Texture.h>
#include <common/Shader.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <SOIL.h>


// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no
// public constructor is defined.
class ResourceManager
{
public:
	// Resource storage
	static std::map<std::string, Shader>    Shaders;
	static std::map<std::string, Texture2D> Textures;

	// Loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
	static Shader LoadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name){
		Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
		return Shaders[name];
	}
	// Retrieves a stored sader
	static Shader GetShader(std::string name){
		return Shaders[name];
	}
	// Loads (and generates) a texture from file
	static Texture2D LoadTexture(const GLchar *file, GLboolean alpha, std::string name){
		Textures[name] = loadTextureFromFile(file, alpha);
		return Textures[name];
	}
	// Retrieves a stored texture
	static Texture2D GetTexture(std::string name){
		return Textures[name];
	}

	static bool IsPresent(std::string name){
		if (Textures.count(name))
			return true;
		return false;
	}

	// Properly de-allocates all loaded resources
	static void Clear(){
		// (Properly) delete all shaders
		for (auto iter : Shaders)
			glDeleteProgram(iter.second.ID);
		// (Properly) delete all textures
		for (auto iter : Textures)
			glDeleteTextures(1, &iter.second.ID);
	}
private:
	// Private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
	ResourceManager() { }
	// Loads and generates a shader from file
	static Shader loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		try
		{
			// Open files
			std::ifstream vertexShaderFile(vShaderFile);
			std::ifstream fragmentShaderFile(fShaderFile);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's buffer contents into streams
			vShaderStream << vertexShaderFile.rdbuf();
			fShaderStream << fragmentShaderFile.rdbuf();
			// close file handlers
			vertexShaderFile.close();
			fragmentShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			// If geometry shader path is present, also load a geometry shader
			if (gShaderFile != nullptr)
			{
				std::ifstream geometryShaderFile(gShaderFile);
				std::stringstream gShaderStream;
				gShaderStream << geometryShaderFile.rdbuf();
				geometryShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::exception e)
		{
			std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
		}
		const GLchar *vShaderCode = vertexCode.c_str();
		const GLchar *fShaderCode = fragmentCode.c_str();
		const GLchar *gShaderCode = geometryCode.c_str();
		// 2. Now create shader object from source code
		Shader shader;
		shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
		return shader;
	}
	// Loads a single texture from file
	static Texture2D loadTextureFromFile(const GLchar *file, GLboolean alpha)
	{
		// Create Texture object
		Texture2D texture;
		if (alpha)
		{
			texture.Internal_Format = GL_RGBA;
			texture.Image_Format = GL_RGBA;
		}
		// Load image
		int width, height;
		unsigned char* image = SOIL_load_image(file, &width, &height, 0, texture.Image_Format == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
		// Now generate texture
		texture.Generate(width, height, image);
		// And finally free image data
		SOIL_free_image_data(image);
		return texture;
	}
};

#endif
