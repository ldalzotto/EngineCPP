
#include "Collision/collision.hpp"
#include "Common/Container/pool_v2.hpp"
#include "Common/Container/vector.hpp"
#include "Math/geometry.hpp"

using namespace Math;

struct BoxCollider
{
	Math::Transform transform;
	Math::Matrix<3, float> rotation_axis;
	AABB<float> local_box;
};

struct TriggerState
{
	TToken<BoxCollider> other;
	Trigger::State state = Trigger::State::UNDEFINED;
};

struct ColliderDetector
{
	TToken<com::Vector<TriggerState>> collision_events;

	inline ColliderDetector(const TToken<com::Vector<TriggerState>>* p_collision_events)
	{
		this->collision_events = *p_collision_events;
	};
};

typedef Pool_v2<TToken<ColliderDetector>> ColliderToDetector;


inline TToken<ColliderDetector>* pool_get(ColliderToDetector* p_collider_to_detector, TToken<BoxCollider>* p_token)
{
	return pool_get(p_collider_to_detector, TToken_cast<BoxCollider, TToken<ColliderDetector>>(p_token));
};

inline void pool_release_element(ColliderToDetector* p_collider_to_detector, TToken<BoxCollider>* p_token)
{
	pool_release_element(p_collider_to_detector, TToken_cast<BoxCollider, TToken<ColliderDetector>>(p_token));
};

inline bool pool_is_token_free(ColliderToDetector* p_collider_to_detector, TToken<BoxCollider>* p_token)
{
	return pool_is_token_free(p_collider_to_detector, TToken_cast<BoxCollider, TToken<ColliderDetector>>(p_token));
};

//TODO -> having more than one ColliderDetector attached to a Collider.
struct CollisionHeap
{
	Pool_v2<BoxCollider> box_colliders;

	com::Vector<TToken<BoxCollider>> box_colliders_indices;
	ColliderToDetector box_colliders_to_collider_detector;

	Pool_v2<ColliderDetector> collider_detectors;
	com::Vector<TToken<ColliderDetector>> collider_detectors_indices;
	Pool_v2<com::Vector<TriggerState>> collider_detectors_events;

	inline void allocate()
	{
		pool_allocate(&this->box_colliders);
		this->box_colliders_indices.allocate(0);
		pool_allocate(&this->box_colliders_to_collider_detector);
		pool_allocate(&this->collider_detectors);
		this->collider_detectors_indices.allocate(0);
		pool_allocate(&this->collider_detectors_events);
	};

	inline void free()
	{
		pool_free_checked(&this->box_colliders);
		this->box_colliders_indices.free_checked();
		pool_free_checked(&this->box_colliders_to_collider_detector);
		pool_free_checked(&this->collider_detectors);
		pool_free_checked(&this->collider_detectors_events);
		this->collider_detectors_indices.free_checked();
	};

	inline TToken<BoxCollider> allocate_boxcollider(const AABB<float>& p_local_aabb)
	{
		BoxCollider l_box_collider = BoxCollider();
		l_box_collider.local_box = p_local_aabb;
		TToken<BoxCollider> l_box_collider_index = pool_alloc_element(&this->box_colliders, &l_box_collider);
		this->box_colliders_indices.push_back(l_box_collider_index);
		pool_alloc_element(&this->box_colliders_to_collider_detector, &TToken_build<ColliderDetector>());
		return l_box_collider_index;
	};

	inline void free_boxcollider(TToken<BoxCollider>& p_boxcollider)
	{
		for (size_t i = 0; i < this->box_colliders_indices.Size; i++)
		{
			if (this->box_colliders_indices[i].val == p_boxcollider.val)
			{
				this->box_colliders_indices.erase_at(i, 1);
				break;
			}
		}

		TToken<ColliderDetector>* l_collider_detector = pool_get(&this->box_colliders_to_collider_detector, &p_boxcollider);
		
		if (l_collider_detector->val != -1)
		{
			this->free_colliderdetector(&p_boxcollider, l_collider_detector);
		}

		pool_release_element(&this->box_colliders, &p_boxcollider);
	};

	inline TToken<ColliderDetector> allocate_colliderdetector(TToken<BoxCollider>& p_collider)
	{

#if COLLIDER_BOUND_TEST
		if (!pool_is_token_free(&this->box_colliders_to_collider_detector, &p_collider))
		{
			TToken<ColliderDetector>* l_collider_detector = pool_get(&this->box_colliders_to_collider_detector, &p_collider);
			if (l_collider_detector->val != -1)
			{
				//Cannot attach multiple collider detector to a collider for now
				abort();
			}
		}
#endif

		TToken<ColliderDetector> l_colider_detector = pool_alloc_element(&this->collider_detectors, &ColliderDetector(&pool_alloc_element(&this->collider_detectors_events, &(com::Vector<TriggerState>()))));
		this->collider_detectors_indices.push_back(l_colider_detector);
		*pool_get(&this->box_colliders_to_collider_detector, &p_collider) = l_colider_detector;
		return l_colider_detector;
	};

