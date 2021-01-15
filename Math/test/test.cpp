
#include "Math/geometry.hpp"
#include "Common/Clock/clock.hpp"
#include <cstdio>

using namespace Math;

int main()
{

	float l_result = Math::dot(Vector<3, float>(1.0f, 2.0f, 3.0f), Vector<3, float>(1.0f, 2.0f, 3.0f));
	Vector<3, float> l_result_v = Math::cross(Vector<3, float>(1.0f, 2.0f, 3.0f), Vector<3, float>(1.0f, 2.0f, 3.0f));
	l_result = Math::length(Vector<3, float>(1.0f, 2.0f, 3.0f));
	l_result_v = Math::normalize(Vector<3, float>(1.0f, 2.0f, 3.0f));
	l_result_v = Math::project(Math::normalize(Vector<3, float>(1.0f, 2.0f, 3.0f)), Math::normalize(Vector<3, float>(0.3f, 0.1f, 0.9f)));
	l_result = Math::distance(Vector<3, float>(1.0f, 2.0f, 3.0f), Vector<3, float>(0.3f, 0.1f, 0.9f));
	l_result = Math::angle(Vector<3, float>(1.0f, 2.0f, 3.0f), Vector<3, float>(0.3f, 0.1f, 0.9f));
	l_result = Math::angle_normalized(Math::normalize(Vector<3, float>(1.0f, 2.0f, 3.0f)), Math::normalize(Vector<3, float>(0.3f, 0.1f, 0.9f)));
	l_result = Math::anglesign(Vector<3, float>(1.0f, 2.0f, 3.0f), Math::normalize(Vector<3, float>(0.3f, 0.1f, 0.9f)), Math::VecConst<float>::FORWARD);
	// Quaternion l_result_q = Math::rotateAround(Math::normalize(Vector<3, float>(0.3f, 0.1f, 0.9f)), M_PI * 0.3f);
	l_result_v = Math::rotate(Vector<3, float>(1.0f, 2.0f, 3.0f), Math::rotateAround(Math::normalize(Vector<3, float>(0.3f, 0.1f, 0.9f)), M_PI * 0.3f));

	Quaternion l_result_q = Math::mul(Quaternion(Vector<3, float>(0.2f, 0.1f, 0.8f), 0.3f), Quaternion(Vector<3, float>(0.8f, 0.2f, 0.3f), 0.1f));
	l_result_q = Math::inv(Quaternion(Vector<3, float>(0.2f, 0.1f, 0.8f), 0.3f));
	l_result_q = Math::cross(Quaternion(Vector<3, float>(0.2f, 0.1f, 0.8f), 0.3f), Quaternion(Vector<3, float>(0.8f, 0.2f, 0.3f), 0.1f));
	Matrix<3, float> l_result_m3 = Math::extractAxis<float>(Quaternion(0.2f, 0.1f, 0.8f, 0.3f));
	l_result_q = Math::fromAxis(Matrix<3, float>(Vector<3, float>(1.0f, 5.0f, 0.3f), Vector<3, float>(6.0f, 0.4f, 2.3f), Vector<3, float>(9.0f, 1.0f, 0.3f)));

	l_result_q = Math::fromEulerAngle(Vector<3, float>(1.0f, 2.0f, 3.0f));
	l_result_q = Math::fromTo(Vector<3, float>(1.0f, 2.0f, 3.0f), Vector<3, float>(0.3f, 0.1f, 0.9f));
	l_result_v = Math::eulerAngle<float>(Quaternion(Vector<3, float>(0.8f, 0.2f, 0.3f), 0.1f));

	Matrix<4, float> l_result_m4 = Math::TRS(Vector<3, float>(1.0f, 5.0f, 0.3f), Math::Matrix<3, float>(Vector<3, float>(1.0f, 5.0f, 0.3f), Vector<3, float>(-0.2f, 3.0f, 1.3f), Vector<3, float>(2.8f, -6.4f, 4.3f)), Vector<3, float>(1.0f, 3.0f, 0.2f));
	l_result_m4 = Math::lookAtRotation<4, float>(Vector<3, float>(1.0f, 5.0f, 0.3f), Vector<3, float>(-0.2f, 3.0f, 1.3f), Vector<3, float>(2.8f, -6.4f, 4.3f));
	l_result_m4 = Math::lookAtRotation_viewmatrix<4, float>(Vector<3, float>(1.0f, 5.0f, 0.3f), Vector<3, float>(-0.2f, 3.0f, 1.3f), Math::normalize(Vector<3, float>(2.8f, -6.4f, 4.3f)));
	// l_result_m4 = Math::inv(Matrix<4, float>(Vector<3, float>));
	l_result_m4 = Math::view(Vector<3, float>(1.0f, 5.0f, 0.3f), Vector<3, float>(-0.2f, 3.0f, 1.3f), Vector<3, float>(2.8f, -6.4f, 4.3f));
	l_result_m4 = Math::perspective<float>(1.0f, 2.0f, 3.0f, 4.0f);


	{
		Math::AABB<float> l_aabb_0 = Math::AABB<float>(Math::vec3f(0.0f, 0.5f, 0.0f), Math::vec3f(2.0f, 2.0f, 2.0f));
		Math::AABB<float> l_aabb_1 = Math::AABB<float>(Math::vec3f(0.0f, 0.0f, 0.5f), Math::vec3f(0.5f, 0.5f, 0.5f));

		bool l_overlap = Geometry::overlap(l_aabb_0, l_aabb_1);
		int zd = 10;
	}

	{

		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(2.37387276, 2.09807682, 2.43587136),
			Math::mat3f(Math::vec3f(-0.469846725, 0.866025269, -0.171010107), Math::vec3f(0.342020094, 0.000000000, -0.939692736), Math::vec3f(-0.813797474, -0.500000298, -0.296198100)),
			Math::vec3f(1.0f, 1.0f, 1.0f));

#if 0
		Math::OBB<float> l_obb_0 = Math::OBB<float>(
			Math::vec3f(1.50000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
#endif
#if 0
		Math::OBB<float> l_obb_1 = Math::OBB<float>(
			Math::vec3f(2.00000000, 3.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(-0.907673478, 0.330365956, 0.258818865), Math::vec3f(0.243210196, -0.0885212421, 0.965925872), Math::vec3f(0.342019975, 0.939692676, 0.000000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
#endif
#if 1
		Math::OBB<float> l_obb_1 = Math::OBB<float>(
			Math::vec3f(2.00000000, 1.00000000, 0.000000000),
			Math::mat3f(Math::vec3f(1.00000000, 0.000000000, 0.000000000), Math::vec3f(0.000000000, 1.00000000, 0.000000000), Math::vec3f(0.000000000, 0.000000000, 1.00000000)),
			Math::vec3f(1.0f, 1.0f, 1.0f));
#endif
		// bool l_overlap = Geometry::overlap(l_obb_0, l_obb_1);


		bool l_01 = Geometry::overlap(l_obb_0, l_obb_1);


		if ((l_01 != Geometry::overlap2(l_obb_0, l_obb_1)) || (l_01 != Geometry::overlap3(l_obb_0, l_obb_1)))
		{
			abort();
		}

	}

	int zd;

};



#if 0
int main()
{





}

#endif