
#include "Collision/collision.hpp"

#include "./collision_definition.hpp"


inline BoxCollider BoxCollider::build_from_local_aabb(const char p_enabled, const aabb& p_local_box)
{
	BoxCollider l_box_collider;
	l_box_collider.enabled = p_enabled;
	l_box_collider.local_box = p_local_box;
	return l_box_collider;
};



inline TriggerEvent TriggerEvent::build_default()
{
	return TriggerEvent{ token_build_default<BoxCollider>(), Trigger::State::UNDEFINED };
};

inline TriggerEvent TriggerEvent::build(const Token<BoxCollider> p_other, const Trigger::State p_state)
{
	return TriggerEvent{ p_other, p_state };
};


inline ColliderDetector ColliderDetector::build(const v2::PoolOfVectorToken<TriggerEvent> p_collidsion_events)
{
	return ColliderDetector{ p_collidsion_events };
};



inline CollisionHeap2 CollisionHeap2::allocate_default()
{
	return CollisionHeap2{
		v2::PoolIndexed<BoxCollider>::allocate_default(),
		v2::Pool<Token<ColliderDetector>>::allocate(0),
		v2::PoolIndexed<ColliderDetector>::allocate_default(),
		v2::PoolOfVector<TriggerEvent>::allocate_default()
	};
};

inline void CollisionHeap2::free()
{
#if COLLIDER_BOUND_TEST
	assert_true(!this->box_colliders.has_allocated_elements());
	assert_true(!this->box_colliders_to_collider_detector.has_allocated_elements());
	assert_true(!this->collider_detectors.has_allocated_elements());
	assert_true(!this->collider_detectors_events_2.has_allocated_elements());
#endif

	this->box_colliders.free();
	this->box_colliders_to_collider_detector.free();
	this->collider_detectors.free();
	this->collider_detectors_events_2.free();
};

inline Token(ColliderDetector) CollisionHeap2::allocate_colliderdetector(const Token<BoxCollider> p_box_collider)
{

#if COLLIDER_BOUND_TEST
	//Cannot attach multiple collider detector to a collider for now
	assert_true(!this->does_boxcollider_have_colliderdetector(p_box_collider));
#endif

	v2::PoolOfVectorToken<TriggerEvent> l_trigger_events = this->collider_detectors_events_2.alloc_vector();
	Token(ColliderDetector) l_collider_detector_token = this->collider_detectors.alloc_element(ColliderDetector::build(l_trigger_events));
	this->get_colliderdetector_from_boxcollider(p_box_collider) = l_collider_detector_token;
	return l_collider_detector_token;
};

inline void CollisionHeap2::free_colliderdetector(const Token<BoxCollider> p_box_collider, const Token<ColliderDetector> p_collider_detector)
{

	{
		ColliderDetector& l_collider_detector = this->collider_detectors.get(p_collider_detector);
		this->collider_detectors_events_2.release_vector(l_collider_detector.collision_events);
		this->collider_detectors.release_element(p_collider_detector);
	}

	{
		this->get_colliderdetector_from_boxcollider(p_box_collider) = token_build_default<ColliderDetector>();
		// this->box_colliders_to_collider_detector.release_element(token_cast_p(Token(ColliderDetector), p_box_collider));
	}

};

inline Token(BoxCollider) CollisionHeap2::allocate_boxcollider(const BoxCollider& p_box_collider)
{
	Token(BoxCollider) l_box_collider_index = this->box_colliders.alloc_element(p_box_collider);
	this->box_colliders_to_collider_detector.alloc_element(token_build_default<ColliderDetector>());
	return l_box_collider_index;
};

inline void CollisionHeap2::push_boxcollider_transform(Token<BoxCollider> p_boxcollider, const transform_pa& p_world_transform)
{
	BoxCollider& l_boxcollider = this->box_colliders.get(p_boxcollider);
	l_boxcollider.transform = p_world_transform;
};

