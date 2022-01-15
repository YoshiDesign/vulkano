#pragma once

#include "aveng_model.h"
#include "AvengComponent.h"

#include <memory>
#include <unordered_map>

namespace aveng {

	class AvengAppObject 
	{

	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, AvengAppObject>;

		static AvengAppObject createAppObject(int texture_id)
		{
			static id_t currentId = 0;
			return AvengAppObject{ currentId++, texture_id };
		}

		AvengAppObject(id_t objId, int texture_id) : id{ objId }, texture_id{ texture_id } {}

		AvengAppObject(const AvengAppObject&) = delete;
		AvengAppObject& operator=(const AvengAppObject&) = delete;
		AvengAppObject(AvengAppObject&&) = default;
		AvengAppObject& operator=(AvengAppObject&&) = default;

		const id_t getId() { return id; }
		std::unique_ptr<AvengModel> model{};

		int get_texture() { return texture_id; }
		void set_texture(int texture) { texture_id = texture; }

		glm::vec3 color{};
		glm::vec3 getPosition();
		glm::vec3 getRotation();

		MetaComponent meta{};
		TransformComponent transform{};
		VisualComponent visual{};
		MotionComponent motion{};

	private:

		int texture_id;
		id_t id;
		
	};
}