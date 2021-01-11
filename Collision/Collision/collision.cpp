
#include "Collision/collision.hpp"
#include "Math/math.hpp"
#include "Math/transform_def.hpp"
#include "Math/geometry.hpp"

struct Collision2
{

	/*
		Collision Shape used for intersection calculation
	*/
	struct BoxCollider
	{
		Math::Transform transform;
		Math::Matrix<3, float> rotation_axis;
		Math::AABB<float> local_box;

		inline static BoxCollider build_from_local_aabb(const Math::AABB<float>* p_local_box)
		{
			BoxCollider l_box_collider;
			l_box_collider.local_box = *p_local_box;
			return l_box_collider;
		};
	};

	/*
		State of the trigger intersection between owner BoxCollider and the other.
	*/
	struct TriggerEvent
	{
		Token<BoxCollider> other;
		Trigger::State state;

		inline static TriggerEvent build_default()
		{
			return TriggerEvent{ token_build_default<BoxCollider>(), Trigger::State::UNDEFINED };
		};

		inline static TriggerEvent build(const Token<BoxCollider>* p_other, const Trigger::State p_state)
		{
			return TriggerEvent{ *p_other, p_state };
		};
	};

	/*
		A ColliderDetector indicates that a collision shape is emitting TriggerEvent collision events.
	*/
	struct ColliderDetector
	{
		v2::PoolOfVectorToken<TriggerEvent> collision_events;

		inline static ColliderDetector build(const v2::PoolOfVectorToken<TriggerEvent>* p_collidsion_events)
		{
			return ColliderDetector{ *p_collidsion_events };
		};
	};

	struct CollisionHeap2
	{
		using ColliderDetector = Collision2::ColliderDetector;
		using TriggerEvent = Collision2::TriggerEvent;

		v2::PoolIndexed<BoxCollider> box_colliders;
		v2::Pool<Token<ColliderDetector>> box_colliders_to_collider_detector;
		v2::PoolIndexed<ColliderDetector> collider_detectors;
		v2::PoolOfVector<TriggerEvent> collider_detectors_events_2;

		inline static CollisionHeap2 allocate_default()
		{
			return CollisionHeap2{
				v2::PoolIndexed<BoxCollider>::allocate_default(),
				v2::Pool<Token<ColliderDetector>>::allocate(0),
				v2::PoolIndexed<ColliderDetector>::allocate_default(),
				v2::PoolOfVector<TriggerEvent>::allocate_default()
			};
		};

		inline void free()
		{
			this->box_colliders.free();
			this->box_colliders_to_collider_detector.free();
			this->collider_detectors.free();
			this->collider_detectors_events_2.free();
		};

		inline Token(ColliderDetector) allocate_colliderdetector(const Token<BoxCollider>* p_box_collider)
		{

#if COLLIDER_BOUND_TEST
			//Cannot attach multiple collider detector to a collider for now
			assert_true(!this->does_boxcollider_have_colliderdetector(p_box_collider));
#endif

			v2::PoolOfVectorToken<TriggerEvent> l_trigger_events = this->collider_detectors_events_2.alloc_vector();
			ColliderDetector l_collider_detector = ColliderDetector::build(&l_trigger_events);
			Token(ColliderDetector) l_collider_detector_token = this->collider_detectors.alloc_element(&l_collider_detector);
			*this->get_colliderdetector_from_boxcollider(p_box_collider) = l_collider_detector_token;
			return l_collider_detector_token;
		};

		inline void free_colliderdetector(const Token<BoxCollider>* p_box_collider, const Token<ColliderDetector>* p_collider_detector)
		{

			{
				ColliderDetector* l_collider_detector = this->collider_detectors.get(p_collider_detector);
				this->collider_detectors_events_2.release_vector(&l_collider_detector->collision_events);
				this->collider_detectors.release_element(p_collider_detector);
			}

			{
				Token<ColliderDetector>* l_collider_detector_token = this->get_colliderdetector_from_boxcollider(p_box_collider);
				*l_collider_detector_token = token_build_default<ColliderDetector>();
				this->box_colliders_to_collider_detector.release_element(token_cast_p(Token(ColliderDetector), p_box_collider));
			}

		};

		inline Token(BoxCollider) allocate_boxcollider(const BoxCollider* p_box_collider)
		{
			Token(BoxCollider) l_box_collider_index = this->box_colliders.alloc_element(p_box_collider);
			this->box_colliders_to_collider_detector.alloc_element_1v(token_build_default<ColliderDetector>());
			return l_box_collider_index;
		};