	inline void free_colliderdetector(TToken<BoxCollider>* p_collider, TToken<ColliderDetector>* p_collider_detector)
	{
		TToken<ColliderDetector>l_removed_collider_detector = *p_collider_detector;

		Token_reset(&pool_get(&this->box_colliders_to_collider_detector, p_collider)->token);
		pool_release_element(&this->box_colliders_to_collider_detector, p_collider);
		
		ColliderDetector* l_collider_detector = pool_get(&this->collider_detectors, &l_removed_collider_detector);
		pool_get(&this->collider_detectors_events, &l_collider_detector->collision_events)->free();
		pool_release_element(&this->collider_detectors_events, &l_collider_detector->collision_events);
		pool_release_element(&this->collider_detectors, &l_removed_collider_detector);

		for (size_t i = 0; i < this->collider_detectors_indices.Size; i++)
		{
			if (this->collider_detectors_indices[i].val == l_removed_collider_detector.val)
			{
				this->collider_detectors_indices.erase_at(i, 1);
				break;
			}
		}
	};

	inline void push_transform(TToken<BoxCollider>* p_boxcollider, const Math::Transform& p_world_transform, const Math::quat& p_local_rotation)
	{
		BoxCollider* l_boxcollider = pool_get(&this->box_colliders, p_boxcollider);
		l_boxcollider->transform = p_world_transform;
		l_boxcollider->rotation_axis = Math::extractAxis<float>(p_world_transform.rotation);
	};

	inline com::Vector<TriggerState>* get_trigger_events(TToken<ColliderDetector>* p_detector)
	{
		return pool_get(
			&this->collider_detectors_events,
			&(pool_get(&this->collider_detectors, p_detector)->collision_events)
		);
	};
};

struct CollisionDetectionStep
{
	CollisionHeap* heap;

	com::Vector<TToken<BoxCollider>> in_colliders_processed;

	struct ColliderDetector_IntersectionEvent
	{
		TToken<ColliderDetector> detector;
		TToken<BoxCollider> source;
		TToken<BoxCollider> other;

		inline ColliderDetector_IntersectionEvent(const TToken<ColliderDetector>& p_detector, const TToken<BoxCollider>& p_source, const TToken<BoxCollider>& p_other)
		{
			this->detector = p_detector;
			this->source = p_source;
			this->other = p_other;
		};
	};

	struct ColliderDetector_TriggerEvent
	{
		TToken<ColliderDetector> detector;
		TToken<BoxCollider> other;

		inline ColliderDetector_TriggerEvent(const TToken<ColliderDetector>& p_detector, const TToken<BoxCollider>& p_other)
		{
			this->detector = p_detector;
			this->other = p_other;
		};
	};

	com::Vector<TToken<BoxCollider>> deleted_colliders_from_last_step;

	com::Vector<ColliderDetector_IntersectionEvent> is_waitingfor_trigger_stay_detector;
	com::Vector<ColliderDetector_IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;

	com::Vector<ColliderDetector_TriggerEvent> is_waitingfor_trigger_none_detector;
	com::Vector<ColliderDetector_TriggerEvent> is_waitingfor_trigger_none_nextframe_detector;

	com::Vector<TToken<ColliderDetector>> lastframe_involved_colliderdetectors;

	inline void allocate(CollisionHeap* p_heap)
	{
		this->heap = p_heap;
	};

	inline void free()
	{
		this->in_colliders_processed.free();
		this->lastframe_involved_colliderdetectors.free();

		this->is_waitingfor_trigger_stay_detector.free();
		this->is_waitingfor_trigger_stay_nextframe_detector.free();

		this->deleted_colliders_from_last_step.free();
	};

