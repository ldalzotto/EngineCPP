
#include "Common2/common2.hpp"
#include "Math/math.hpp"
#include "Math/transform_def.hpp"
#include "Math/geometry.hpp"

/*
	Collision Shape used for intersection calculation
*/
struct BoxCollider
{
	Math::Transform transform;
	Math::Matrix<3, float> rotation_axis;
	Math::AABB<float> local_box;
};


struct Trigger
{
	enum class State
	{
		UNDEFINED = 0,
		TRIGGER_ENTER = 1,
		TRIGGER_STAY = 2,
		TRIGGER_EXIT = 3,
		NONE = 4
	};
};

/*
	State of the trigger intersection between owner BoxCollider and the other.
*/
struct TriggerState
{
	Token<BoxCollider> other;
	Trigger::State state;
};

inline TriggerState triggerstate_build_default()
{
	return TriggerState{ token_build_default<BoxCollider>(), Trigger::State::UNDEFINED };
};

inline TriggerState triggerstate_build(const Token<BoxCollider>* p_other, const Trigger::State p_state)
{
	return TriggerState{ *p_other, p_state };
};


/*
	A ColliderDetector indicates that a collision shape is emitting TriggerState collision events.
*/
struct ColliderDetector
{
	PoolOfVectorToken<TriggerState> collision_events;
};

inline ColliderDetector colliderdetector_build(const PoolOfVectorToken<TriggerState>* p_collidsion_events)
{
	return ColliderDetector{ *p_collidsion_events };
};

struct CollisionHeap
{
	PoolIndexed<BoxCollider> box_colliders;
	Pool<Token<ColliderDetector>> box_colliders_to_collider_detector;
	PoolIndexed<ColliderDetector> collider_detectors;
	PoolOfVector<TriggerState> collider_detectors_events_2;
};

inline CollisionHeap collisionheap_allocate_default()
{
	return CollisionHeap{
		poolindexed_allocate_default<BoxCollider>(),
		pool_allocate<Token<ColliderDetector>>(0),
		poolindexed_allocate_default<ColliderDetector>(),
		poolofvector_allocate_default<TriggerState>()
	};
};

inline void collisionheap_free(CollisionHeap* p_collisions_heap)
{
	poolindexed_free(&p_collisions_heap->box_colliders);
	pool_free(&p_collisions_heap->box_colliders_to_collider_detector);
	poolindexed_free(&p_collisions_heap->collider_detectors);
	poolofvector_free(&p_collisions_heap->collider_detectors_events_2);
};

inline Token(ColliderDetector)* _collisionheap_get_colliderdetector_from_boxcollider(CollisionHeap* p_collision_heap, const Token(BoxCollider)* p_box_collider)
{
	return pool_get(&p_collision_heap->box_colliders_to_collider_detector, token_cast_p(Token<ColliderDetector>, p_box_collider));
};

inline char _collisionheap_does_boxcollider_have_colliderdetector(CollisionHeap* p_collision_heap, const Token(BoxCollider)* p_box_collider)
{
	if (!pool_is_element_free(&p_collision_heap->box_colliders_to_collider_detector, token_cast_p(Token<ColliderDetector>, p_box_collider)))
	{
		if (_collisionheap_get_colliderdetector_from_boxcollider(p_collision_heap, p_box_collider)->tok != -1)
		{
			return 1;
		}
	};
	return 0;
};

inline Token(ColliderDetector) collisionheap_allocate_colliderdetector(CollisionHeap* p_collision_heap, const Token<BoxCollider>* p_box_collider, const ColliderDetector* p_collider_detector)
{

#if COLLIDER_BOUND_TEST
	//Cannot attach multiple collider detector to a collider for now
	assert_true(!_collisionheap_does_boxcollider_have_colliderdetector(p_collision_heap, p_box_collider));
#endif

	Token(ColliderDetector) l_collider_detector = poolindexed_alloc_element(&p_collision_heap->collider_detectors, p_collider_detector);
	*_collisionheap_get_colliderdetector_from_boxcollider(p_collision_heap, p_box_collider) = l_collider_detector;
};

