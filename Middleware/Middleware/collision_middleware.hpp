#pragma once

#include "Collision/collision.hpp"
#include "Scene/scene.hpp"

struct CollisionEntry
{
	SceneNodeToken node;
	BoxColliderHandle box_collider;
	bool force_update = false;
};

struct CollisionMiddleware
{
	CollisionHandle collision;
	com::Vector<CollisionEntry> collision_entries;

	inline void allocate(const CollisionHandle& p_collision)
	{
		this->collision = p_collision;
	};

	inline void free()
	{
		this->collision_entries.free_checked();
	};

	inline void before_collision(Scene* p_scene)
	{
		for (size_t i = 0; i < this->collision_entries.Size; i++)
		{
			CollisionEntry& l_collision_entry = this->collision_entries[i];
			NTreeResolve<SceneNode> l_scenenode = SceneKernel::resolve_node(p_scene, l_collision_entry.node);
			if (l_collision_entry.force_update || l_scenenode.element->state.haschanged_thisframe)
			{
				l_collision_entry.box_collider.on_collider_moved(this->collision, Math::Transform(
					SceneKernel::get_worldposition(l_scenenode.element, p_scene),
					SceneKernel::get_worldrotation(l_scenenode.element, p_scene),
					SceneKernel::get_worldscalefactor(l_scenenode.element, p_scene)
				));
			}
		}
	};
};