		inline Token(BoxCollider) allocate_boxcollider(const BoxCollider p_box_collider)
		{
			return allocate_boxcollider(&p_box_collider);
		};

		inline void push_boxcollider_transform(Token<BoxCollider>* p_boxcollider, const Math::Transform* p_world_transform, const Math::quat* p_local_rotation)
		{
			BoxCollider* l_boxcollider = this->box_colliders.get(p_boxcollider);
			l_boxcollider->transform = *p_world_transform;
			l_boxcollider->rotation_axis = Math::extractAxis<float>(p_world_transform->rotation);
		};

		inline void free_boxcollider(const Token<BoxCollider>* p_box_collider)
		{
			Token<ColliderDetector>* l_collider_detector = this->get_colliderdetector_from_boxcollider(p_box_collider);
			if (l_collider_detector->tok != -1)
			{
				this->free_colliderdetector(p_box_collider, l_collider_detector);
			}
			this->box_colliders.release_element(p_box_collider);
		};

		inline Token(ColliderDetector)* get_colliderdetector_from_boxcollider(const Token(BoxCollider)* p_box_collider)
		{
			return this->box_colliders_to_collider_detector.get(token_cast_p(Token<ColliderDetector>, p_box_collider));
		};

		inline Slice<TriggerEvent> get_triggerevents_from_boxcollider(const Token(BoxCollider)* p_box_collider)
		{
			if (this->does_boxcollider_have_colliderdetector(p_box_collider))
			{
				Token(ColliderDetector)* l_collider_detextor = this->get_colliderdetector_from_boxcollider(p_box_collider);
				return this->collider_detectors_events_2.get_vector(&this->collider_detectors.get(l_collider_detextor)->collision_events).Memory;
			}
			return Slice<TriggerEvent>::build_default();
		};

		inline Slice<TriggerEvent> get_triggerevents_from_colliderdetector(const Token(ColliderDetector)* p_collider_detector)
		{
			return this->collider_detectors_events_2.get_vector(&this->collider_detectors.get(p_collider_detector)->collision_events).Memory;
		};

		inline char does_boxcollider_have_colliderdetector(const Token(BoxCollider)* p_box_collider)
		{
			if (!this->box_colliders_to_collider_detector.is_element_free(token_cast_p(Token<ColliderDetector>, p_box_collider)))
			{
				if (this->get_colliderdetector_from_boxcollider(p_box_collider)->tok != -1)
				{
					return 1;
				}
			};
			return 0;
		};

	};

	struct CollisionDetectionStep
	{
		using ColliderDetector = Collision2::ColliderDetector;
		using TriggerEvent = Collision2::TriggerEvent;

		/*
			An IntersectionEvent is an internal structure of the CollisionDetectionStep.
			It is the output generated when handling processed Colliders (that cna come from either a deletion or an entry in in_colliders_processed).
			The event indicates that something has happened between a collision detector and another collider.
		*/
		struct IntersectionEvent
		{
			Token<ColliderDetector> detector;
			Token<BoxCollider> other;

			inline static IntersectionEvent build(const Token<ColliderDetector>* p_detector, const Token<BoxCollider>* p_other)
			{
				return IntersectionEvent{ *p_detector, *p_other };
			};

			inline char equals_intersectionevent(const IntersectionEvent* p_other)
			{
				return (this->detector.tok == p_other->detector.tok) && (this->other.tok == p_other->other.tok);
			};
		};


		Collision2::CollisionHeap2* heap;
		v2::Vector<Token(BoxCollider)> in_colliders_processed;

		v2::Vector<Token<BoxCollider>> deleted_colliders_from_last_step;

		v2::Vector<IntersectionEvent> currentstep_enter_intersection_events;
		v2::Vector<IntersectionEvent> currentstep_exit_intersection_events;

		v2::Vector<IntersectionEvent> is_waitingfor_trigger_stay_detector;
		v2::Vector<IntersectionEvent> is_waitingfor_trigger_none_detector;

		v2::Vector<IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;
		v2::Vector<IntersectionEvent> is_waitingfor_trigger_none_nextframe_detector;

		inline static CollisionDetectionStep allocate(Collision2::CollisionHeap2* p_heap)
		{
			return CollisionDetectionStep{
				p_heap,
				v2::Vector<Token(BoxCollider)>::allocate(0),
				v2::Vector<Token(BoxCollider)>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0),
				v2::Vector<IntersectionEvent>::allocate(0)
			};
		};