inline void CollisionHeap2::free_boxcollider(const Token<BoxCollider> p_box_collider)
{
	Token<ColliderDetector>& l_collider_detector = this->get_colliderdetector_from_boxcollider(p_box_collider);
	if (l_collider_detector.tok != -1)
	{
		this->free_colliderdetector(p_box_collider, l_collider_detector);
	}
	this->box_colliders.release_element(p_box_collider);
	this->box_colliders_to_collider_detector.release_element(token_cast_v(Token(ColliderDetector), p_box_collider));
};

inline Token<ColliderDetector>& CollisionHeap2::get_colliderdetector_from_boxcollider(const Token<BoxCollider> p_box_collider)
{
	return this->box_colliders_to_collider_detector.get(token_cast_v(Token<ColliderDetector>, p_box_collider));
};

inline Slice<TriggerEvent> CollisionHeap2::get_triggerevents_from_boxcollider(const Token(BoxCollider) p_box_collider)
{
	if (this->does_boxcollider_have_colliderdetector(p_box_collider))
	{
		Token(ColliderDetector)& l_collider_detextor = this->get_colliderdetector_from_boxcollider(p_box_collider);
		return this->collider_detectors_events_2.get_vector(this->collider_detectors.get(l_collider_detextor).collision_events);
	}
	return Slice<TriggerEvent>::build_default();
};

inline Slice<TriggerEvent> CollisionHeap2::get_triggerevents_from_colliderdetector(const Token(ColliderDetector) p_collider_detector)
{
	return this->collider_detectors_events_2.get_vector(this->collider_detectors.get(p_collider_detector).collision_events);
};

inline char CollisionHeap2::does_boxcollider_have_colliderdetector(const Token(BoxCollider) p_box_collider)
{
	if (!this->box_colliders_to_collider_detector.is_element_free(token_cast_v(Token<ColliderDetector>, p_box_collider)))
	{
		if (this->get_colliderdetector_from_boxcollider(p_box_collider).tok != -1)
		{
			return 1;
		}
	};
	return 0;
};




inline CollisionDetectionStep::IntersectionEvent CollisionDetectionStep::IntersectionEvent::build(const Token<ColliderDetector> p_detector, const Token<BoxCollider> p_other)
{
	return IntersectionEvent{ p_detector, p_other };
};

inline char CollisionDetectionStep::IntersectionEvent::equals_intersectionevent(const IntersectionEvent& p_other)
{
	return (this->detector.tok == p_other.detector.tok) && (this->other.tok == p_other.other.tok);
};



inline CollisionDetectionStep::CollisionDetectorDeletionEvent CollisionDetectionStep::CollisionDetectorDeletionEvent::build(const Token<BoxCollider> p_box_collider, const Token<ColliderDetector> p_collider_detector)
{
	return CollisionDetectorDeletionEvent{ p_box_collider, p_collider_detector };
};



inline CollisionDetectionStep CollisionDetectionStep::allocate(CollisionHeap2* p_heap)
{
	return CollisionDetectionStep{
		p_heap,
		v2::Vector<Token(BoxCollider)>::allocate(0),
		v2::Vector<Token(BoxCollider)>::allocate(0),
		v2::Vector<Token(BoxCollider)>::allocate(0),
		v2::Vector<CollisionDetectorDeletionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
		v2::Vector<IntersectionEvent>::allocate(0),
	};
};

inline void CollisionDetectionStep::free()
{
	//To free pending resources
	this->step_freeingresource_only();

	this->in_colliders_processed.free();
	this->deleted_colliders_from_last_step.free();
	this->deleted_collider_detectors_from_last_step.free();
	this->currentstep_enter_intersection_events.free();
	this->currentstep_exit_intersection_events.free();
	this->is_waitingfor_trigger_stay_detector.free();
	this->is_waitingfor_trigger_stay_nextframe_detector.free();
	this->is_waitingfor_trigger_none_detector.free();
	this->is_waitingfor_trigger_none_nextframe_detector.free();
};

