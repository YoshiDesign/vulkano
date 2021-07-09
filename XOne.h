#pragma once

#include <memory>
#include <vector>
#include "app_objects/app_object.h"
#include "aveng_window.h"
#include "EngineDevice.h"
#include "Renderer/Renderer.h"

namespace aveng {



	class XOne {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		XOne();
		~XOne();

		XOne(const XOne&) = delete;
		XOne &operator=(const XOne&) = delete;
		
		void run();

	private:
		void loadAppObjects();

		// The window API - Stack allocated
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Vulkan 0" };

		EngineDevice engineDevice{ aveng_window };
		Renderer renderer{ aveng_window, engineDevice };
		std::vector<AvengAppObject> appObjects;

	};

}


/* Note */
// The Graphics API - Previously stack allocated
//GFXPipeline gfxPipeline{
//	engineDevice, 
//	"shaders/simple_shader.vert.spv", 
//	"shaders/simple_shader.frag.spv", 
//	GFXPipeline::defaultPipelineConfig(WIDTH, HEIGHT) 
//};