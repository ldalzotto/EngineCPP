#pragma once

#include "Common/Container/vector_def.hpp"
#include "Common/Memory/handle.hpp"
#include "Math/matrix_def.hpp"
#include "Math/geometry_def.hpp"
#include "Math/transform_def.hpp"

struct CollisionHandle
{
	void* handle = nullptr;

	void allocate();
	void free();
	void update();
};

struct ColliderDetectorHandle;

struct BoxColliderHandle : public Handle
{
	using Handle::Handle;

	void allocate(CollisionHandle p_collision, const Math::AABB<float>& p_local_aabb);
	void free(CollisionHandle p_collision);
	void on_collider_moved(CollisionHandle p_collision, const Math::Transform& p_transform, const Math::quat& p_local_rotation);
};

struct ColliderDetectorHandle
{
	size_t handle = -1;
	BoxColliderHandle collider = BoxColliderHandle();

	inline void reset()
	{
		this->handle = -1;
		this->collider.reset();
	}

	void allocate(CollisionHandle p_collision, BoxColliderHandle p_collider);
	void free(CollisionHandle p_collision);

	com::Vector<BoxColliderHandle>& get_collision_events(CollisionHandle& p_collision);
};