inline void collisionheap_free_colliderdetector(CollisionHeap* p_collision_heap, const Token<BoxCollider>* p_box_collider, const Token<ColliderDetector>* p_collider_detector)
{
	{
		Token<ColliderDetector>* l_collider_detector_token = _collisionheap_get_colliderdetector_from_boxcollider(p_collision_heap, p_box_collider);
		*l_collider_detector_token = token_build_default<ColliderDetector>();
		pool_release_element(&p_collision_heap->box_colliders_to_collider_detector, token_cast_p(Token(ColliderDetector), p_box_collider));
	}

	{
		ColliderDetector* l_collider_detector = poolindexed_get(&p_collision_heap->collider_detectors, p_collider_detector);
		poolofvector_release_vector(&p_collision_heap->collider_detectors_events_2, &l_collider_detector->collision_events);
		poolindexed_release_element(&p_collision_heap->collider_detectors, p_collider_detector);
	}
};

inline Token(BoxCollider) collisionheap_allocate_boxcollider(CollisionHeap* p_collision_heap, const BoxCollider* p_box_collider)
{
	Token(BoxCollider) l_box_collider_index = poolindexed_alloc_element(&p_collision_heap->box_colliders, p_box_collider);
	pool_alloc_element_1v(&p_collision_heap->box_colliders_to_collider_detector, token_build_default<ColliderDetector>());
	return l_box_collider_index;
};

inline void collisionheap_free_boxcollider(CollisionHeap* p_collision_heap, const Token<BoxCollider>* p_box_collider)
{
	Token<ColliderDetector>* l_collider_detector = _collisionheap_get_colliderdetector_from_boxcollider(p_collision_heap, p_box_collider);
	if (l_collider_detector->tok != -1)
	{
		collisionheap_free_colliderdetector(p_collision_heap, p_box_collider, l_collider_detector);
	}
	poolindexed_release_element(&p_collision_heap->box_colliders, p_box_collider);
};


struct ColliderDetector_IntersectionEvent
{
	Token<ColliderDetector> detector;
	Token<BoxCollider> source;
	Token<BoxCollider> other;
};

inline ColliderDetector_IntersectionEvent colliderdetector_intersectionevent_build(const Token<ColliderDetector>* p_detector, const Token<BoxCollider>* p_source, const Token<BoxCollider>* p_other)
{
	return ColliderDetector_IntersectionEvent
	{
		*p_detector, *p_source, *p_other
	};
};

struct ColliderDetector_TriggerEvent
{
	Token<ColliderDetector> detector;
	Token<BoxCollider> other;
};

inline ColliderDetector_TriggerEvent colliderdetector_triggerevent_build(const Token<ColliderDetector>* p_detector, const Token<BoxCollider>* p_other)
{
	return ColliderDetector_TriggerEvent{ *p_detector, *p_other };
};


struct CollisionDetectionStep
{
	CollisionHeap* heap;
	Vector<Token(BoxCollider)> in_colliders_processed;

	Vector<Token<BoxCollider>> deleted_colliders_from_last_step;

	Vector<ColliderDetector_IntersectionEvent> is_waitingfor_trigger_stay_detector;
	Vector<ColliderDetector_IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;

	Vector<ColliderDetector_TriggerEvent> is_waitingfor_trigger_none_detector;
	Vector<ColliderDetector_TriggerEvent> is_waitingfor_trigger_none_nextframe_detector;

	Vector<Token<ColliderDetector>> lastframe_involved_colliderdetectors;
};

inline CollisionDetectionStep collisiondetectionstep_allocate(CollisionHeap* p_heap)
{
	return CollisionDetectionStep{
		p_heap,
		vector_allocate<Token(BoxCollider)>(0),
		vector_allocate<Token(BoxCollider)>(0),
		vector_allocate<ColliderDetector_IntersectionEvent>(0),
		vector_allocate<ColliderDetector_IntersectionEvent>(0),
		vector_allocate<ColliderDetector_TriggerEvent>(0),
		vector_allocate<ColliderDetector_TriggerEvent>(0),
		vector_allocate<Token<ColliderDetector>>(0)
	};
};

inline void collisiondetectionstep_free(CollisionDetectionStep* p_collisiondetectionstep)
{
	vector_free(&p_collisiondetectionstep->in_colliders_processed);
	vector_free(&p_collisiondetectionstep->deleted_colliders_from_last_step);
	vector_free(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector);
	vector_free(&p_collisiondetectionstep->is_waitingfor_trigger_stay_nextframe_detector);
	vector_free(&p_collisiondetectionstep->is_waitingfor_trigger_none_detector);
	vector_free(&p_collisiondetectionstep->is_waitingfor_trigger_none_nextframe_detector);
	vector_free(&p_collisiondetectionstep->lastframe_involved_colliderdetectors);
};