/*
A *frame* of the collision engine.
	1* Swap "per frame" buffers.

	2* Execute ColliderDetectors destroy event.
	   By :
		 1/ Cleaning all references of the deleted Detectors from all CollisionDetectionStep calculation data.
			To avoid manipulating Detectors that have already been deleted.
		 2/ Freeing detectors from the heap.

	3* Execute Colliders destroy event.
	   By :
		 1/ Cleaning all references of the attached Detectors from all CollisionDetectionStep calculation data.
		 2/ Cleaning all references of the deleted Colliders from all CollisionDetectionStep calculation data.
		 3/ Freeing Collides from the heap.

	4* Process moved colliders.
	   By :
		 1/ Calculating intersections for every possible combinations with the moved Colliders that have a ColliderDetector.
		 2/ Pushing enter and exit IntersectionEvents for further processing.
	   The Colliders processing may induce duplicate intersections. eg: two Colliders with ColliderDetectors that have moved and intersect will generate 4 IntersectionEvents.
	   IntersectionEvents generation is always done from the point of view of a single ColliderDetector. This is to handle the case where a ColliderDetector that is not moving (thus, not processed)
	   stille generates events when another collider intersect with it.

	5* Removing IntersectionEvents duplicates.

	6* Update the TriggerEvent state based on last frame IntersectionEvents.
		When TriggerEvent states are updated in 7*, they may also generated deferred IntersectionEvents that will be processed the next frame.
		For example, when a TriggerState value is set to TRIGGER_ENTER, then if nothing happens, on the next frame, the TriggerState will automaticaly be set to
		TRIGGER_STAY.

	7* Update the TriggerEvent state based on generated IntersectionEvents.

	8* 9* Clear resrouces.
*/
inline void CollisionDetectionStep::step()
{
	this->swap_detector_events(); // 1*

	this->process_deleted_collider_detectors(); // 2*
	this->process_deleted_colliders(); // 3*

	this->process_input_colliders(); // 4*
	this->remove_current_step_event_duplicates(); // 5*

	this->udpate_triggerstate_from_lastframe_intersectionevents(); // 6*
	this->udpate_triggerstate_from_intersectionevents(); // 7*

	this->clear_current_step_events(); // 8*

	this->free_deleted_colliders(); // 9*
};

inline void CollisionDetectionStep::push_collider_for_process(const Token<BoxCollider> p_moved_collider)
{
	this->in_colliders_processed.push_back_element(p_moved_collider);
};

inline void CollisionDetectionStep::push_collider_for_deletion(const Token<BoxCollider> p_collider)
{
	this->deleted_colliders_from_last_step.push_back_element(p_collider);
};

inline void CollisionDetectionStep::push_collider_detector_for_deletion(const Token<BoxCollider> p_collider, const Token<ColliderDetector> p_detector)
{
	this->deleted_collider_detectors_from_last_step.push_back_element(CollisionDetectorDeletionEvent::build(p_collider, p_detector));
};


inline void CollisionDetectionStep::step_freeingresource_only()
{
	this->process_deleted_collider_detectors();
	this->free_deleted_colliders();
};

inline void CollisionDetectionStep::swap_detector_events()
{
	{
		v2::Vector<IntersectionEvent> l_tmp = this->is_waitingfor_trigger_stay_detector;
		this->is_waitingfor_trigger_stay_detector = this->is_waitingfor_trigger_stay_nextframe_detector;
		this->is_waitingfor_trigger_stay_nextframe_detector = l_tmp;
	}
	{
		v2::Vector<IntersectionEvent> l_tmp = this->is_waitingfor_trigger_none_detector;
		this->is_waitingfor_trigger_none_detector = this->is_waitingfor_trigger_none_nextframe_detector;
		this->is_waitingfor_trigger_none_nextframe_detector = l_tmp;
	}
};

