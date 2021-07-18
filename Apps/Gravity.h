#pragma once

#include <memory>
#include <vector>
#include "../app_objects/app_object.h"
#include "../aveng_window.h"
#include "../EngineDevice.h"
#include "../Renderer/Renderer.h"

namespace aveng {

	class Gravity {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Gravity();
		~Gravity();

		Gravity(const Gravity&) = delete;
		Gravity& operator=(const Gravity&) = delete;

		void run();

	private:
		void loadAppObjects();

		// The window API - Stack allocated
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Gravity 0" };

		EngineDevice engineDevice{ aveng_window };
		Renderer renderer{ aveng_window, engineDevice };
		std::vector<AvengAppObject> appObjects;

	};

}