		inline void free()
		{
			this->in_colliders_processed.free();
			this->deleted_colliders_from_last_step.free();
			this->currentstep_enter_intersection_events.free();
			this->currentstep_exit_intersection_events.free();
			this->is_waitingfor_trigger_stay_detector.free();
			this->is_waitingfor_trigger_stay_nextframe_detector.free();
			this->is_waitingfor_trigger_none_detector.free();
			this->is_waitingfor_trigger_none_nextframe_detector.free();
		};

		inline void step()
		{
			this->swap_detector_events();

			this->process_deleted_colliders();
			this->process_input_colliders();

			this->remove_current_step_event_duplicates();

			this->update_pending_detectors();
			this->process_current_step_events();

			this->clear_current_step_events();
		};

		inline void push_collider_for_process(const Token<BoxCollider>* p_moved_collider)
		{
			this->in_colliders_processed.push_back_element(p_moved_collider);
		};

		inline void push_collider_for_deletion(const Token<BoxCollider>* p_collider)
		{
			this->deleted_colliders_from_last_step.push_back_element(p_collider);
		};

	private:

		inline void swap_detector_events()
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
		inline void enter_collision(const IntersectionEvent* p_intersection_event)
		{
			v2::PoolOfVectorToken<TriggerEvent> l_collider_triggerevents_nestedvector = this->heap->collider_detectors.get(&p_intersection_event->detector)->collision_events;
			v2::VectorOfVector_Element<TriggerEvent> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(&l_collider_triggerevents_nestedvector);
			bool l_trigger_event_found = false;
			for (loop(i, 0, l_collider_triggerevents.Header.Size))
			{
				TriggerEvent* l_collider_trigger_event = l_collider_triggerevents.Memory.get(i);
				if (l_collider_trigger_event->other.tok == p_intersection_event->other.tok)
				{
					l_collider_trigger_event->state = Trigger::State::TRIGGER_STAY;
					l_trigger_event_found = true;
					break;
				}
			}

			if (!l_trigger_event_found)
			{
				this->heap->collider_detectors_events_2.element_push_back_element_2v(
					&l_collider_triggerevents_nestedvector,
					TriggerEvent::build(&p_intersection_event->other, Trigger::State::TRIGGER_ENTER)
				);

				this->is_waitingfor_trigger_stay_nextframe_detector.push_back_element(p_intersection_event);
			}
		};

		//We get all ColliderDetector associated to the p_source_collider and check if they have an active state with the involved collider
		//if that's the case, then we invalidate the collision
		inline void exit_collision(const IntersectionEvent* p_intersection_event)
		{
			v2::VectorOfVector_Element<TriggerEvent> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(
				&this->heap->collider_detectors.get(&p_intersection_event->detector)->collision_events
			);

			for (loop(i, 0, l_collider_triggerevents.Header.Size))
			{
				TriggerEvent* l_trigger_event = l_collider_triggerevents.Memory.get(i);
				if (l_trigger_event->other.tok == p_intersection_event->other.tok)
				{
					l_trigger_event->state = Trigger::State::TRIGGER_EXIT;
					// on next step, collision event will be deleted
					this->is_waitingfor_trigger_none_nextframe_detector.push_back_element(p_intersection_event);
				}
			}
		};

		/*
			When a collider is deleted :
				-> All pending processing data that refers to the deleted collider are deleted
				-> All ColliderDetectors are notified that the collider is no more intersecting
		*/
		inline void process_deleted_colliders()
		{
			if (this->deleted_colliders_from_last_step.Size > 0)
			{
				for (vector_loop(&this->deleted_colliders_from_last_step, i))
				{
					Token(BoxCollider)* l_deleted_collider = this->deleted_colliders_from_last_step.get(i);

					vector_erase_if_2_begin(&this->in_colliders_processed, j, l_collider)
						char l_erased = l_collider->tok == l_deleted_collider->tok;
					vector_erase_if_2_end(&this->in_colliders_processed, j, l_erased);

					vector_erase_if_2_begin(&this->is_waitingfor_trigger_stay_detector, j, l_intsersrection_event)
						char l_erased = l_intsersrection_event->other.tok == l_deleted_collider->tok;
					vector_erase_if_2_end(&this->is_waitingfor_trigger_stay_detector, j, l_erased);

					vector_erase_if_2_begin(&this->is_waitingfor_trigger_none_detector, j, l_trigger_event)
						char l_erased = l_trigger_event->other.tok == l_deleted_collider->tok;
					vector_erase_if_2_end(&this->is_waitingfor_trigger_none_detector, j, l_erased);

					//Simulate a collision exit (because the collider has just been deleted) Notify other Trigger events
					//TODO -> In the future, we want to partition the space to not notify the entire world
					poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_box_collider)
					{
						Token(ColliderDetector)* l_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_box_collider);
						if (l_collider_detector->tok != -1)
						{
							this->currentstep_exit_intersection_events.push_back_element_1v(IntersectionEvent::build(l_collider_detector, l_deleted_collider));
						}
					}
					poolindexed_foreach_token_2_end()
				}

