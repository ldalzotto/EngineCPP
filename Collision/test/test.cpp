#include "Collision/collision.hpp"
#include "Math/math.hpp"
int main()
{
	CollisionHandle l_collision;
	l_collision.allocate();

	BoxColliderHandle l_box_collider;
	l_box_collider.allocate(l_collision, Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f)));
	l_box_collider.on_collider_moved(l_collision, Math::Transform(Math::vec3f(10.0f, 10.0f, 10.0f), Math::Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Math::vec3f(1.0f, 1.0f, 1.0f)));

	BoxColliderHandle l_box_collider_2;
	l_box_collider_2.allocate(l_collision, Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f)));
	l_box_collider_2.on_collider_moved(l_collision, Math::Transform(Math::vec3f(12.0f, 12.0f, 12.0f), Math::rotateAround(Math::VecConst<float>::UP, 90.0f * DEG_TO_RAD), Math::vec3f(10.0f, 10.0f, 10.0f)));
	// l_box_collider_2.push_trs(l_collision, Math::TRS(Math::vec3f(10.5f, 12.6f, 10.5f), Math::extractAxis<float>(Math::Quaternion(0.0f, 0.0f, 0.0f, 1.0f)), Math::vec3f(1.0f, 1.0f, 1.0f)));

	l_collision.update();

	auto l_events = get_collision_events(l_collision, l_box_collider);
	l_events = get_collision_events(l_collision, l_box_collider_2);

	l_box_collider_2.free(l_collision);
	l_box_collider.free(l_collision);

	l_collision.free();
}