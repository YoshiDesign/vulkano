#pragma once

#include "../aveng_model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace aveng {

	// Move things up down left & right
	struct TransformComponent {
		glm::vec3 translation{};	// Position offset
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		glm::mat4 mat4() {
		
			// Creates a 4x4 translation matrix using the current translation values
			auto transform = glm::translate(glm::mat4{ 1.f }, translation);

			/*
				Matrix corresponds to translate * Ry * Rx * Rz * scale transformation
				Rotation conventions uses tai-bryan angles with axis order Y(1), X(2), Z(3)
			*/
			transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f });
			transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
			transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });

			// transform x scale (matrix mult)
			transform = glm::scale(transform, scale);
			return transform;

		}
	};

	class AvengAppObject {


		// For Gravity App
		struct RigidBody2dComponent {
			glm::vec2 velocity;
			float mass{1.0f};
		};

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
		TransformComponent transform{};
		RigidBody2dComponent rigidBody2d;

	private:

		AvengAppObject(id_t objId) : id{ objId } {}
		id_t id;

	};
}