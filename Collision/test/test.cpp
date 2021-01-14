
#include "Collision/collision.hpp"

//TODO -> test scenario

/*
	3 BoxColliders that intersect each other : (1,2,3)
	ColliderDetector attached to 1
		-> B1 generates two TRIGGER_ENTER TriggerEvent with B2 and B3
	Nothing happens
		-> B1 have two TRIGGER_STAY TriggerEvent with B2 and B3
	BoxCollider1 moves away
		-> B1 have two TRIGGER_EXIT TriggerEvent with B2 and B3
	Nothing happens
		-> B1 have two NONE TriggerEvent with B2 and B3
*/
inline void collision_test_01()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_box_collider_1_transform = Math::Transform(Math::vec3f(1000000.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_box_collider_1.free(l_collision);
	l_box_collider_2.free(l_collision);
	l_box_collider_3.free(l_collision);

	l_collision.free();
}

/*
	3 BoxColliders that intersect each other : (1,2,3)
	ColliderDetector attached to 1
	B1 TriggerEvent is already TRIGGER_STAY with B2 and B3
	BoxCollider2 moves away
		-> B1~B2 : TRIGGER_EXIT, B1~B3 : TRIGGER_STAY
	Nothing happens
		-> B1~B2 : NONE, B1~B3 : TRIGGER_STAY
	Deleting the B3
		-> B1~B2 : NONE, B1~B3 : TRIGGER_EXIT
	Nothing happens
		-> B1~B2 : NONE
*/
inline void collision_test_02()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
	}

	l_collision.step();

	l_collision.step();

	l_box_collider_2_transform = Math::Transform(Math::vec3f(1000000.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_box_collider_3.free(l_collision);

	//Taking deletion into account
	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}

	l_box_collider_1.free(l_collision);
	l_box_collider_2.free(l_collision);

	l_collision.free();
}
 
/*
	3 BoxColliders that intersect each other : (1,2,3). 
	ColliderDetector attached to all.
		-> B1~B2 : TRIGGER_ENTER, B1~B3 : TRIGGER_ENTER
		   B2~B1 : TRIGGER_ENTER, B2~B3 : TRIGGER_ENTER
		   B3~B1 : TRIGGER_ENTER, B3~B2 : TRIGGER_ENTER
	BoxCollider2 moves away
		-> B1~B2 : TRIGGER_EXIT, B1~B3 : TRIGGER_STAY
		   B2~B1 : TRIGGER_EXIT, B2~B3 : TRIGGER_EXIT
		   B3~B1 : TRIGGER_STAY, B3~B2 : TRIGGER_EXIT
	BoxCollider1 destroyed
		-> B1~B2 : NONE, B1~B3 : TRIGGER_EXIT
		   B2~B1 : TRIGGER_EXIT, B2~B3 : NONE
		   B3~B1 : TRIGGER_EXIT, B3~B2 : NONE
	Nothing happens
		-> B2~B3 : NONE
		   B3~B2 : NONE
*/
inline void collision_test_03()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_2_events = l_box_collider_2_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_2_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_2_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}

	l_box_collider_2_transform = Math::Transform(Math::vec3f(1000000.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(0).other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_1_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_2_events = l_box_collider_2_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_2_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_2_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}

	l_box_collider_1.free(l_collision);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_2_events = l_box_collider_2_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_2_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_2_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_2_events = l_box_collider_2_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_2_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_2_events.get(1).other.handle == l_box_collider_3.handle);
	}
	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::NONE);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}

	l_box_collider_2.free(l_collision);
	l_box_collider_3.free(l_collision);

	l_collision.free();
}

/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to all.
	All trigger are already TRIGGER_STAY
	BoxCollider1 destroyed
	ColliderDetector2 destroyed
		-> B3-B1 : TRIGGER_EXIT, B3~B2 : TRIGGER_STAY
	Creating a new BoxCollider far away
		-> B3-B1 : NONE, B3~B2 : TRIGGER_STAY
