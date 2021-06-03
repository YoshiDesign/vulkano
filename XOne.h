#pragma once

#include "aveng_window.h"
#include "GFXPipeline.h"

namespace aveng {

	class XOne {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		
		void run();

	private:
		// Not a dynamic allocation
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Vulkan 0" };
		GFXPipeline gfxPipeline{"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv" };
	};

}