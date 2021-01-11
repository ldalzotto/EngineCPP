#pragma once


void CollisionHandle::allocate()
{
	Collision2::allocate(cast(Collision2**, &this->handle));
};

void CollisionHandle::free()
{
	Collision2::free(cast(Collision2**, &this->handle));
};

void CollisionHandle::step()
{
	cast(Collision2*, this->handle)->step();
};

using Collision2Ext = Collision2::ExternalInterface;

void BoxColliderHandle::allocate(CollisionHandle p_collision, const Math::AABB<float>* p_local_aabb)
{
	this->handle = Collision2Ext::allocate_boxcollider(cast(Collision2*, p_collision.handle), p_local_aabb).tok;
};

void BoxColliderHandle::free(CollisionHandle p_collision)
{
	Collision2Ext::free_collider(cast(Collision2*, p_collision.handle), cast(Token(Collision2::BoxCollider)*, &this->handle));
};

void BoxColliderHandle::on_collider_moved(CollisionHandle p_collision, const Math::Transform* p_transform, const Math::quat* p_local_rotation)
{
	Collision2Ext::on_collider_moved(cast(Collision2*, p_collision.handle), cast(Token(Collision2::BoxCollider)*, &this->handle), p_transform, p_local_rotation);
};

void BoxColliderHandle::on_collider_moved(CollisionHandle p_collision, const Math::Transform p_transform, const Math::quat p_local_rotation)
{
	this->on_collider_moved(p_collision, &p_transform, &p_local_rotation);
};


void ColliderDetectorHandle::allocate(CollisionHandle p_collision, BoxColliderHandle p_collider)
{
	this->handle = Collision2Ext::allocate_colliderdetector(cast(Collision2*, p_collision.handle), cast(Token(Collision2::BoxCollider)*, &p_collider.handle)).tok;
	this->collider = p_collider;
};

void ColliderDetectorHandle::free(CollisionHandle p_collision)
{
	Collision2Ext::free_colliderdetector(
		cast(Collision2*, p_collision.handle), 
		cast(Token(Collision2::BoxCollider)*, &this->collider.handle),
		cast(Token(Collision2::ColliderDetector)*, &this->handle)
	);
};

Slice<Trigger::Event> ColliderDetectorHandle::get_collision_events(CollisionHandle& p_collision)
{
	return slice_cast_0v<Trigger::Event>(
		cast(Collision2*, p_collision.handle)->collision_heap.get_triggerevents_from_colliderdetector(cast(Token(Collision2::ColliderDetector)*, &this->handle))
		.build_aschar()
	);
};
