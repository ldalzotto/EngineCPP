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

	com::Vector<BoxColliderHandle> removed_collider_detectors;
	com::Vector<BoxColliderHandle> removed_collider_detectors_lastframe;

	inline void allocate(const CollisionHandle& p_collision)
	{
		this->collision = p_collision;
	};

	inline void free()
	{
		this->collision_entries.free_checked();
		this->removed_collider_detectors.free();
		this->removed_collider_detectors_lastframe.free();
	};

	inline void before_collision(Scene* p_scene)
	{
		for (size_t i = 0; i < this->collision_entries.Size; i++)
		{
			CollisionEntry& l_collision_entry = this->collision_entries[i];
			NTreeResolve<SceneNode> l_scenenode = SceneKernel::resolve_node(p_scene, l_collision_entry.node);
			if (l_collision_entry.force_update || l_scenenode.element->state.haschanged_thisframe)
			{
				l_collision_entry.box_collider.on_collider_moved(this->collision,
					transform_pa{
						*(v3f*)&SceneKernel::get_worldposition(l_scenenode.element, p_scene),
						(*(quat*)&SceneKernel::get_worldrotation(l_scenenode.element, p_scene)).to_axis()
					}
				);
				l_collision_entry.force_update = false;
			}
		}
	};

	inline void push_collider(SceneNodeToken p_node, const BoxCollider& p_box_collider )
	{
		BoxColliderHandle l_box_collider;
		l_box_collider.allocate(this->collision, *(aabb*)&p_box_collider.local_box);
		CollisionEntry l_entry;
		l_entry.box_collider = l_box_collider;
		l_entry.force_update = true;
		l_entry.node = p_node;
		this->collision_entries.push_back(l_entry);
	};

	inline void remove_collider(SceneNodeToken p_node)
	{
		for (size_t i = 0; i < this->collision_entries.Size; i++)
		{
			if (this->collision_entries[i].node.val == p_node.val)
			{
				this->collision_entries[i].box_collider.free(this->collision);
				this->collision_entries.erase_at(i, 1);
				break;
			}
		}
	};

	inline ColliderDetectorHandle attach_collider_detector(SceneNodeToken p_node)
	{
		BoxColliderHandle l_box_collider = this->get_collider(p_node);
		ColliderDetectorHandle l_detector;
		l_detector.allocate(this->collision, l_box_collider);
		return l_detector;
	};

	inline void remove_collider_detector(ColliderDetectorHandle& p_collider_detector)
	{
		p_collider_detector.free(this->collision);
	};

	inline BoxColliderHandle get_collider(SceneNodeToken p_node)
	{
		for (size_t i = 0; i < this->collision_entries.Size; i++)
		{
			if (this->collision_entries[i].node.val == p_node.val)
			{
				return this->collision_entries[i].box_collider;
			}
		}
		
		abort();
	};

};

typedef CollisionMiddleware* CollisionMiddlewareHandle;