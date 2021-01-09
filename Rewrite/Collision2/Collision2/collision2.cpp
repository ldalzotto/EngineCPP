
#include "Common2/common2.hpp"
#include "Collision2/collision2.hpp"
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
	struct TriggerState
	{
		Token<BoxCollider> other;
		Trigger::State state;

		inline static TriggerState build_default()
		{
			return TriggerState{ token_build_default<BoxCollider>(), Trigger::State::UNDEFINED };
		};

		inline static TriggerState build(const Token<BoxCollider>* p_other, const Trigger::State p_state)
		{
			return TriggerState{ *p_other, p_state };
		};
	};

	/*
		A ColliderDetector indicates that a collision shape is emitting TriggerState collision events.
	*/
	struct ColliderDetector
	{
		PoolOfVectorToken<TriggerState> collision_events;

		inline static ColliderDetector build(const PoolOfVectorToken<TriggerState>* p_collidsion_events)
		{
			return ColliderDetector{ *p_collidsion_events };
		};
	};

	struct CollisionHeap2
	{
		using ColliderDetector = Collision2::ColliderDetector;
		using TriggerState = Collision2::TriggerState;

		PoolIndexed<BoxCollider> box_colliders;
		Pool<Token<ColliderDetector>> box_colliders_to_collider_detector;
		PoolIndexed<ColliderDetector> collider_detectors;
		PoolOfVector<TriggerState> collider_detectors_events_2;

		inline static CollisionHeap2 allocate_default()
		{
			return CollisionHeap2{
				PoolIndexed<BoxCollider>::allocate_default(),
				Pool<Token<ColliderDetector>>::allocate(0),
				PoolIndexed<ColliderDetector>::allocate_default(),
				PoolOfVector<TriggerState>::allocate_default()
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

			PoolOfVectorToken<TriggerState> l_trigger_events = this->collider_detectors_events_2.alloc_vector();
			ColliderDetector l_collider_detector = ColliderDetector::build(&l_trigger_events);
			Token(ColliderDetector) l_collider_detector_token = this->collider_detectors.alloc_element(&l_collider_detector);
			*this->get_colliderdetector_from_boxcollider(p_box_collider) = l_collider_detector_token;
			return l_collider_detector_token;
		};

		inline void free_colliderdetector(const Token<BoxCollider>* p_box_collider, const Token<ColliderDetector>* p_collider_detector)
		{
			{
				Token<ColliderDetector>* l_collider_detector_token = this->get_colliderdetector_from_boxcollider(p_box_collider);
				*l_collider_detector_token = token_build_default<ColliderDetector>();
				this->box_colliders_to_collider_detector.release_element(token_cast_p(Token(ColliderDetector), p_box_collider));
			}

			{
				ColliderDetector* l_collider_detector = this->collider_detectors.get(p_collider_detector);
				this->collider_detectors_events_2.release_vector(&l_collider_detector->collision_events);
				this->collider_detectors.release_element(p_collider_detector);
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

		inline Slice<TriggerState> get_triggerevents_from_boxcollider(const Token(BoxCollider)* p_box_collider)
		{
			if (this->does_boxcollider_have_colliderdetector(p_box_collider))
			{
				Token(ColliderDetector)* l_collider_detextor = this->get_colliderdetector_from_boxcollider(p_box_collider);
				return this->collider_detectors_events_2.get_vector(&this->collider_detectors.get(l_collider_detextor)->collision_events).Memory;
			}
			return Slice<TriggerState>::build_default();
		};

		inline Slice<TriggerState> get_triggerevents_from_colliderdetector(const Token(ColliderDetector)* p_collider_detector)
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
		using TriggerState = Collision2::TriggerState;

		struct IntersectionEvent
		{
			Token<ColliderDetector> detector;
			Token<BoxCollider> source;
			Token<BoxCollider> other;

			inline static IntersectionEvent build(const Token<ColliderDetector>* p_detector, const Token<BoxCollider>* p_source, const Token<BoxCollider>* p_other)
			{
				return IntersectionEvent{ *p_detector, *p_source, *p_other };
			};
		};

		struct TriggerEvent
		{
			Token<ColliderDetector> detector;
			Token<BoxCollider> other;

			inline TriggerEvent static build(const Token<ColliderDetector>* p_detector, const Token<BoxCollider>* p_other)
			{
				return TriggerEvent{ *p_detector, *p_other };
			};
		};


		Collision2::CollisionHeap2* heap;
		Vector<Token(BoxCollider)> in_colliders_processed;

		Vector<Token<BoxCollider>> deleted_colliders_from_last_step;

		Vector<IntersectionEvent> is_waitingfor_trigger_stay_detector;
		Vector<IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;

		Vector<TriggerEvent> is_waitingfor_trigger_none_detector;
		Vector<TriggerEvent> is_waitingfor_trigger_none_nextframe_detector;

		Vector<Token<ColliderDetector>> lastframe_involved_colliderdetectors;

		inline static CollisionDetectionStep allocate(Collision2::CollisionHeap2* p_heap)
		{
			return CollisionDetectionStep{
				p_heap,
				Vector<Token(BoxCollider)>::allocate(0),
				Vector<Token(BoxCollider)>::allocate(0),
				Vector<IntersectionEvent>::allocate(0),
				Vector<IntersectionEvent>::allocate(0),
				Vector<TriggerEvent>::allocate(0),
				Vector<TriggerEvent>::allocate(0),
				Vector<Token<ColliderDetector>>::allocate(0)
			};
		};

		inline void free()
		{
			this->in_colliders_processed.free();
			this->deleted_colliders_from_last_step.free();
			this->is_waitingfor_trigger_stay_detector.free();
			this->is_waitingfor_trigger_stay_nextframe_detector.free();
			this->is_waitingfor_trigger_none_detector.free();
			this->is_waitingfor_trigger_none_nextframe_detector.free();
			this->lastframe_involved_colliderdetectors.free();
		};

		inline void step()
		{
			this->clear_lastframe_collision_events();
			this->swap_triggerstaw_waiting_buffers();
			this->swap_triggernone_waiting_buffers();
			this->clean_deleted_colliders();

			this->process_input_colliders();

			this->update_pending_detectors();
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

		inline void clear_lastframe_collision_events()
		{
			this->lastframe_involved_colliderdetectors.clear();
		};

		inline void swap_triggerstaw_waiting_buffers()
		{
			Vector<IntersectionEvent> l_tmp = this->is_waitingfor_trigger_stay_detector;
			this->is_waitingfor_trigger_stay_detector = this->is_waitingfor_trigger_stay_nextframe_detector;
			this->is_waitingfor_trigger_stay_nextframe_detector = l_tmp;
		};

		inline void swap_triggernone_waiting_buffers()
		{
			Vector<TriggerEvent> l_tmp = this->is_waitingfor_trigger_none_detector;
			this->is_waitingfor_trigger_none_detector = this->is_waitingfor_trigger_none_nextframe_detector;
			this->is_waitingfor_trigger_none_nextframe_detector = l_tmp;
		};

		// When an intersection from the source collider to target occurs
		// If there is already a TriggerState event between then, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
		inline void enter_collision(const Token(BoxCollider)* p_source_collider, const Token(BoxCollider)* p_intersected_collider)
		{
			Token<ColliderDetector>* l_source_collider_detector = this->heap->get_colliderdetector_from_boxcollider(p_source_collider);
			if (l_source_collider_detector->tok != -1)
			{
				this->lastframe_involved_colliderdetectors.push_back_element(l_source_collider_detector);

				PoolOfVectorToken<TriggerState> l_collider_triggerevents_nestedvector = this->heap->collider_detectors.get(l_source_collider_detector)->collision_events;
				VectorOfVector_Element<TriggerState> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(&l_collider_triggerevents_nestedvector);
				bool l_trigger_event_found = false;
				for (loop(i, 0, l_collider_triggerevents.Header.Size))
				{
					TriggerState* l_collider_trigger_event = l_collider_triggerevents.Memory.get(i);
					if (l_collider_trigger_event->other.tok == p_intersected_collider->tok)
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
						TriggerState::build(p_intersected_collider, Trigger::State::TRIGGER_ENTER)
					);

					this->is_waitingfor_trigger_stay_detector.push_back_element_1v(
						IntersectionEvent::build(l_source_collider_detector, p_source_collider, p_intersected_collider)
					);
				}
			}
		};

		//We get all ColliderDetector and check if they have an active state with the deleted collider
		//if that's the case, then we invalidate the collision
		inline void exit_collision(const Token(BoxCollider)* p_source_collider, const Token(BoxCollider)* p_intersected_collider)
		{
			Token<ColliderDetector>* l_source_collider_detector = this->heap->get_colliderdetector_from_boxcollider(p_source_collider);
			if (l_source_collider_detector->tok != -1)
			{
				VectorOfVector_Element<TriggerState> l_collider_triggerevents = this->heap->collider_detectors_events_2.get_vector(
					&this->heap->collider_detectors.get(l_source_collider_detector)->collision_events
				);

				for (loop(i, 0, l_collider_triggerevents.Header.Size))
				{
					TriggerState* l_trigger_event = l_collider_triggerevents.Memory.get(i);
					if (l_trigger_event->other.tok == p_intersected_collider->tok)
					{
						l_trigger_event->state = Trigger::State::TRIGGER_EXIT;
						// next step, collision event will be deleted
						this->is_waitingfor_trigger_none_detector.push_back_element_1v(TriggerEvent::build(l_source_collider_detector, p_intersected_collider));
					}
				}
			}
		};

		inline void clean_deleted_colliders()
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
					vector_erase_if_2_end(&this->is_waitingfor_trigger_stay_detector, j, l_erased);
					//Notify other Trigger events

					poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_box_collider)
						this->exit_collision(l_box_collider, l_deleted_collider);
					poolindexed_foreach_token_2_end()
				}

				this->deleted_colliders_from_last_step.clear();
			}
		};

		inline void on_collision_detected(const Token<BoxCollider>* p_left, const Token<BoxCollider>* p_right)
		{
			this->enter_collision(p_left, p_right);
			this->enter_collision(p_right, p_left);
		};

		inline void on_collision_detection_failed(const Token<BoxCollider>* p_left, const Token<BoxCollider>* p_right)
		{
			this->exit_collision(p_left, p_right);
			this->exit_collision(p_right, p_left);
		};

		inline void process_input_colliders()
		{
			for (loop(i, 0, this->in_colliders_processed.Size))
			{
				Token(BoxCollider)* l_left_collider_token = this->in_colliders_processed.get(i);

				if (this->heap->does_boxcollider_have_colliderdetector(l_left_collider_token))
				{
					BoxCollider* l_left_collider = this->heap->box_colliders.get(l_left_collider_token);
					Math::OBB<float>l_left_projected = Geometry::to_obb(l_left_collider->local_box, l_left_collider->transform, l_left_collider->rotation_axis);

					poolindexed_foreach_token_2_begin(&this->heap->box_colliders, j, l_right_collider_token);
					//TODO -> FIX - we also need to avoid calculatin the same collision between two Colliders (in the case where multiple colliders in this->in_colliders_processed are colliding between each other)
					//		  because the on_collision_detected and on_collision_detection_failed operations are bidirectional.
					//		  This bug presents the TRIGGER_ENTER event to be triggered.

					//Avoid self test
					if (l_left_collider_token->tok != l_right_collider_token->tok)
					{
						BoxCollider* l_right_collider = this->heap->box_colliders.get(l_right_collider_token);
						Math::OBB<float>l_right_projected = Geometry::to_obb(l_right_collider->local_box, l_right_collider->transform, l_right_collider->rotation_axis);

						if (Geometry::overlap3(l_left_projected, l_right_projected))
						{
							this->on_collision_detected(l_left_collider_token, l_right_collider_token);
						}
						else
						{
							this->on_collision_detection_failed(l_left_collider_token, l_right_collider_token);
						}
					}
					poolindexed_foreach_token_2_end();
				};
			}

			this->in_colliders_processed.clear();
		};

		inline void set_triggerstate_matchingWith_boxcollider(Token<ColliderDetector>* p_collision_detector, const Token(BoxCollider)* p_matched_boxcollider, const Trigger::State p_trigger_state)
		{
			ColliderDetector* l_collider_detector = this->heap->collider_detectors.get(p_collision_detector);
			VectorOfVector_Element<TriggerState> l_events = this->heap->collider_detectors_events_2.get_vector(&l_collider_detector->collision_events);
			for (loop(i, 0, l_events.Header.Size))
			{
				TriggerState* l_trigger_event = l_events.Memory.get(i);
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
				IntersectionEvent* l_intersection_event = this->is_waitingfor_trigger_stay_detector.get(i);
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

	inline void update()
	{
		this->collision_detection_step.step();
	};


	inline Token<BoxCollider> composition_allocate_boxcollider(const Math::AABB<float>* p_local_box)
	{
		return this->collision_heap.allocate_boxcollider(BoxCollider::build_from_local_aabb(p_local_box));
	};

	inline void composition_on_collider_moved(Token<BoxCollider>* p_moved_collider, const Math::Transform* p_world_transform, const Math::quat* p_local_rotation)
	{
		this->collision_heap.push_boxcollider_transform(p_moved_collider, p_world_transform, p_local_rotation);
		this->collision_detection_step.push_collider_for_process(p_moved_collider);
	};

	inline void composion_free_collider(Token<BoxCollider>* p_moved_collider)
	{
		this->collision_heap.free_boxcollider(p_moved_collider);
		this->collision_detection_step.push_collider_for_deletion(p_moved_collider);
	};
};

#include "./collision2_objects.hpp"