// When an intersection from the source collider to target occurs
// If there is already a TriggerEvent event between them, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
inline void CollisionDetectionStep::enter_collision(const IntersectionEvent& p_intersection_event)
{
	v2::PoolOfVectorToken<TriggerEvent> l_collider_triggerevents_nestedvector = this->heap->collider_detectors.get(p_intersection_event.detector).collision_events;
	Slice<TriggerEvent> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(l_collider_triggerevents_nestedvector);
	bool l_trigger_event_found = false;
	for (loop(i, 0, l_collider_triggerevents.Size))
	{
		TriggerEvent& l_collider_trigger_event = l_collider_triggerevents.get(i);
		if (l_collider_trigger_event.other.tok == p_intersection_event.other.tok)
		{
			l_collider_trigger_event.state = Trigger::State::TRIGGER_STAY;
			l_trigger_event_found = true;
			break;
		}
	}

	if (!l_trigger_event_found)
	{
		this->heap->collider_detectors_events_2.element_push_back_element(
			l_collider_triggerevents_nestedvector,
			TriggerEvent::build(p_intersection_event.other, Trigger::State::TRIGGER_ENTER)
		);

		this->is_waitingfor_trigger_stay_nextframe_detector.push_back_element(p_intersection_event);
	}
};

//We get all ColliderDetector associated to the p_source_collider and check if they have an active state with the involved collider
//if that's the case, then we invalidate the collision
inline void CollisionDetectionStep::exit_collision(const IntersectionEvent& p_intersection_event)
{
	Slice<TriggerEvent> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(
		this->heap->collider_detectors.get(p_intersection_event.detector).collision_events
	);

	for (loop(i, 0, l_collider_triggerevents.Size))
	{
		TriggerEvent& l_trigger_event = l_collider_triggerevents.get(i);
		if (l_trigger_event.other.tok == p_intersection_event.other.tok)
		{
			l_trigger_event.state = Trigger::State::TRIGGER_EXIT;
			// on next step, collision event will be deleted
			this->is_waitingfor_trigger_none_nextframe_detector.push_back_element(p_intersection_event);
		}
	}
};

inline void CollisionDetectionStep::remove_references_to_colliderdetector(const Token(ColliderDetector) p_collider_detector)
{
	vector_erase_if_2_begin(&this->is_waitingfor_trigger_stay_detector, j, l_intsersrection_event)
		char l_erased = l_intsersrection_event.detector.tok == p_collider_detector.tok;
	vector_erase_if_2_end(&this->is_waitingfor_trigger_stay_detector, j, l_erased);

	vector_erase_if_2_begin(&this->is_waitingfor_trigger_none_detector, j, l_trigger_event)
		char l_erased = l_trigger_event.detector.tok == p_collider_detector.tok;
	vector_erase_if_2_end(&this->is_waitingfor_trigger_none_detector, j, l_erased);

	vector_erase_if_2_begin(&this->in_colliders_processed, j, l_disabled_collider);
	char l_erased = false;
	Token(ColliderDetector)& l_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_disabled_collider);
	if (l_collider_detector.tok != -1 && l_collider_detector.tok == p_collider_detector.tok)
	{
		l_erased = true;
	}
	vector_erase_if_2_end(&this->in_colliders_processed, j, l_erased);
};

// /!\ Do not take care of the associated ColliderDetectors.
inline void CollisionDetectionStep::remove_references_to_boxcollider(const Token(BoxCollider) p_box_collider)
{
	vector_erase_if_2_begin(&this->in_colliders_processed, j, l_collider)
		char l_erased = l_collider.tok == p_box_collider.tok;
	vector_erase_if_2_end(&this->in_colliders_processed, j, l_erased);

	vector_erase_if_2_begin(&this->is_waitingfor_trigger_stay_detector, j, l_intsersrection_event)
		char l_erased = l_intsersrection_event.other.tok == p_box_collider.tok;
	vector_erase_if_2_end(&this->is_waitingfor_trigger_stay_detector, j, l_erased);

	vector_erase_if_2_begin(&this->is_waitingfor_trigger_none_detector, j, l_trigger_event)
		char l_erased = l_trigger_event.other.tok == p_box_collider.tok;
	vector_erase_if_2_end(&this->is_waitingfor_trigger_none_detector, j, l_erased);
};