*/
inline void collision_test_04()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);
	}

	l_collision.step();

	l_box_collider_1.free(l_collision);
	l_box_collider_2_detector_handle.free(l_collision);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}


	Math::Transform l_box_collider_4_transform;
	{
		l_box_collider_4_transform = Math::Transform(Math::vec3f(100000.0f, 100000.0f, 100000.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}
	BoxColliderHandle l_box_collider_4;
	l_box_collider_4.allocate(l_collision, &l_unit_aabb);

	l_collision.step();


	{
		Slice<Trigger::Event> l_box_collider_3_events = l_box_collider_3_detector_handle.get_collision_events(l_collision);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::NONE);
		assert_true(l_box_collider_3_events.get(0).other.handle == l_box_collider_1.handle);
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(l_box_collider_3_events.get(1).other.handle == l_box_collider_2.handle);
	}


	l_box_collider_2.free(l_collision);
	l_box_collider_3.free(l_collision);
	l_box_collider_4.free(l_collision);

	l_collision.free();
}

/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to all.
	All trigger are already TRIGGER_STAY.
	On the same frame:
	destroy everything, create and destroy again.
	create a BoxCollider with ColliderDetector at the same position of B1
		-> B1 has no trigger events
*/
inline void collision_test_05()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);
	}

	l_collision.step();

	{
		l_box_collider_1.free(l_collision);
		l_box_collider_2.free(l_collision);
		l_box_collider_3.free(l_collision);

		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);

		l_box_collider_1.free(l_collision);
		l_box_collider_2.free(l_collision);
		l_box_collider_3.free(l_collision);

		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 0);
	}


	l_box_collider_1.free(l_collision);

	l_collision.free();
}


/*
inline void collision_test_06()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(1000.0f, 1000.0f, 1000.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);
	}

	l_collision.step();

	l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0)->state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(0)->other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1)->state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(1)->other.handle == l_box_collider_3.handle);
	}

	l_box_collider_1_transform = Math::Transform(Math::vec3f(10000.0f, 10000.0f, 10000.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0)->state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(0)->other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1)->state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(1)->other.handle == l_box_collider_3.handle);
	}

	l_collision.free();
}


inline void collision_test_07()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	Math::AABB<float> l_unit_aabb = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));
	Math::Transform l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = Math::Transform(Math::vec3f(0.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_2_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, 0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
		l_box_collider_3_transform = Math::Transform(Math::vec3f(0.25f, 0.0f, -0.25f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	}

	BoxColliderHandle l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1.allocate(l_collision, &l_unit_aabb);
		l_box_collider_2.allocate(l_collision, &l_unit_aabb);
		l_box_collider_3.allocate(l_collision, &l_unit_aabb);

		l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_2.on_collider_moved(l_collision, &l_box_collider_2_transform, &Math::QuatConst::IDENTITY);
		l_box_collider_3.on_collider_moved(l_collision, &l_box_collider_3_transform, &Math::QuatConst::IDENTITY);
	}

	ColliderDetectorHandle l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle.allocate(l_collision, l_box_collider_1);
		l_box_collider_2_detector_handle.allocate(l_collision, l_box_collider_2);
		l_box_collider_3_detector_handle.allocate(l_collision, l_box_collider_3);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0)->state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(0)->other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1)->state == Trigger::State::TRIGGER_ENTER);
		assert_true(l_box_collider_1_events.get(1)->other.handle == l_box_collider_3.handle);
	}


	l_box_collider_1_transform = Math::Transform(Math::vec3f(10000.0f, 0.0f, 0.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE);
	l_box_collider_1.on_collider_moved(l_collision, &l_box_collider_1_transform, &Math::QuatConst::IDENTITY);

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0)->state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(0)->other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1)->state == Trigger::State::TRIGGER_EXIT);
		assert_true(l_box_collider_1_events.get(1)->other.handle == l_box_collider_3.handle);
	}

	l_collision.step();

	{
		Slice<Trigger::Event> l_box_collider_1_events = l_box_collider_1_detector_handle.get_collision_events(l_collision);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0)->state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(0)->other.handle == l_box_collider_2.handle);
		assert_true(l_box_collider_1_events.get(1)->state == Trigger::State::NONE);
		assert_true(l_box_collider_1_events.get(1)->other.handle == l_box_collider_3.handle);
	}

	l_collision.free();
}
*/

inline void collision_test()
{
	collision_test_01();
	collision_test_02();
	collision_test_03();
	collision_test_04();
	collision_test_05();
};

int main()
{
	collision_test();
}
