#pragma once

#include "../aveng_model.h"
#include "AvengComponent.h"

#include <memory>
#include <unordered_map>

namespace aveng {

	class AvengAppObject 
	{

		enum textures {
			THEME_1 = 0,
			THEME_2,
			THEME_3,
			THEME_4
		};

		// For Physics
		struct RigidBody2dComponent 
		{
			glm::vec2 velocity;
			float mass{1.0f};
		};

	public:
		using id_t = unsigned int;

		static AvengAppObject createAppObject(int texture_id)
		{
			static id_t currentId = 0;
			return AvengAppObject{ currentId++, texture_id };
		}

		AvengAppObject(id_t objId, int texture_id) : id{ objId }, texture_id{ texture_id } {}

		AvengAppObject(const AvengAppObject&) = delete;
		AvengAppObject &operator=(const AvengAppObject&) = delete;
		AvengAppObject(AvengAppObject&&) = default;
		AvengAppObject& operator=(AvengAppObject&&) = default;

		const id_t getId() { return id; }
		std::shared_ptr<AvengModel> model{};

		int get_texture() { return texture_id; }
		void set_texture(int texture) { texture_id = texture; }

		glm::vec3 color{};
		glm::vec3 getPosition();
		glm::vec3 getRotation();

		TransformComponent transform{};
	private:

		int texture_id;
		id_t id;
		
	};
}