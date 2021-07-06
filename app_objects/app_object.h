#pragma once

#include "../aveng_model.h"
#include <memory>

namespace aveng {

	// Move things up down left & right
	struct Transform2dComponent {
		glm::vec2 translation{};	// Position offset
		glm::vec2 scale{ 1.f, 1.f };
		float rotation;

		glm::mat2 mat2() {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c, s}, {s, c} };

			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} }; // These args are i-hat and j-hat
			return rotMatrix * scaleMat;
		}
	};

	class AvengAppObject {

	public:
		using id_t = unsigned int;

		static AvengAppObject createAppObject() {
			static id_t currentId = 0;
			return AvengAppObject{ currentId++ };
		}

		AvengAppObject(const AvengAppObject&) = delete;
		AvengAppObject &operator=(const AvengAppObject&) = delete;
		AvengAppObject(AvengAppObject&&) = default;
		AvengAppObject& operator=(AvengAppObject&&) = default;

		const id_t getId() { return id; }

		std::shared_ptr<AvengModel> model{};
		glm::vec3 color{};
		Transform2dComponent transform2d;


	private:

		AvengAppObject(id_t objId) : id{ objId } {}
		id_t id;

	};
}