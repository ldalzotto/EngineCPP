#pragma once

#include "Common2/common2.hpp"
#include "Math/matrix_def.hpp"
#include "Math/geometry_def.hpp"
#include "Math/transform_def.hpp"

struct CollisionHandle
{
	void* handle;

	inline static CollisionHandle build_default()
	{
		return CollisionHandle{ nullptr };
	};

	void allocate();
	void free();
	void step();
};

struct BoxColliderHandle
{
	size_t handle;

	inline static BoxColliderHandle build_default() { return BoxColliderHandle{ cast(size_t, -1) }; }
	inline void reset() { *this = build_default(); };

	void allocate(CollisionHandle p_collision, const Math::AABB<float>& p_local_aabb);
	void free(CollisionHandle p_collision);
	void on_collider_moved(CollisionHandle p_collision, const Math::Transform& p_transform, const Math::quat& p_local_rotation);
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

	struct Event
	{
		BoxColliderHandle other;
		Trigger::State state = Trigger::State::UNDEFINED;
	};
};

struct ColliderDetectorHandle
{
	size_t handle;
	BoxColliderHandle collider;

	inline static ColliderDetectorHandle build_default()
	{
		return ColliderDetectorHandle{
			cast(size_t, -1), BoxColliderHandle::build_default()
		};
	};

	inline void reset()
	{
		*this = build_default();
	}

	void allocate(CollisionHandle p_collision, BoxColliderHandle p_collider);
	void free(CollisionHandle p_collision);

	Slice<Trigger::Event> get_collision_events(CollisionHandle p_collision);
};