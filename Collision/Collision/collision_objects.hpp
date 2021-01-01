#pragma once

#include "Collision/collision.hpp"

void CollisionHandle::allocate()
{
	Collision* l_collision = new Collision();
	l_collision->allocate();
	this->handle = l_collision;
};

void CollisionHandle::free()
{
	((Collision*)this->handle)->free();
	delete ((Collision*)this->handle);
};

void CollisionHandle::update()
{
	((Collision*)this->handle)->update();
};

void BoxColliderHandle::allocate(CollisionHandle p_collision, const Math::AABB<float>& p_local_aabb)
{
	this->handle = ((Collision*)p_collision.handle)->collision_heap.allocate_boxcollider(p_local_aabb).val;
};

void BoxColliderHandle::free(CollisionHandle p_collision)
{
	((Collision*)p_collision.handle)->free_boxcollider(TToken_cast_numeric<BoxCollider>(&this->handle));
	this->reset();
};

void BoxColliderHandle::on_collider_moved(CollisionHandle p_collision, const Math::Transform& p_transform, const Math::quat& p_local_rotation)
{
	((Collision*)p_collision.handle)->on_collider_moved(TToken_cast_numeric<BoxCollider>(&this->handle), p_transform, p_local_rotation);
};


void ColliderDetectorHandle::allocate(CollisionHandle p_collision, BoxColliderHandle p_collider)
{
	this->handle = ((Collision*)p_collision.handle)->collision_heap.allocate_colliderdetector(TToken_build<BoxCollider>(p_collider.handle)).val;
	this->collider = p_collider;
};

void ColliderDetectorHandle::free(CollisionHandle p_collision)
{
	((Collision*)p_collision.handle)->free_colliderdetector(TToken_cast_numeric<BoxCollider>(&this->collider.handle), TToken_cast_numeric<ColliderDetector>(&this->handle));
	this->reset();
};

com::Vector<Trigger::Event>& ColliderDetectorHandle::get_collision_events(CollisionHandle& p_collision)
{
	Collision* l_collision = (Collision*)p_collision.handle;

//TODO -> is it really necessary ?
#if COLLIDER_BOUND_TEST
	if (pool_is_token_free(&l_collision->collision_heap.collider_detectors, TToken_cast_numeric<ColliderDetector>(&this->handle)))
	{
		abort();
	}
#endif

	return (com::Vector<Trigger::Event>&)*l_collision->collision_heap.get_trigger_events(TToken_cast_numeric<ColliderDetector>(&this->handle));	
};
