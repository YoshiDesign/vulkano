#pragma once
#include "../app_objects/app_object.h"
#include "../aveng_window.h"
#include <iostream>

namespace aveng {

	class TestController {
    public:

        struct KeyMappings {
            //int moveLeft = GLFW_KEY_A;
            //int moveRight = GLFW_KEY_D;
            //int moveForward = GLFW_KEY_W;
            //int moveBackward = GLFW_KEY_S;
            //int moveUp = GLFW_KEY_E;
            //int moveDown = GLFW_KEY_Q;
            //int lookLeft = GLFW_KEY_LEFT;
            //int lookRight = GLFW_KEY_RIGHT;
            //int lookUp = GLFW_KEY_UP;
            //int lookDown = GLFW_KEY_DOWN;
            int spacebar = GLFW_KEY_SPACE;
        };


    private:
        KeyMappings keys{};

	};

}