// Norify all ColliderDetectors with an exit_collision event.
//TODO -> In the future, we want to partition the space to not notify the entire world
inline void CollisionDetectionStep::generate_exit_collision_for_collider(const Token(BoxCollider) p_box_collider)
{
	poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_box_collider)
	{
		if (this->heap->box_colliders.get(l_box_collider).enabled)
		{
			Token(ColliderDetector)& l_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_box_collider);
			if (l_collider_detector.tok != -1)
			{
				this->currentstep_exit_intersection_events.push_back_element(IntersectionEvent::build(l_collider_detector, p_box_collider));
			}
		}
	}
	poolindexed_foreach_token_2_end()
};

inline void CollisionDetectionStep::process_deleted_collider_detectors()
{
	for (vector_loop(&this->deleted_collider_detectors_from_last_step, i))
	{
		CollisionDetectorDeletionEvent& l_deletion_event = this->deleted_collider_detectors_from_last_step.get(i);
		this->remove_references_to_colliderdetector(l_deletion_event.detector);
		this->heap->free_colliderdetector(l_deletion_event.collider, l_deletion_event.detector);
	}
	this->deleted_collider_detectors_from_last_step.clear();
};

inline void CollisionDetectionStep::process_deleted_colliders()
{
	// dereferencing ColliderDetectors and Colliders.
	for (vector_loop(&this->deleted_colliders_from_last_step, i))
	{
		Token(BoxCollider)& l_deleted_collider = this->deleted_colliders_from_last_step.get(i);
		Token(ColliderDetector)& l_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_deleted_collider);
		if (l_collider_detector.tok != -1)
		{
			this->remove_references_to_colliderdetector(l_collider_detector);
			this->heap->free_colliderdetector(l_deleted_collider, l_collider_detector);
		}

		this->heap->box_colliders.get(l_deleted_collider).enabled = false;
		this->remove_references_to_boxcollider(l_deleted_collider);
	}

	// Once the collision step is cleaned up, we can generate exit_collision event
	// to notify all other ColliderDetectors that a collider has gone
	// Some exit events will be false positive (event sended but there was no collision at the first plane), 
	// but that's not a problem as it will be ignored by the udpate_triggerstate_from_intersectionevents step.
	for (vector_loop(&this->deleted_colliders_from_last_step, i))
	{
		Token(BoxCollider)& l_disabled_box_collider_token = this->deleted_colliders_from_last_step.get(i);
		this->generate_exit_collision_for_collider(l_disabled_box_collider_token);
	}
};

