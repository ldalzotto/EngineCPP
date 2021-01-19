#pragma once


/*
	Collision Shape used for intersection calculation
*/
struct BoxCollider
{
	int8 enabled;
	transform_pa transform;
	aabb local_box;

	static BoxCollider build_from_local_aabb(const int8 p_enabled, const aabb& p_local_box);;
};

/*
		State of the trigger intersection between owner BoxCollider and the other.
	*/
struct TriggerEvent
{
	Token(BoxCollider) other;
	Trigger::State state;

	static TriggerEvent build_default();
	static TriggerEvent build(const Token(BoxCollider) p_other, const Trigger::State p_state);
};

/*
	A ColliderDetector indicates that a collision shape is emitting TriggerEvent collision events.
*/
struct ColliderDetector
{
	v2::PoolOfVectorToken<TriggerEvent> collision_events;

	static ColliderDetector build(const v2::PoolOfVectorToken<TriggerEvent> p_collidsion_events);
};

struct CollisionHeap2
{
	v2::PoolIndexed<BoxCollider> box_colliders;
	v2::Pool<Token(ColliderDetector)> box_colliders_to_collider_detector;
	v2::PoolIndexed<ColliderDetector> collider_detectors;
	v2::PoolOfVector<TriggerEvent> collider_detectors_events_2;

	static CollisionHeap2 allocate_default();
	void free();

	Token(ColliderDetector) allocate_colliderdetector(const Token(BoxCollider) p_box_collider);
	void free_colliderdetector(const Token(BoxCollider) p_box_collider, const Token(ColliderDetector) p_collider_detector);

	Token(BoxCollider) allocate_boxcollider(const BoxCollider& p_box_collider);
	void push_boxcollider_transform(Token(BoxCollider) p_boxcollider, const transform_pa& p_world_transform);
	void free_boxcollider(const Token(BoxCollider) p_box_collider);

	Token(ColliderDetector)& get_colliderdetector_from_boxcollider(const Token(BoxCollider) p_box_collider);

	Slice<TriggerEvent> get_triggerevents_from_boxcollider(const Token(BoxCollider) p_box_collider);
	Slice<TriggerEvent> get_triggerevents_from_colliderdetector(const Token(ColliderDetector) p_collider_detector);

	int8 does_boxcollider_have_colliderdetector(const Token(BoxCollider) p_box_collider);
};


struct CollisionDetectionStep
{
	/*
		An IntersectionEvent is an internal structure of the CollisionDetectionStep.
		It is the output generated when handling processed Colliders (that can come from either a deletion or an entry in in_colliders_processed).
		An intersectin event is unidirectional. Meaning that intersection is true only from the point of view of the ColliderDetector.
		It can either be an "enter" or "exit" collision.
	*/
	struct IntersectionEvent
	{
		Token(ColliderDetector) detector;
		Token(BoxCollider) other;

		inline static IntersectionEvent build(const Token(ColliderDetector) p_detector, const Token(BoxCollider) p_other);
		inline int8 equals_intersectionevent(const IntersectionEvent& p_other);
	};

	struct CollisionDetectorDeletionEvent
	{
		Token(BoxCollider) collider;
		Token(ColliderDetector) detector;

		inline static CollisionDetectorDeletionEvent build(const Token(BoxCollider) p_box_collider, const Token(ColliderDetector) p_collider_detector);
	};

	CollisionHeap2* heap;
	v2::Vector<Token(BoxCollider)> in_colliders_disabled;
	v2::Vector<Token(BoxCollider)> in_colliders_processed;

	v2::Vector<Token(BoxCollider)> deleted_colliders_from_last_step;
	v2::Vector<CollisionDetectorDeletionEvent> deleted_collider_detectors_from_last_step;

	v2::Vector<IntersectionEvent> currentstep_enter_intersection_events;
	v2::Vector<IntersectionEvent> currentstep_exit_intersection_events;

	v2::Vector<IntersectionEvent> is_waitingfor_trigger_stay_detector;
	v2::Vector<IntersectionEvent> is_waitingfor_trigger_none_detector;

	v2::Vector<IntersectionEvent> is_waitingfor_trigger_stay_nextframe_detector;
	v2::Vector<IntersectionEvent> is_waitingfor_trigger_none_nextframe_detector;

	inline static CollisionDetectionStep allocate(CollisionHeap2* p_heap);
	inline void free();

	/* A frame of the Collision engine. */
	inline void step();

	inline void push_collider_for_process(const Token(BoxCollider) p_moved_collider);
	inline void push_collider_for_deletion(const Token(BoxCollider) p_collider);
	inline void push_collider_detector_for_deletion(const Token(BoxCollider) p_collider, const Token(ColliderDetector) p_detector);

private:

	inline void step_freeingresource_only();

	inline void swap_detector_events();

	// When an intersection from the source collider to target occurs
	// If there is already a TriggerEvent event between them, we set to TRIGGER_STAY else, we initialize to TRIGGER_ENTER
	inline void enter_collision(const IntersectionEvent& p_intersection_event);

	//We get all ColliderDetector associated to the p_source_collider and check if they have an active state with the involved collider
	//if that's the case, then we invalidate the collision
	inline void exit_collision(const IntersectionEvent& p_intersection_event);

	inline void remove_references_to_colliderdetector(const Token(ColliderDetector) p_collider_detector);

	// /!\ Do not take care of the associated ColliderDetectors.
	inline void remove_references_to_boxcollider(const Token(BoxCollider) p_box_collider);

	// Norify all ColliderDetectors with an exit_collision event.
	//TODO -> In the future, we want to partition the space to not notify the entire world
	inline void generate_exit_collision_for_collider(const Token(BoxCollider) p_box_collider);

	inline void process_deleted_collider_detectors();


	/*
		When a collider is deleted :
			-> All associated ColliderDetectors and their references are freed.
			-> All pending processing data that refers to the deleted collider are deleted
			-> All ColliderDetectors are notified that the collider is no more intersecting
	*/
	inline void process_deleted_colliders();
	inline void process_input_colliders();

	inline void remove_intersectionevents_duplicate(v2::Vector<IntersectionEvent>* in_out_intersection_events);

	/*
		Previous step may push the same intersection events. Because a collider detector and it's related box collider may have moved, so they both generate the same intersection event.
	*/
	inline void remove_current_step_event_duplicates();
	inline void udpate_triggerstate_from_intersectionevents();
	inline void clear_current_step_events();
	inline void set_triggerstate_matchingWith_boxcollider(const Token(ColliderDetector) p_collision_detector, const Token(BoxCollider) p_matched_boxcollider, const Trigger::State p_trigger_state);
	inline void udpate_triggerstate_from_lastframe_intersectionevents();
	inline void free_deleted_colliders();
};