inline void collisiondetectionstep_clear_lastframe_collision_events(CollisionDetectionStep* p_collisiondetectionstep)
{
	vector_clear(&p_collisiondetectionstep->lastframe_involved_colliderdetectors);
};

inline void collisiondetectionstep_swap_triggerstaw_waiting_buffers(CollisionDetectionStep* p_collisiondetectionstep)
{
	Vector<ColliderDetector_IntersectionEvent> l_tmp = p_collisiondetectionstep->is_waitingfor_trigger_stay_detector;
	p_collisiondetectionstep->is_waitingfor_trigger_stay_detector = p_collisiondetectionstep->is_waitingfor_trigger_stay_nextframe_detector;
	p_collisiondetectionstep->is_waitingfor_trigger_stay_nextframe_detector = l_tmp;
};

inline void collisiondetectionstep_swap_triggernone_waiting_buffers(CollisionDetectionStep* p_collisiondetectionstep)
{
	Vector<ColliderDetector_TriggerEvent> l_tmp = p_collisiondetectionstep->is_waitingfor_trigger_none_detector;
	p_collisiondetectionstep->is_waitingfor_trigger_none_detector = p_collisiondetectionstep->is_waitingfor_trigger_none_nextframe_detector;
	p_collisiondetectionstep->is_waitingfor_trigger_none_nextframe_detector = l_tmp;
};

// When an intersection from the source collider to target occurs
// If there is already a TriggerState event between then, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
inline void collisiondetectionstep_enter_collision(CollisionDetectionStep* p_collisiondetectionstep, const Token(BoxCollider)* p_source_collider, const Token(BoxCollider)* p_intersected_collider)
{
	Token<ColliderDetector>* l_source_collider_detector = _collisionheap_get_colliderdetector_from_boxcollider(p_collisiondetectionstep->heap, p_source_collider);
	if (l_source_collider_detector->tok != -1)
	{
		vector_push_back_element(&p_collisiondetectionstep->lastframe_involved_colliderdetectors, l_source_collider_detector);

		PoolOfVectorToken<TriggerState> l_collider_triggerevents_nestedvector = poolindexed_get(&p_collisiondetectionstep->heap->collider_detectors, l_source_collider_detector)->collision_events;
		VectorOfVector_Element<TriggerState> l_collider_triggerevents = poolofvector_get_vector(&p_collisiondetectionstep->heap->collider_detectors_events_2, &l_collider_triggerevents_nestedvector);
		bool l_trigger_event_found = false;
		for (loop(i, 0, l_collider_triggerevents.Header.Size))
		{
			TriggerState* l_collider_trigger_event = slice_get(&l_collider_triggerevents.Memory, i);
			if (l_collider_trigger_event->other.tok == p_intersected_collider->tok)
			{
				l_collider_trigger_event->state = Trigger::State::TRIGGER_STAY;
				l_trigger_event_found = true;
				break;
			}
		}

		if (!l_trigger_event_found)
		{
			poolofvector_element_push_back_element_2v(
				&p_collisiondetectionstep->heap->collider_detectors_events_2, &l_collider_triggerevents_nestedvector,
				triggerstate_build(p_intersected_collider, Trigger::State::TRIGGER_ENTER)
			);

			vector_push_back_element_1v(
				&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector,
				colliderdetector_intersectionevent_build(l_source_collider_detector, p_source_collider, p_intersected_collider)
			);
		}
	}
};

//We get all ColliderDetector and check if they have an active state with the deleted collider
//if that's the case, then we invalidate the collision
inline void collisiondetectionstep_exit_collision(CollisionDetectionStep* p_collisiondetectionstep, const Token(BoxCollider)* p_source_collider, const Token(BoxCollider)* p_intersected_collider)
{
	Token<ColliderDetector>* l_source_collider_detector = _collisionheap_get_colliderdetector_from_boxcollider(p_collisiondetectionstep->heap, p_source_collider);
	if (l_source_collider_detector->tok != -1)
	{
		VectorOfVector_Element<TriggerState> l_collider_triggerevents = poolofvector_get_vector(
			&p_collisiondetectionstep->heap->collider_detectors_events_2,
			&poolindexed_get(&p_collisiondetectionstep->heap->collider_detectors, l_source_collider_detector)->collision_events
		);

		for (loop(i, 0, l_collider_triggerevents.Header.Size))
		{
			TriggerState* l_trigger_event = slice_get(&l_collider_triggerevents.Memory, i);
			if (l_trigger_event->other.tok == p_intersected_collider->tok)
			{
				l_trigger_event->state = Trigger::State::TRIGGER_EXIT;
				// next step, collision event will be deleted
				vector_push_back_element_1v(&p_collisiondetectionstep->is_waitingfor_trigger_none_detector, colliderdetector_triggerevent_build(l_source_collider_detector, p_intersected_collider));
			}
		}
	}
};

