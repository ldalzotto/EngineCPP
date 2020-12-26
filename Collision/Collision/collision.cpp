
#include "Collision/collision.hpp"
#include "Common/Container/pool.hpp"
#include "Common/Container/vector.hpp"
#include "Math/geometry.hpp"

using namespace Math;

struct BoxCollider
{
	Math::Transform transform;
	Math::Matrix<3, float> rotation_axis;
	AABB<float> local_box;
};

struct ColliderDetector
{
	com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>> collision_events;

	inline ColliderDetector(const com::TPoolToken<com::Vector<com::TPoolToken<BoxCollider>>>& p_collision_events)
	{
		this->collision_events = p_collision_events;
	};
};

struct CollisionHeap
{
	struct BoxCollider_Heap : public com::Pool<BoxCollider> { using com::Pool<BoxCollider>::Pool; } box_colliders;

	com::Vector<com::TPoolToken<BoxCollider>> box_colliders_indices;

	struct BoxCollider_to_ColliderDetector : public com::Pool<com::TPoolToken<ColliderDetector>>
	{
		using com::Pool<com::TPoolToken<ColliderDetector>>::Pool;

		inline com::TPoolToken<ColliderDetector>& operator[](const com::TPoolToken<BoxCollider> p_index) {
			return com::Pool<com::TPoolToken<ColliderDetector>>::Pool::operator[](p_index.val);
		};

		inline void release_element(const com::TPoolToken<BoxCollider> p_element)
		{
			com::Pool<com::TPoolToken<ColliderDetector>>::Pool::release_element(com::TPoolToken<com::TPoolToken<ColliderDetector>>(p_element.val));
		};

	} box_colliders_to_collider_detector;

	com::Pool<ColliderDetector> collider_detectors;
	com::Pool<com::Vector<com::TPoolToken<BoxCollider>>> collider_detectors_events;

	inline void allocate()
	{
		this->box_colliders.allocate(0);
		this->box_colliders_indices.allocate(0);
		this->box_colliders_to_collider_detector.allocate(0);
		this->collider_detectors.allocate(0);
		this->collider_detectors_events.allocate(0);
	};

	inline void free()
	{
		this->box_colliders.free_checked();
		this->box_colliders_indices.free_checked();
		this->collider_detectors.free_checked();
		this->collider_detectors_events.free_checked();
	};

	inline com::TPoolToken<BoxCollider> allocate_boxcollider(const AABB<float>& p_local_aabb)
	{
		BoxCollider l_box_collider = BoxCollider();
		l_box_collider.local_box = p_local_aabb;
		com::TPoolToken<BoxCollider> l_box_collider_index = this->box_colliders.alloc_element(l_box_collider);
		this->box_colliders_indices.push_back(l_box_collider_index);
		this->box_colliders_to_collider_detector.alloc_element(com::TPoolToken<ColliderDetector>());
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
		
		com::TPoolToken<ColliderDetector>& l_collider_detector = this->box_colliders_to_collider_detector[p_boxcollider];
		if (l_collider_detector.val != -1)
		{
			this->free_colliderdetector(p_boxcollider, l_collider_detector);
		}
		
		this->box_colliders.release_element(p_boxcollider);
	};

	inline com::TPoolToken<ColliderDetector> allocate_colliderdetector(const com::TPoolToken<BoxCollider>& p_collider)
	{
		return this->box_colliders_to_collider_detector[p_collider] =
			this->collider_detectors.alloc_element(ColliderDetector(this->collider_detectors_events.alloc_element(com::Vector<com::TPoolToken<BoxCollider>>())));
	};

	inline void free_colliderdetector(const com::TPoolToken<BoxCollider>& p_collider, const com::TPoolToken<ColliderDetector>& p_collider_detector)
	{
		com::TPoolToken<ColliderDetector> l_removed_collider_detector = p_collider_detector;

		this->box_colliders_to_collider_detector[p_collider].reset();
		this->box_colliders_to_collider_detector.release_element(p_collider);

		ColliderDetector& l_collider_detector = this->collider_detectors[l_removed_collider_detector];
		this->collider_detectors_events[l_collider_detector.collision_events].free();
		this->collider_detectors_events.release_element(l_collider_detector.collision_events);
		this->collider_detectors.release_element(l_removed_collider_detector);
	};