				this->deleted_colliders_from_last_step.clear();
			}
		};

		inline void process_input_colliders()
		{
			for (loop(i, 0, this->in_colliders_processed.Size))
			{
				Token(BoxCollider)* l_left_collider_token = this->in_colliders_processed.get(i);
				Token(ColliderDetector)* l_left_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_left_collider_token);

				// If the processed collider have a collider detector, we calculate intersection with other BoxColliders
				// then push collision_event according to the intersection result
				if (l_left_collider_detector->tok != -1)
				{
					BoxCollider* l_left_collider = this->heap->box_colliders.get(l_left_collider_token);
					Math::OBB<float>l_left_projected = Geometry::to_obb(l_left_collider->local_box, l_left_collider->transform, l_left_collider->rotation_axis);

					//TODO -> In the future, we want to avoid to query the world
					poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_right_collider_token);
					
					//Avoid self test
					if (l_left_collider_token->tok != l_right_collider_token->tok)
					{
						BoxCollider* l_right_collider = this->heap->box_colliders.get(l_right_collider_token);
						Math::OBB<float>l_right_projected = Geometry::to_obb(l_right_collider->local_box, l_right_collider->transform, l_right_collider->rotation_axis);

						if (Geometry::overlap3(l_left_projected, l_right_projected))
						{
							this->currentstep_enter_intersection_events.push_back_element_1v(
								IntersectionEvent::build(
									l_left_collider_detector,
									l_right_collider_token
								)
							);

							Token<ColliderDetector>* l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
							if (l_right_collider_detector->tok != -1)
							{
								this->currentstep_enter_intersection_events.push_back_element_1v(
									IntersectionEvent::build(
										l_right_collider_detector,
										l_left_collider_token
									)
								);
							}
						}
						else
						{
							this->currentstep_exit_intersection_events.push_back_element_1v(
								IntersectionEvent::build(
									l_left_collider_detector,
									l_right_collider_token
								)
							);

							Token<ColliderDetector>* l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
							if (l_right_collider_detector->tok != -1)
							{
								this->currentstep_exit_intersection_events.push_back_element_1v(
									IntersectionEvent::build(
										l_right_collider_detector,
										l_left_collider_token
									)
								);
							}
						}
					}
					poolindexed_foreach_token_2_end();
				}
				// If the processed collider doesn't have a collider detector, we get all other collider detectors and test collision
				// then push collision_event according to the intersection result
				else
				{
					BoxCollider* l_left_collider = this->heap->box_colliders.get(l_left_collider_token);
					Math::OBB<float>l_left_projected = Geometry::to_obb(l_left_collider->local_box, l_left_collider->transform, l_left_collider->rotation_axis);

					//TODO -> In the future, we want to avoid to query the world
					poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_right_collider_token)
					{
						BoxCollider* l_right_collider = this->heap->box_colliders.get(l_right_collider_token);
						Token(ColliderDetector)* l_right_collider_detector = this->heap->get_colliderdetector_from_boxcollider(l_right_collider_token);
						if (l_right_collider_detector->tok != -1)
						{
							Math::OBB<float>l_right_projected = Geometry::to_obb(l_right_collider->local_box, l_right_collider->transform, l_right_collider->rotation_axis);

							if (Geometry::overlap3(l_left_projected, l_right_projected))
							{
								this->currentstep_enter_intersection_events.push_back_element_1v(
									IntersectionEvent::build(l_right_collider_detector, l_left_collider_token)
								);
							}
							else
							{
								this->currentstep_exit_intersection_events.push_back_element_1v(
									IntersectionEvent::build(l_right_collider_detector, l_left_collider_token)
								);
							}
						}
					}
					poolindexed_foreach_token_2_end()
				}
			}

			this->in_colliders_processed.clear();
		};

		inline void remove_intersectionevents_duplicate(v2::Vector<IntersectionEvent>* in_out_intersection_events)
		{
			for (loop(i, 0, in_out_intersection_events->Size))
			{
				IntersectionEvent* l_left_intersection_event = in_out_intersection_events->get(i);
				for (loop(j, i + 1, in_out_intersection_events->Size))
				{
					IntersectionEvent* l_right_intersection_event = in_out_intersection_events->get(j);

					if (l_left_intersection_event->equals_intersectionevent(l_right_intersection_event))
					{
						in_out_intersection_events->erase_element_at(j);
					}
				}
			}
		};

		/*
			Previous step may push the same intersection events. Because a collider detector and it's related box collider may have moved, so they both generate the same intersection event.
		*/
		inline void remove_current_step_event_duplicates()
		{
			this->remove_intersectionevents_duplicate(&this->currentstep_enter_intersection_events);
			this->remove_intersectionevents_duplicate(&this->currentstep_exit_intersection_events);
		};

		inline void process_current_step_events()
		{
			vector_foreach_begin(&this->currentstep_enter_intersection_events, i, l_intersection_event);
			this->enter_collision(l_intersection_event);
			vector_foreach_end();

			vector_foreach_begin(&this->currentstep_exit_intersection_events, i, l_intersection_event);
			this->exit_collision(l_intersection_event);
			vector_foreach_end();
		};

		inline void clear_current_step_events()
		{
			this->currentstep_enter_intersection_events.clear();
			this->currentstep_exit_intersection_events.clear();
		};

		inline void set_triggerstate_matchingWith_boxcollider(Token<ColliderDetector>* p_collision_detector, const Token(BoxCollider)* p_matched_boxcollider, const Trigger::State p_trigger_state)
		{
			ColliderDetector* l_collider_detector = this->heap->collider_detectors.get(p_collision_detector);
			v2::VectorOfVector_Element<TriggerEvent> l_events = this->heap->collider_detectors_events_2.get_vector(&l_collider_detector->collision_events);
			for (loop(i, 0, l_events.Header.Size))
			{
				TriggerEvent* l_trigger_event = l_events.Memory.get(i);
				if (l_trigger_event->other.tok == p_matched_boxcollider->tok)
				{
					l_trigger_event->state = p_trigger_state;
				}
			}
		};

		inline void update_pending_detectors()
		{
			for (vector_loop(&this->is_waitingfor_trigger_stay_detector, i))
			{
				IntersectionEvent* l_intersection_event = this->is_waitingfor_trigger_stay_detector.get(i);
				this->set_triggerstate_matchingWith_boxcollider(&l_intersection_event->detector, &l_intersection_event->other, Trigger::State::TRIGGER_STAY);
			}

			for (vector_loop(&this->is_waitingfor_trigger_none_detector, i))
			{
				IntersectionEvent* l_intersection_event = this->is_waitingfor_trigger_none_detector.get(i);
				this->set_triggerstate_matchingWith_boxcollider(&l_intersection_event->detector, &l_intersection_event->other, Trigger::State::NONE);
			}

			this->is_waitingfor_trigger_stay_detector.clear();
			this->is_waitingfor_trigger_none_detector.clear();
		};


	};

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
		inline static Token<BoxCollider> allocate_boxcollider(Collision2* thiz, const Math::AABB<float>* p_local_box)
		{
			return thiz->collision_heap.allocate_boxcollider(BoxCollider::build_from_local_aabb(p_local_box));
		};

		inline static void on_collider_moved(Collision2* thiz, Token<BoxCollider>* p_moved_collider, const Math::Transform* p_world_transform, const Math::quat* p_local_rotation)
		{
			thiz->collision_heap.push_boxcollider_transform(p_moved_collider, p_world_transform, p_local_rotation);
			thiz->collision_detection_step.push_collider_for_process(p_moved_collider);
		};

		inline static void free_collider(Collision2* thiz, Token<BoxCollider>* p_moved_collider)
		{
			thiz->collision_heap.free_boxcollider(p_moved_collider);
			thiz->collision_detection_step.push_collider_for_deletion(p_moved_collider);
		};

		
		inline static Token<ColliderDetector> allocate_colliderdetector(Collision2* thiz, const Token<BoxCollider>* p_box_collider)
		{
			return thiz->collision_heap.allocate_colliderdetector(p_box_collider);
		};

		inline static void free_colliderdetector(Collision2* thiz, Token<BoxCollider>* p_collider, Token<ColliderDetector>* p_collider_detector)
		{
			thiz->collision_heap.free_colliderdetector(p_collider, p_collider_detector);
		};
	};
};

#include "./collision_objects.hpp"
