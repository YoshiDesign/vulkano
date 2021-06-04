#pragma once

#include "aveng_window.h"
#include "GFXPipeline.h"
#include "EngineDevice.hpp"


namespace aveng {

	class XOne {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		
		void run();

	private:
		// The window API - Stack allocated
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Vulkan 0" };

		EngineDevice engineDevice{ aveng_window };

		// The Graphics API - Stack allocated
		GFXPipeline gfxPipeline{
			engineDevice, 
			"shaders/simple_shader.vert.spv", 
			"shaders/simple_shader.frag.spv", 
			GFXPipeline::defaultPipelineConfig(WIDTH, HEIGHT) 
		};
	};

}