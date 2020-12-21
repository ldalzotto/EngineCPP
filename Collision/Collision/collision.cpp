
#include "Collision/collision.hpp"
#include "Common/Container/pool.hpp"
#include "Common/Container/vector.hpp"
#include "Math/geometry.hpp"

using namespace Math;

struct BoxCollider
{
	Math::Transform transform;
	AABB<float> local_box;
};

struct CollisionHeap
{
	struct CollisionDetection
	{
		com::Pool<com::Vector<com::TPoolToken<BoxCollider>>> collision_events;
		com::Vector<com::TPoolToken<BoxCollider>> involved_colliders_lastframe;

		inline void free()
		{
			this->collision_events.free();
			this->involved_colliders_lastframe.free();
		};

		inline void clear_lastframe()
		{
			for (size_t i = 0; i < this->involved_colliders_lastframe.Size; i++)
			{
				this->collision_events[com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>(this->involved_colliders_lastframe[i].val)].clear();
			}
			this->involved_colliders_lastframe.clear();
		};

		inline void push_collision_event(const com::TPoolToken<BoxCollider>& p_left, const com::TPoolToken<BoxCollider>& p_right)
		{
			this->collision_events[com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>(p_left.val)].push_back(p_right);
			this->collision_events[com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>(p_right.val)].push_back(p_left);
			this->involved_colliders_lastframe.push_back(p_left);
			this->involved_colliders_lastframe.push_back(p_right);
		};

		inline com::Vector<com::TPoolToken<BoxCollider>>& get_collision_events(const com::TPoolToken<BoxCollider>& p_collider)
		{
			return this->collision_events[com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>(p_collider.val)];
		};

	} collision_detection;

	com::Pool<BoxCollider> box_colliders;

	com::Vector<com::TPoolToken<BoxCollider>> box_colliders_indices;

	inline void allocate()
	{
		this->box_colliders.allocate(0);
		this->box_colliders_indices.allocate(0);
	};

	inline void free()
	{
		this->box_colliders.free_checked();
		this->box_colliders_indices.free_checked();
		this->collision_detection.free();
	};

	inline com::TPoolToken<BoxCollider> allocate_boxcollider(const AABB<float>& p_local_aabb)
	{
		BoxCollider l_box_collider = BoxCollider();
		l_box_collider.local_box = p_local_aabb;
		com::TPoolToken<BoxCollider> l_box_collider_index = this->box_colliders.alloc_element(l_box_collider);
		this->box_colliders_indices.push_back(l_box_collider_index);
		this->collision_detection.collision_events.alloc_element(com::Vector<com::TPoolToken<BoxCollider>>());
		return l_box_collider_index;
	};

	inline void free_boxcollider(const com::TPoolToken<BoxCollider>& p_boxcollider)
	{
		for (size_t i = 0; i < this->box_colliders_indices.Size; i++)
		{
			if (this->box_colliders_indices[i].val == p_boxcollider.val)
			{
				this->box_colliders_indices.erase_at(i, 1);
				break;
			}
		}
		this->box_colliders.release_element(p_boxcollider);
		this->collision_detection.collision_events.release_element(com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>(p_boxcollider.val));
	};

	inline void push_transform(const com::TPoolToken<BoxCollider>& p_boxcollider, const Math::Transform& p_transform)
	{
		this->box_colliders[p_boxcollider].transform = p_transform;
	};

};


struct CollisionDetectionStep
{
	CollisionHeap* heap;
	com::Vector<com::TPoolToken<BoxCollider>> colliders_tobe_process;
	
	inline void allocate(CollisionHeap* p_heap)
	{
		this->heap = p_heap;
	};

	inline void free()
	{
		this->colliders_tobe_process.free();
	};

	inline void step()
	{
		//TODO 1/ Clear last frame results
		this->heap->collision_detection.clear_lastframe();

		for (size_t i = 0; i < this->colliders_tobe_process.Size; i++)
		{
			com::TPoolToken<BoxCollider>& l_left_collider_token = this->colliders_tobe_process[i];

			BoxCollider& l_left_collider = this->heap->box_colliders[l_left_collider_token];

			AABB<float> l_left_projected = Geometry::project(l_left_collider.local_box, l_left_collider.transform);

			for (size_t j = 0; j < this->heap->box_colliders_indices.Size; j++)
			{
				com::TPoolToken<BoxCollider>& l_right_collider_token = this->heap->box_colliders_indices[j];
				if (l_left_collider_token.val != l_right_collider_token.val)
				{
					BoxCollider& l_right_collider = this->heap->box_colliders[l_right_collider_token];

					// mat4f l_right_to_left_TRS = mul(l_left_collider_TRS_inv, l_right_collider.trs);
					// AABB<float> l_right_projected = Geometry::project(l_right_collider.local_box, l_right_to_left_TRS);

					AABB<float> l_right_projected = Geometry::project(l_right_collider.local_box, l_right_collider.transform);

					//TODO using OBB
					if (Geometry::overlap(l_left_projected, l_right_projected))
					{
						this->heap->collision_detection.push_collision_event(l_left_collider_token, l_right_collider_token);
					}
				}
			}

		}

		this->colliders_tobe_process.clear();
	}

	inline void on_collider_moved(com::TPoolToken<BoxCollider>& p_moved_collider)
	{
		this->colliders_tobe_process.push_back(p_moved_collider);
	};
};

struct Collision
{
	CollisionHeap collision_heap;
	CollisionDetectionStep collision_detection_step;

	inline void allocate()
	{
		this->collision_heap.allocate();
		
		this->collision_detection_step.allocate(&this->collision_heap);
	};

	inline void free()
	{
		this->collision_detection_step.free();
		this->collision_heap.free();
	};

	inline void update()
	{
		this->collision_detection_step.step();
	};

	inline void on_collider_moved(com::TPoolToken<BoxCollider>& p_moved_collider, const Transform& p_world_transform)
	{
		this->collision_heap.push_transform(p_moved_collider, p_world_transform);
		this->collision_detection_step.on_collider_moved(p_moved_collider);
	};
};


#include "collision_objects.hpp"