inline void CollisionDetectionStep::process_input_colliders()
{
	for (loop(i, 0, this->in_colliders_processed.Size))
	{
		Token(BoxCollider)& l_left_collider_token = this->in_colliders_processed.get(i);
		Token(ColliderDetector)& l_left_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_left_collider_token);


		// If the processed collider have a collider detector, we calculate intersection with other BoxColliders
		// then push collision_event according to the intersection result
		if (l_left_collider_detector.tok != -1)
		{
			BoxCollider& l_left_collider = this->heap->box_colliders.get(l_left_collider_token);
			if (l_left_collider.enabled)
			{
				obb l_left_projected = l_left_collider.local_box.add_position_rotation(l_left_collider.transform);

				//TODO -> In the future, we want to avoid to query the world
				poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_right_collider_token);

				//Avoid self test
				if (l_left_collider_token.tok != l_right_collider_token.tok)
				{
					BoxCollider& l_right_collider = this->heap->box_colliders.get(l_right_collider_token);
					if (l_right_collider.enabled)
					{
						obb l_right_projected = l_left_collider.local_box.add_position_rotation(l_right_collider.transform);

						if (l_left_projected.overlap2(l_right_projected))
						{
							this->currentstep_enter_intersection_events.push_back_element(
								IntersectionEvent::build(
									l_left_collider_detector,
									l_right_collider_token
								)
							);

							Token<ColliderDetector>& l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
							if (l_right_collider_detector.tok != -1)
							{
								this->currentstep_enter_intersection_events.push_back_element(
									IntersectionEvent::build(
										l_right_collider_detector,
										l_left_collider_token
									)
								);
							}
						}
						else
						{
							this->currentstep_exit_intersection_events.push_back_element(
								IntersectionEvent::build(
									l_left_collider_detector,
									l_right_collider_token
								)
							);

							Token<ColliderDetector>& l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
							if (l_right_collider_detector.tok != -1)
							{
								this->currentstep_exit_intersection_events.push_back_element(
									IntersectionEvent::build(
										l_right_collider_detector,
										l_left_collider_token
									)
								);
							}
						}
					}
				}
				poolindexed_foreach_token_2_end();
			}
		}
		// If the processed collider doesn't have a collider detector, we get all other collider detectors and test collision
		// then push collision_event according to the intersection result
		else
		{
			BoxCollider& l_left_collider = this->heap->box_colliders.get(l_left_collider_token);
			if (l_left_collider.enabled)
			{
				obb l_left_projected = l_left_collider.local_box.add_position_rotation(l_left_collider.transform);

				//TODO -> In the future, we want to avoid to query the world
				poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_right_collider_token)
				{
					BoxCollider& l_right_collider = this->heap->box_colliders.get(l_right_collider_token);
					if (l_right_collider.enabled)
					{
						Token(ColliderDetector)& l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
						if (l_right_collider_detector.tok != -1)
						{
							obb l_right_projected = l_left_collider.local_box.add_position_rotation(l_right_collider.transform);

							if (l_left_projected.overlap2(l_right_projected))
							{
								this->currentstep_enter_intersection_events.push_back_element(
									IntersectionEvent::build(l_right_collider_detector, l_left_collider_token)
								);
							}
							else
							{
								this->currentstep_exit_intersection_events.push_back_element(
									IntersectionEvent::build(l_right_collider_detector, l_left_collider_token)
								);
							}
						}
					}
				}
				poolindexed_foreach_token_2_end()
			}
		}
	}

	this->in_colliders_processed.clear();
};

inline void CollisionDetectionStep::remove_intersectionevents_duplicate(v2::Vector<IntersectionEvent>* in_out_intersection_events)
{
	for (loop(i, 0, in_out_intersection_events->Size))
	{
		IntersectionEvent& l_left_intersection_event = in_out_intersection_events->get(i);
		for (loop(j, i + 1, in_out_intersection_events->Size))
		{
			IntersectionEvent& l_right_intersection_event = in_out_intersection_events->get(j);

			if (l_left_intersection_event.equals_intersectionevent(l_right_intersection_event))
			{
				in_out_intersection_events->erase_element_at(j);
			}
		}
	}
};

/*
	Previous step may push the same intersection events. Because a collider detector and it's related box collider may have moved, so they both generate the same intersection event.
*/
inline void CollisionDetectionStep::remove_current_step_event_duplicates()
{
	this->remove_intersectionevents_duplicate(&this->currentstep_enter_intersection_events);
	this->remove_intersectionevents_duplicate(&this->currentstep_exit_intersection_events);
};

inline void CollisionDetectionStep::udpate_triggerstate_from_intersectionevents()
{
	vector_foreach_begin(&this->currentstep_enter_intersection_events, i, l_intersection_event);
	this->enter_collision(l_intersection_event);
	vector_foreach_end();

	vector_foreach_begin(&this->currentstep_exit_intersection_events, i, l_intersection_event);
	this->exit_collision(l_intersection_event);
	vector_foreach_end();
};

inline void CollisionDetectionStep::clear_current_step_events()
{
	this->currentstep_enter_intersection_events.clear();
	this->currentstep_exit_intersection_events.clear();
};

inline void CollisionDetectionStep::set_triggerstate_matchingWith_boxcollider(const Token<ColliderDetector> p_collision_detector, const Token(BoxCollider) p_matched_boxcollider, const Trigger::State p_trigger_state)
{
	ColliderDetector& l_collider_detector = this->heap->collider_detectors.get(p_collision_detector);
	Slice<TriggerEvent> l_events = this->heap->collider_detectors_events_2.get_vector(l_collider_detector.collision_events);
	for (loop(i, 0, l_events.Size))
	{
		TriggerEvent& l_trigger_event = l_events.get(i);
		if (l_trigger_event.other.tok == p_matched_boxcollider.tok)
		{
			l_trigger_event.state = p_trigger_state;
		}
	}
};

