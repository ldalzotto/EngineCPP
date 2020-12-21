
#include "Math/geometry.hpp"

int main()
{
	Math::AABB<float> l_aabb_0 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(2.0f, 2.0f, 2.0f));
	Math::AABB<float> l_aabb_1 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));

	bool l_overlap = Geometry::overlap(l_aabb_0, l_aabb_1);
	// bool l_contains = Geometry::contains(l_aabb_0, l_aabb_1);
}