	inline void step()
	{
		this->clear_lastframe_collision_events();
		this->swap_triggerstaw_waiting_buffers();
		this->swap_triggernone_waiting_buffers();

		this->clean_deleted_colliders();

		for (size_t i = 0; i < this->in_colliders_processed.Size; i++)
		{
			TToken<BoxCollider>& l_left_collider_token = this->in_colliders_processed[i];

			//If there is a detector attached to the collider
			if (pool_get(&this->heap->box_colliders_to_collider_detector, &l_left_collider_token)->val != -1)
			{
				BoxCollider* l_left_collider = pool_get(&this->heap->box_colliders, &l_left_collider_token);

				OBB<float>l_left_projected = Geometry::to_obb(l_left_collider->local_box, l_left_collider->transform, l_left_collider->rotation_axis);

				//TODO -> we compare with the whole world. using spatial partitioning in the future
				for (size_t j = 0; j < this->heap->box_colliders_indices.Size; j++)
				{
					TToken<BoxCollider>& l_right_collider_token = this->heap->box_colliders_indices[j];

					// Avoid self test
					//TODO -> FIX - we also need to avoid calculatin the same collision between two Colliders (in the case where multiple colliders in this->in_colliders_processed are colliding between each other)
					//		  because the on_collision_detected and on_collision_detection_failed operations are bidirectional.
					//		  This bug presents the TRIGGER_ENTER event to be triggered.
					if (l_left_collider_token.val != l_right_collider_token.val)
					{
						BoxCollider* l_right_collider = pool_get(&this->heap->box_colliders, &l_right_collider_token);

						OBB<float>l_right_projected = Geometry::to_obb(l_right_collider->local_box, l_right_collider->transform, l_right_collider->rotation_axis);

						if (Geometry::overlap3(l_left_projected, l_right_projected))
						{
							this->on_collision_detected(l_left_collider_token, l_right_collider_token);
						}
						else
						{
							this->on_collision_detection_failed(l_left_collider_token, l_right_collider_token);
						}
					}
				}
			}
		}

		for (size_t j = 0; j < this->is_waitingfor_trigger_stay_detector.Size; j++)
		{
			ColliderDetector_IntersectionEvent& l_waiting = this->is_waitingfor_trigger_stay_detector[j];

			com::Vector<TriggerState>* l_events = this->heap->get_trigger_events(&l_waiting.detector);

			for (size_t k = 0; k < l_events->Size; k++)
			{
				TriggerState& l_trigger_event = l_events->operator[](k);
				if (l_trigger_event.other.val == l_waiting.other.val)
				{
					l_trigger_event.state = Trigger::State::TRIGGER_STAY;
				}
			}
		}

		for (size_t i = 0; i < this->is_waitingfor_trigger_none_detector.Size; i++)
		{
			ColliderDetector_TriggerEvent& l_waiting = this->is_waitingfor_trigger_none_detector[i];

			com::Vector<TriggerState>* l_events = this->heap->get_trigger_events(&l_waiting.detector);

			for (size_t k = 0; k < l_events->Size; k++)
			{
				TriggerState& l_trigger_event = l_events->operator[](k);
				if (l_trigger_event.other.val == l_waiting.other.val)
				{
					l_trigger_event.state = Trigger::State::NONE;
				}
			}
		}

		this->is_waitingfor_trigger_stay_detector.clear();
		this->is_waitingfor_trigger_none_detector.clear();
		this->in_colliders_processed.clear();

	}

	inline void clear_lastframe_collision_events()
	{
		this->lastframe_involved_colliderdetectors.clear();
	};

	inline void swap_triggerstaw_waiting_buffers()
	{
		com::Vector<ColliderDetector_IntersectionEvent> l_tmp = this->is_waitingfor_trigger_stay_detector;
		this->is_waitingfor_trigger_stay_detector = this->is_waitingfor_trigger_stay_nextframe_detector;
		this->is_waitingfor_trigger_stay_nextframe_detector = l_tmp;
	};

	inline void swap_triggernone_waiting_buffers()
	{
		com::Vector<ColliderDetector_TriggerEvent> l_tmp = this->is_waitingfor_trigger_none_detector;
		this->is_waitingfor_trigger_none_detector = this->is_waitingfor_trigger_none_nextframe_detector;
		this->is_waitingfor_trigger_none_nextframe_detector = l_tmp;
	};

	inline void on_collider_moved(TToken<BoxCollider>& p_moved_collider)
	{
		this->in_colliders_processed.push_back(p_moved_collider);
	};

	inline void on_collider_deleted(const TToken<BoxCollider> p_collider)
	{
		this->deleted_colliders_from_last_step.push_back(p_collider);
	};

private:

	inline void on_collision_detected(TToken<BoxCollider>& p_left, TToken<BoxCollider>& p_right)
	{
		push_colliderdetector_event(p_left, p_right);
		push_colliderdetector_event(p_right, p_left);
	};

	inline void on_collision_detection_failed(TToken<BoxCollider>& p_left, TToken<BoxCollider>& p_right)
	{
		no_more_collision(p_left, p_right);
		no_more_collision(p_right, p_left);
	};

