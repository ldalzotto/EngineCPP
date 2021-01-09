#pragma once


void CollisionHandle::allocate()
{
	Collision2::allocate(cast(Collision2**, &this->handle));
};

void CollisionHandle::free()
{
	Collision2::free(cast(Collision2**, &this->handle));
};

void CollisionHandle::update()
{
	cast(Collision2*, this->handle)->update();
};

void BoxColliderHandle::allocate(CollisionHandle p_collision, const Math::AABB<float>* p_local_aabb)
{
	this->handle = cast(Collision2*, p_collision.handle)->composition_allocate_boxcollider(p_local_aabb).tok;
};

void BoxColliderHandle::free(CollisionHandle p_collision)
{
	cast(Collision2*, p_collision.handle)->composion_free_collider(cast(Token(Collision2::BoxCollider)*, &this->handle));
	this->reset();
};

void BoxColliderHandle::on_collider_moved(CollisionHandle p_collision, const Math::Transform* p_transform, const Math::quat* p_local_rotation)
{
	cast(Collision2*, p_collision.handle)->composition_on_collider_moved(cast(Token(Collision2::BoxCollider)*, &this->handle), p_transform, p_local_rotation);
};


void ColliderDetectorHandle::allocate(CollisionHandle p_collision, BoxColliderHandle p_collider)
{
	this->handle = cast(Collision2*, p_collision.handle)->collision_heap.allocate_colliderdetector(cast(Token(Collision2::BoxCollider)*, &p_collider.handle)).tok;
	this->collider = p_collider;
};

void ColliderDetectorHandle::free(CollisionHandle p_collision)
{
	cast(Collision2*, p_collision.handle)->collision_heap.free_colliderdetector(
						cast(Token(Collision2::BoxCollider)*, &this->collider.handle), 
						cast(Token(Collision2::ColliderDetector)*, &this->handle));
	this->reset();
};

Slice<Trigger::Event> ColliderDetectorHandle::get_collision_events(CollisionHandle& p_collision)
{
	return slice_cast_0v<Trigger::Event>(
		cast(Collision2*, p_collision.handle)->collision_heap.get_triggerevents_from_colliderdetector(cast(Token(Collision2::ColliderDetector)*, &this->handle))
		.build_aschar()
	);
};