	inline void push_transform(const com::TPoolToken<BoxCollider>& p_boxcollider, const Math::Transform& p_world_transform, const Math::quat& p_local_rotation)
	{
		BoxCollider& l_boxcollider = this->box_colliders[p_boxcollider];
		l_boxcollider.transform = p_world_transform;
		l_boxcollider.rotation_axis = Math::extractAxis<float>(p_world_transform.rotation);
	};

	

};


struct CollisionDetectionStep
{
	CollisionHeap* heap;
	com::Vector<com::TPoolToken<BoxCollider>> colliders_tobe_process;
	com::Vector<com::TPoolToken<ColliderDetector>> lastframe_involved_colliderdetectors;

	inline void allocate(CollisionHeap* p_heap)
	{
		this->heap = p_heap;
	};

	inline void free()
	{
		this->colliders_tobe_process.free();
		this->lastframe_involved_colliderdetectors.free();
	};

	inline void step()
	{
		this->clear_lastframe_collision_events();

		for (size_t i = 0; i < this->colliders_tobe_process.Size; i++)
		{
			com::TPoolToken<BoxCollider>& l_left_collider_token = this->colliders_tobe_process[i];

			//If there is a detector attached to the collider
			if (this->heap->box_colliders_to_collider_detector[l_left_collider_token].val != -1)
			{
				BoxCollider& l_left_collider = this->heap->box_colliders[l_left_collider_token];

				OBB<float>l_left_projected = Geometry::to_obb(l_left_collider.local_box, l_left_collider.transform, l_left_collider.rotation_axis);

				//TODO -> we compare with the whole world. using spatial partitioning in the future
				for (size_t j = 0; j < this->heap->box_colliders_indices.Size; j++)
				{
					com::TPoolToken<BoxCollider>& l_right_collider_token = this->heap->box_colliders_indices[j];

					//Avoid self test
					if (l_left_collider_token.val != l_right_collider_token.val)
					{
						BoxCollider& l_right_collider = this->heap->box_colliders[l_right_collider_token];

						OBB<float>l_right_projected = Geometry::to_obb(l_right_collider.local_box, l_right_collider.transform, l_right_collider.rotation_axis);

						if (Geometry::overlap3(l_left_projected, l_right_projected))
						{
							this->on_collision_detected(l_left_collider_token, l_right_collider_token);
						}
					}
				}
			}
		}

		this->colliders_tobe_process.clear();

	}

	inline void clear_lastframe_collision_events()
	{
		for (size_t i = 0; i < this->lastframe_involved_colliderdetectors.Size; i++)
		{
			this->heap->collider_detectors_events[this->heap->collider_detectors[this->lastframe_involved_colliderdetectors[i]].collision_events].clear();
		}
		this->lastframe_involved_colliderdetectors.clear();
	};

	inline void on_collider_moved(com::TPoolToken<BoxCollider>& p_moved_collider)
	{
		this->colliders_tobe_process.push_back(p_moved_collider);
	};

private:

	//TODO -> collision events doesn't care of previous state. They need to if we want to recreate some events like "trigger enter" or "trigger exit"
	inline void on_collision_detected(const com::TPoolToken<BoxCollider>& p_left, const com::TPoolToken<BoxCollider>& p_right)
	{
		push_colliderdetector_event(p_left);
		push_colliderdetector_event(p_right);
	};

	inline void push_colliderdetector_event(const com::TPoolToken<BoxCollider>& p_intersected_collider)
	{
		com::TPoolToken<ColliderDetector>& l_left_collider_detector = this->heap->box_colliders_to_collider_detector[p_intersected_collider];
		if (l_left_collider_detector.val != -1)
		{
			this->lastframe_involved_colliderdetectors.push_back(l_left_collider_detector);
			this->heap->collider_detectors_events[this->heap->collider_detectors[l_left_collider_detector].collision_events].push_back(p_intersected_collider);
		}
	}
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

	inline void on_collider_moved(com::TPoolToken<BoxCollider>& p_moved_collider, const Transform& p_world_transform, const Math::quat& p_local_rotation)
	{
		this->collision_heap.push_transform(p_moved_collider, p_world_transform, p_local_rotation);
		this->collision_detection_step.on_collider_moved(p_moved_collider);
	};
};


#include "collision_objects.hpp"