	inline void push_colliderdetector_event(TToken<BoxCollider>& p_source_collider, const TToken<BoxCollider>& p_intersected_collider)
	{
		TToken<ColliderDetector>* l_source_collider_detector = pool_get(&this->heap->box_colliders_to_collider_detector, &p_source_collider);
		if (l_source_collider_detector->val != -1)
		{
			this->lastframe_involved_colliderdetectors.push_back(*l_source_collider_detector);

			com::Vector<TriggerState>* l_collider_triggerevents = this->heap->get_trigger_events(l_source_collider_detector);

			bool l_trigger_event_found = false;
			for (size_t i = 0; i < l_collider_triggerevents->Size; i++)
			{
				TriggerState& l_collider_trigger_event = l_collider_triggerevents->operator[](i);
				if (l_collider_trigger_event.other.val == p_intersected_collider.val)
				{
					l_collider_trigger_event.state = Trigger::State::TRIGGER_STAY;
					l_trigger_event_found = true;
					break;
				}
			}

			if (!l_trigger_event_found)
			{
				TriggerState l_trigger_event = TriggerState();
				l_trigger_event.other = p_intersected_collider;
				l_trigger_event.state = Trigger::State::TRIGGER_ENTER;
				l_collider_triggerevents->push_back(l_trigger_event);
				this->is_waitingfor_trigger_stay_nextframe_detector.push_back(ColliderDetector_IntersectionEvent(*l_source_collider_detector, p_source_collider, p_intersected_collider));
			}
		}
	}

	inline void no_more_collision(TToken<BoxCollider>& p_source_collider, TToken<BoxCollider>& p_intersected_collider)
	{
		TToken<ColliderDetector>* l_source_collider_detector = pool_get(&this->heap->box_colliders_to_collider_detector, &p_source_collider);
		if (l_source_collider_detector->val != -1)
		{
			com::Vector<TriggerState>* l_source_trigger_events = this->heap->get_trigger_events(l_source_collider_detector);
			for (size_t i = 0; i < l_source_trigger_events->Size; i++)
			{
				TriggerState& l_trigger_event = l_source_trigger_events->operator[](i);
				if (l_trigger_event.other.val == p_intersected_collider.val)
				{
					l_trigger_event.state = Trigger::State::TRIGGER_EXIT;
					this->is_waitingfor_trigger_none_nextframe_detector.push_back(ColliderDetector_TriggerEvent(*l_source_collider_detector, p_intersected_collider));
				}
			}
		}
	}

	inline void clean_deleted_colliders()
	{
		if (this->deleted_colliders_from_last_step.Size > 0)
		{
			for (size_t i = 0; i < this->deleted_colliders_from_last_step.Size; i++)
			{
				TToken<BoxCollider>& l_deleted_collider = this->deleted_colliders_from_last_step[i];
				for (size_t j = this->in_colliders_processed.Size - 1; j != -1; --j)
				{
					if (TToken_equals(&l_deleted_collider, &this->in_colliders_processed[j]))
					{
						this->in_colliders_processed.erase_at(j, 1);
					}
				}
				for (size_t j = this->is_waitingfor_trigger_stay_detector.Size - 1; j != -1; --j)
				{
					if (TToken_equals(&l_deleted_collider, &this->is_waitingfor_trigger_stay_detector[j].other))
					{
						this->is_waitingfor_trigger_stay_detector.erase_at(j, 1);
					}
				}
				for (size_t j = this->is_waitingfor_trigger_none_detector.Size - 1; j != -1; --j)
				{
					if (TToken_equals(&l_deleted_collider, &this->is_waitingfor_trigger_none_detector[j].other))
					{
						this->is_waitingfor_trigger_none_detector.erase_at(j, 1);
					}
				}

				//Notify other Trigger events
				//We get all ColliderDetector and check if they have an active state with the deleted collider
				//if that's the case, then we invalidate the collision
				for (size_t j = 0; j < this->heap->box_colliders_indices.Size; j++)
				{
					this->no_more_collision(this->heap->box_colliders_indices[j], l_deleted_collider);
				}
			}

			this->deleted_colliders_from_last_step.clear();
		}
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

	inline void on_collider_moved(TToken<BoxCollider>* p_moved_collider, const Transform& p_world_transform, const Math::quat& p_local_rotation)
	{
		this->collision_heap.push_transform(p_moved_collider, p_world_transform, p_local_rotation);
		this->collision_detection_step.on_collider_moved(*p_moved_collider);
	};

	inline void free_boxcollider(TToken<BoxCollider>* p_boxcollider)
	{
		this->collision_heap.free_boxcollider(*p_boxcollider);
		this->collision_detection_step.on_collider_deleted(*p_boxcollider);
	};

	inline void free_colliderdetector(TToken<BoxCollider>* p_collider, TToken<ColliderDetector>* p_collider_detector)
	{
		this->collision_heap.free_colliderdetector(p_collider, p_collider_detector);
	};
};


#include "collision_objects.hpp"