inline void CollisionDetectionStep::udpate_triggerstate_from_lastframe_intersectionevents()
{
	for (vector_loop(&this->is_waitingfor_trigger_stay_detector, i))
	{
		IntersectionEvent& l_intersection_event = this->is_waitingfor_trigger_stay_detector.get(i);
		this->set_triggerstate_matchingWith_boxcollider(l_intersection_event.detector, l_intersection_event.other, Trigger::State::TRIGGER_STAY);
	}

	for (vector_loop(&this->is_waitingfor_trigger_none_detector, i))
	{
		IntersectionEvent& l_intersection_event = this->is_waitingfor_trigger_none_detector.get(i);
		this->set_triggerstate_matchingWith_boxcollider(l_intersection_event.detector, l_intersection_event.other, Trigger::State::NONE);
	}

	this->is_waitingfor_trigger_stay_detector.clear();
	this->is_waitingfor_trigger_none_detector.clear();
};

inline void CollisionDetectionStep::free_deleted_colliders()
{
	for (vector_loop(&this->deleted_colliders_from_last_step, i))
	{
		this->heap->free_boxcollider(this->deleted_colliders_from_last_step.get(i));
	}
	this->deleted_colliders_from_last_step.clear();
};



/*
	The Collision engine provides a way to hook when a 3D geometric shape enters in collision with anoter one.
	It is currently a "brute-force" implementation. Meaning that Colliders are not spatially indexed.
*/
struct Collision2
{

	CollisionHeap2 collision_heap;
	CollisionDetectionStep collision_detection_step;

	inline static void allocate(Collision2** p_collision_ptr)
	{
		*p_collision_ptr = cast(Collision2*, heap_malloc(sizeof(Collision2)));
		(*p_collision_ptr)->collision_heap = CollisionHeap2::allocate_default();
		(*p_collision_ptr)->collision_detection_step = CollisionDetectionStep::allocate(&(*p_collision_ptr)->collision_heap);
	};

	inline static void free(Collision2** p_collision_ptr)
	{
		(*p_collision_ptr)->free();
		heap_free(cast(char*, *p_collision_ptr));
	};

	inline void free()
	{
		this->collision_detection_step.free();
		this->collision_heap.free();
	};

	inline void step()
	{
		this->collision_detection_step.step();
	};

	struct ExternalInterface
	{
		inline static Token<BoxCollider> allocate_boxcollider(Collision2* thiz, const aabb& p_local_box)
		{
			return thiz->collision_heap.allocate_boxcollider(BoxCollider::build_from_local_aabb(true, p_local_box));
		};

		inline static void on_collider_moved(Collision2* thiz, const Token<BoxCollider> p_moved_collider, const transform_pa& p_world_transform)
		{
			thiz->collision_heap.push_boxcollider_transform(p_moved_collider, p_world_transform);
			thiz->collision_detection_step.push_collider_for_process(p_moved_collider);
		};

		inline static void free_collider(Collision2* thiz, const Token<BoxCollider> p_moved_collider)
		{
			thiz->collision_detection_step.push_collider_for_deletion(p_moved_collider);
		};


		inline static Token<ColliderDetector> allocate_colliderdetector(Collision2* thiz, const Token<BoxCollider> p_box_collider)
		{
			return thiz->collision_heap.allocate_colliderdetector(p_box_collider);
		};

		inline static void free_colliderdetector(Collision2* thiz, const  Token<BoxCollider> p_collider, const Token<ColliderDetector> p_collider_detector)
		{
			thiz->collision_detection_step.push_collider_detector_for_deletion(p_collider, p_collider_detector);
		};
	};
};

#include "./collision_objects.hpp"
