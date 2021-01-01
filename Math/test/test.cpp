
#include "Math/geometry.hpp"
#include "Common/Clock/clock.hpp"
#include <cstdio>

#if 0

int main()
{
	{
		Math::AABB<float> l_aabb_0 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(2.0f, 2.0f, 2.0f));
		Math::AABB<float> l_aabb_1 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.0f), Math::vec3f(0.5f, 0.5f, 0.5f));

		bool l_overlap = Geometry::overlap(l_aabb_0, l_aabb_1);
	}
	{
		
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(2.37387276, 2.09807682, 2.43587136), 
			Math::mat3f(Math::vec3f(-0.469846725, 0.866025269, -0.171010107), Math::vec3f(0.342020094, 0.000000000, -0.939692736), Math::vec3f(-0.813797474, -0.500000298, -0.296198100)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
			
		/*
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(1.50000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));

		Math::OBB<float> l_obb_1 = Math::OBB<float>(
			Math::vec3f(2.00000000, 3.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(-0.907673478, 0.330365956, 0.258818865), Math::vec3f(0.243210196, -0.0885212421, 0.965925872), Math::vec3f(0.342019975, 0.939692676, 0.000000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
					*/
		
		Math::OBB<float> l_obb_1 = Math::OBB<float>(
			Math::vec3f(2.00000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
			
		// bool l_overlap = Geometry::overlap(l_obb_0, l_obb_1);

		
		bool l_01 = Geometry::overlap(l_obb_0, l_obb_1);


		if ((l_01 != Geometry::overlap2(l_obb_0, l_obb_1)) || (l_01 != Geometry::overlap3(l_obb_0, l_obb_1)))
		{
			abort();
		}

		/*
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
		*/
	}
	/*
	{
		
		for (size_t i = 0; i < 100000; i++)
		{
			Math::OBB<float> l_obb_0 = Math::OBB<float>(
				Math::vec3f(1.50000000, 1.00000000, 0.000000000),
				Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
				Math::vec3f(1.0f, 1.0f, 1.0f));

			Math::OBB<float> l_obb_1 = Math::OBB<float>(
				Math::vec3f(2.00000000, 1.00000000, 0.000000000),
				Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
				Math::vec3f(1.0f, 1.0f, 1.0f));

			bool l_01 = Geometry::overlap(l_obb_0, l_obb_1);


			if ((l_01 != Geometry::overlap2(l_obb_0, l_obb_1)) || (l_01 != Geometry::overlap3(l_obb_0, l_obb_1)))
			{
				abort();
			}
		}

	}
		*/
}

#endif 

#include "Common/Clock/clock.hpp"

struct Entity
{
	Math::vec3f position;
	Math::vec4f rotation;
	Math::vec3f scale;
};

template<unsigned N>
struct EntitySOA
{
	Math::vec3f position[N];
	Math::vec4f rotation[N];
	Math::vec3f scale[N];
};

int main()
{
	constexpr int size = 10000000;
	Math::vec3f l_add = Math::vec3f(1.0f, 0.0f, 1.0f);
	Math::vec4f l_add_4 = Math::vec4f(1.0f, 0.0f, 1.0f, 1.0f);

	
	Entity* l_arr_of_enetity = (Entity*)malloc(sizeof(Entity) * size);

	TimeClockPrecision l_clock = clock_currenttime_mics();
	for (size_t i = 0; i < size; i++)
	{
		Entity* l_current_entity = &l_arr_of_enetity[i];
		l_current_entity->position = Math::add(l_current_entity->position, l_add);
		l_current_entity->rotation = Math::add(l_current_entity->rotation, l_add_4);
		l_current_entity->scale = Math::add(   l_current_entity->scale, l_add);
	}
	printf("%lld \n", clock_currenttime_mics() - l_clock);
	

	
	EntitySOA<size>* l_arr_of_enetity_soa = (EntitySOA<size>*)malloc(sizeof(EntitySOA<size>));
	
	l_clock = clock_currenttime_mics();
	for (size_t i = 0; i < size; i++)
	{
		l_arr_of_enetity_soa->position[i] = Math::add(l_arr_of_enetity_soa->position[i], l_add);
		l_arr_of_enetity_soa->rotation[i] = Math::add(l_arr_of_enetity_soa->rotation[i], l_add_4);
		l_arr_of_enetity_soa->scale[i] = Math::add(l_arr_of_enetity_soa->scale[i], l_add);
	}
	printf("%lld", clock_currenttime_mics() - l_clock);
	
	// EntitySOA<size> l_soa_of_entity;
}