inline void collisiondetectionstep_clean_deleted_colliders(CollisionDetectionStep* p_collisiondetectionstep)
{
	if (p_collisiondetectionstep->deleted_colliders_from_last_step.Size > 0)
	{
		for (vector_loop(&p_collisiondetectionstep->deleted_colliders_from_last_step, i))
		{
			Token(BoxCollider)* l_deleted_collider = vector_get(&p_collisiondetectionstep->deleted_colliders_from_last_step, i);

			vector_erase_if_2_begin(&p_collisiondetectionstep->in_colliders_processed, j, l_collider)
				char l_erased = l_collider->tok == l_deleted_collider->tok;
			vector_erase_if_2_end(&p_collisiondetectionstep->in_colliders_processed, j, l_erased);

			vector_erase_if_2_begin(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, j, l_intsersrection_event)
				char l_erased = l_intsersrection_event->other.tok == l_deleted_collider->tok;
			vector_erase_if_2_end(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, j, l_erased);

			vector_erase_if_2_begin(&p_collisiondetectionstep->is_waitingfor_trigger_none_detector, j, l_trigger_event)
				char l_erased = l_trigger_event->other.tok == l_deleted_collider->tok;
			vector_erase_if_2_end(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, j, l_erased);
			//Notify other Trigger events

				//Notify other Trigger events
			poolindexed_foreach_token_2_begin(&p_collisiondetectionstep->heap->box_colliders, j, l_box_collider)
				collisiondetectionstep_exit_collision(p_collisiondetectionstep, l_box_collider, l_deleted_collider);
			poolindexed_foreach_token_2_end()
		}

		vector_clear(&p_collisiondetectionstep->deleted_colliders_from_last_step);
	}
};

inline void collisiondetectionstep_on_collision_detected(CollisionDetectionStep* p_collisiondetectionstep, const Token<BoxCollider>* p_left, const Token<BoxCollider>* p_right)
{
	collisiondetectionstep_enter_collision(p_collisiondetectionstep, p_left, p_right);
	collisiondetectionstep_enter_collision(p_collisiondetectionstep, p_right, p_left);
};

inline void collisiondetectionstep_on_collision_detection_failed(CollisionDetectionStep* p_collisiondetectionstep, const Token<BoxCollider>* p_left, const Token<BoxCollider>* p_right)
{
	collisiondetectionstep_exit_collision(p_collisiondetectionstep, p_left, p_right);
	collisiondetectionstep_exit_collision(p_collisiondetectionstep, p_right, p_left);
};

inline void collisiondetectionstep_process_input_colliders(CollisionDetectionStep* p_collisiondetectionstep)
{
	for (loop(i, 0, p_collisiondetectionstep->in_colliders_processed.Size))
	{
		Token(BoxCollider)* l_left_collider_token = vector_get(&p_collisiondetectionstep->in_colliders_processed, i);

		if (_collisionheap_does_boxcollider_have_colliderdetector(p_collisiondetectionstep->heap, l_left_collider_token))
		{
			BoxCollider* l_left_collider = poolindexed_get(&p_collisiondetectionstep->heap->box_colliders, l_left_collider_token);
			Math::OBB<float>l_left_projected = Geometry::to_obb(l_left_collider->local_box, l_left_collider->transform, l_left_collider->rotation_axis);

			poolindexed_foreach_token_2_begin(&p_collisiondetectionstep->heap->box_colliders, j, l_right_collider_token);
					//TODO -> FIX - we also need to avoid calculatin the same collision between two Colliders (in the case where multiple colliders in this->in_colliders_processed are colliding between each other)
					//		  because the on_collision_detected and on_collision_detection_failed operations are bidirectional.
					//		  This bug presents the TRIGGER_ENTER event to be triggered.

					//Avoid self test
					if (l_left_collider_token->tok != l_right_collider_token->tok)
					{
						BoxCollider* l_right_collider = poolindexed_get(&p_collisiondetectionstep->heap->box_colliders, l_right_collider_token);
						Math::OBB<float>l_right_projected = Geometry::to_obb(l_right_collider->local_box, l_right_collider->transform, l_right_collider->rotation_axis);

						if (Geometry::overlap3(l_left_projected, l_right_projected))
						{
							collisiondetectionstep_on_collision_detected(p_collisiondetectionstep, l_left_collider_token, l_right_collider_token);
						}
						else
						{
							collisiondetectionstep_on_collision_detection_failed(p_collisiondetectionstep, l_left_collider_token, l_right_collider_token);
						}
					}
			poolindexed_foreach_token_2_end();
		};
	}

	vector_clear(&p_collisiondetectionstep->in_colliders_processed);
};

