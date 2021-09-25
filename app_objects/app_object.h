#pragma once

#include "../aveng_model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace aveng {

	// All kinds of matrix
	struct TransformComponent 
	{
		glm::vec3 translation{};	// Position offset
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corresponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 _mat4();

		glm::mat3 normalMatrix();

	};

	class AvengAppObject 
	{

		// For Physics
		struct RigidBody2dComponent 
		{
			glm::vec2 velocity;
			float mass{1.0f};
		};

	public:
		using id_t = unsigned int;

		static AvengAppObject createAppObject() 
		{
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