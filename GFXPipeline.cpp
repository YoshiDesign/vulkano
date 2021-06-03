#include "GFXPipeline.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace aveng {

	// Ctor
	GFXPipeline::GFXPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
	{
		createGFXPipeline(vertFilepath, fragFilepath);
	}

	// Read our shader binary into a vector
	std::vector<char> GFXPipeline::readFile(const std::string& filepath)
	{ 
		// Seek to end of `filepath` by moving cursor via bit-flag std::ios::ate (similar to calling fseek), and read in binary
		// We're seeking the end immediately to get the size.
		// Reading as binary avoids any unwanted text transformation.
		std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

		// Check for success
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file: " + filepath);
		}

		// Get the size
		size_t fileSize = static_cast<size_t>(file.tellg());

		// Initialize a vector to the size of the binary
		std::vector<char> buffer(fileSize);

		// Read our shader program binary into the vector's array
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}


	/**
	* Read our vertex and fragment code
	*/
	void GFXPipeline::createGFXPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
	{
		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilepath);

		// Debug vectors
		std::cout << "Vertex Shader: " << vertCode.size() << std::endl;
		std::cout << "FragmentShader: " << fragCode.size() << std::endl;
	}


}