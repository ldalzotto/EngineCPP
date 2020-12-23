
#include "Math/geometry.hpp"
#include "Common/Clock/clock.hpp"
#include <cstdio>
int main()
{
	{
		Math::AABB<float> l_aabb_0 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(2.0f, 2.0f, 2.0f));
		Math::AABB<float> l_aabb_1 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));

		bool l_overlap = Geometry::overlap(l_aabb_0, l_aabb_1);
	}
	{
		/*
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(2.37387276, 2.09807682, 2.43587136), 
			Math::mat3f(Math::vec3f(-0.469846725, 0.866025269, -0.171010107), Math::vec3f(0.342020094, 0.000000000, -0.939692736), Math::vec3f(-0.813797474, -0.500000298, -0.296198100)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
			*/
		
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(1.50000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
		/*
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(2.00000000, 3.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(-0.907673478, 0.330365956, 0.258818865), Math::vec3f(0.243210196, -0.0885212421, 0.965925872), Math::vec3f(0.342019975, 0.939692676, 0.000000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
			*/
		Math::OBB<float> l_obb_1 = Math::OBB<float>(
			Math::vec3f(2.00000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));

		// bool l_overlap = Geometry::overlap(l_obb_0, l_obb_1);

		TimeClockPrecision l_t = clock_currenttime_mics();
		int l_at = 0;
		int i = 0;
		while (i < 10000000)
		{
			l_at += Geometry::overlap3(l_obb_0, l_obb_1);
			i += 1;
		}
		printf("%lld \n", clock_currenttime_mics() - l_t);
		printf("%ld", l_at);
	}
}