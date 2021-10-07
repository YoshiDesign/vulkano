#pragma once

#include "../app_object.h"
#include "aveng_window.h"
#include "../Action.h"
#include "../data.h"


namespace aveng {
	class KeyboardController {

	public:
        struct KeyMappings {
            int w = GLFW_KEY_W;
            int a = GLFW_KEY_A;
            int s = GLFW_KEY_S;
            int d = GLFW_KEY_D;
            int e = GLFW_KEY_E;
            int q = GLFW_KEY_Q;
            int left = GLFW_KEY_LEFT;
            int lookUp = GLFW_KEY_UP;
            int right = GLFW_KEY_RIGHT;
            int lookDown = GLFW_KEY_DOWN;
        };
        KeyboardController() {};
        void moveInPlaneXZ(GLFWwindow* window, float dt, AvengAppObject& appObject);
        void updatePlayer(GLFWwindow* window, AvengAppObject& appObject, float dt);

        KeyMappings keys{};
        float moveSpeed{ 10.f };
        float lookSpeed{ 2.0f };
        float rollSpeed{ 3.0f };

    private:

	};
}