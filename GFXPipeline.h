#pragma once
#include <string>
#include <vector>

/**
* @class GFXPipeline
* Used to implement our precompiled shaders
*/
namespace aveng {


	class GFXPipeline {

	public:

		GFXPipeline(const std::string& vertFilepath, const std::string& fragFilepath);

	private:
		static std::vector<char> readFile(const std::string& filepath);
		void createGFXPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
	};


} // NS