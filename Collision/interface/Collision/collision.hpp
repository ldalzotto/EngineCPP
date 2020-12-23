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

struct BoxColliderHandle : public Handle
{
	using Handle::Handle;

	void allocate(CollisionHandle p_collision, const Math::AABB<float>& p_local_aabb);
	void free(CollisionHandle p_collision);
	void on_collider_moved(CollisionHandle p_collision, const Math::Transform& p_transform, const Math::quat& p_local_rotation);
};

com::Vector<BoxColliderHandle>& get_collision_events(CollisionHandle& p_collision, BoxColliderHandle& p_box_collider);