inline void collisiondetector_set_triggerstate_matchingWith_boxcollider(
		Token<ColliderDetector>* p_collision_detector, CollisionHeap* p_collisionheap, const Token(BoxCollider)* p_matched_boxcollider,const Trigger::State p_trigger_state)
{
	ColliderDetector* l_collider_detector = poolindexed_get(&p_collisionheap->collider_detectors, p_collision_detector);
	VectorOfVector_Element<TriggerState> l_events = poolofvector_get_vector(&p_collisionheap->collider_detectors_events_2, &l_collider_detector->collision_events);
	for (loop(i, 0, l_events.Header.Size))
	{
		TriggerState* l_trigger_event = slice_get(&l_events.Memory, i);
		if (l_trigger_event->other.tok == p_matched_boxcollider->tok)
		{
			l_trigger_event->state = p_trigger_state;
		}
	}
};


inline void collisiondetectionstep_update_pending_detectors(CollisionDetectionStep* p_collisiondetectionstep)
{
	for (vector_loop(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, i))
	{
		ColliderDetector_IntersectionEvent* l_intersection_event = vector_get(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, i);
		collisiondetector_set_triggerstate_matchingWith_boxcollider(&l_intersection_event->detector,
			p_collisiondetectionstep->heap, &l_intersection_event->other, Trigger::State::TRIGGER_STAY);
	}

	for (vector_loop(&p_collisiondetectionstep->is_waitingfor_trigger_none_detector, i))
	{
		ColliderDetector_IntersectionEvent* l_intersection_event = vector_get(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector, i);
		collisiondetector_set_triggerstate_matchingWith_boxcollider(&l_intersection_event->detector,
			p_collisiondetectionstep->heap, &l_intersection_event->other, Trigger::State::NONE);
	}

	vector_clear(&p_collisiondetectionstep->is_waitingfor_trigger_stay_detector);
	vector_clear(&p_collisiondetectionstep->is_waitingfor_trigger_none_detector);
};

inline void collisiondetectionstep_step(CollisionDetectionStep* p_collisiondetectionstep)
{
	collisiondetectionstep_clear_lastframe_collision_events(p_collisiondetectionstep);
	collisiondetectionstep_swap_triggerstaw_waiting_buffers(p_collisiondetectionstep);
	collisiondetectionstep_swap_triggernone_waiting_buffers(p_collisiondetectionstep);
	collisiondetectionstep_clean_deleted_colliders(p_collisiondetectionstep);
	
	collisiondetectionstep_process_input_colliders(p_collisiondetectionstep);

	collisiondetectionstep_update_pending_detectors(p_collisiondetectionstep);
};


struct Collision
{
	CollisionHeap collision_heap;
	CollisionDetectionStep collision_detection_step;
};

inline Collision* collision_allocate()
{
	Collision* l_collision = cast(Collision*, heap_malloc(sizeof(Collision)));
	l_collision->collision_heap = collisionheap_allocate_default();
	l_collision->collision_detection_step = collisiondetectionstep_allocate(&l_collision->collision_heap);
	return l_collision;
};

inline void collision_free(Collision* p_collision)
{
	collisiondetectionstep_free(&p_collision->collision_detection_step);
	collisionheap_free(&p_collision->collision_heap);
};

inline void collision_update(Collision* p_collision)
{
	collisiondetectionstep_step(&p_collision->collision_detection_step);
};

//TODO