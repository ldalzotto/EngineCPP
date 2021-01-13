
#include "Math2/math.hpp"
#include "Common2/common2.hpp"

using namespace v2;


int main()
{
	assert_true((v3f{1.0f, 2.0f, 3.0f} + v3f{1.0f, 2.0f, 3.0f}) == v3f{2.0f, 4.0f, 6.0f} );
	assert_true((v3f{1.0f, 2.0f, 3.0f} - v3f{1.0f, 2.0f, 3.0f}) == v3f_const::ZERO );
	assert_true((v3f{1.0f, 2.0f, 3.0f} * v3f{1.0f, 2.0f, 3.0f}) == v3f{1.0f, 4.0f, 9.0f} );
	assert_true(v3f{1.0f, 2.0f, 3.0f}.dot(v3f{1.0f, 2.0f, 3.0f}) == 14.0f);
	assert_true(v3f{1.0f, 2.0f, 3.0f}.cross(v3f{1.0f, 2.0f, 3.0f}) == v3f{0.0f, 0.0f, 0.0f});
	assert_true(v3f{1.0f, 2.0f, 3.0f}.length() == 3.74165750f);
	assert_true(v3f{1.0f, 2.0f, 3.0f}.normalize() == v3f{0.267261237f, 0.534522474f, 0.801783681f});
	assert_true(v3f{1.0f, 2.0f, 3.0f}.project(v3f{0.3f, 0.1f, 0.9f}) == v3f{1.00635338f, 0.335451126f, 3.01906013f} );
	assert_true(v3f{1.0f, 2.0f, 3.0f}.distance(v3f{0.3f, 0.1f, 0.9f}) == 2.91719031f);
	assert_true(v3f{1.0f, 2.0f, 3.0f}.angle(v3f{0.3f, 0.1f, 0.9f}) == 0.458921432f);
	assert_true(v3f{1.0f, 2.0f, 3.0f}.normalize().angle_normalized(v3f{0.3f, 0.1f, 0.9f}.normalize()) == 0.458921283f);
	assert_true(v3f{1.0f, 2.0f, 3.0f}.anglesign(v3f{0.3f, 0.1f, 0.9f}, v3f_const::FORWARD) == -1.0f);
	
	assert_true(v3f{1.0f, 2.0f, 3.0f}.rotate(quat::rotate_around(v3f{0.3f, 0.1f, 0.9f}.normalize(), Math::M_PI * 0.3f)) 
		== v3f{-0.249471188f, 1.32052481f, 3.49198771f});
	
	assert_true((quat{0.2f, 0.1f, 0.8f, 0.3f} * quat{0.8f, 0.2f, 0.3f, 0.1f}) == quat{0.166666672f, 0.833333313f, 0.166666672f, -0.500000000f});
	assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.inv() == quat{-0.200000003f, -0.100000001f, -0.800000012f, 0.300000012f});
	assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.cross(quat{0.8f, 0.2f, 0.3f, 0.1f}) == quat{0.0f, 0.0f, 0.0f, 1.0f});

	assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.rotation_to_axis() ==
		m33f{-0.458563447f, 0.794843137f, 0.397421569f, 
			-0.694313765f, -0.568075001f, 0.441836059f, 
			0.388646394f, 0.0409101509f, 0.920478284f}
	 );

	 assert_true(
		m33f::build_columns(v3f{1.0f, 5.0f, 0.3f},v3f{6.0f, 0.4f, 2.3f},v3f{9.0f, 1.0f, 0.3f}).axis_to_rotation()
		== quat{0.395577371f, 2.64732552f, -0.304290295f, 0.821583867f}
	 );
};
