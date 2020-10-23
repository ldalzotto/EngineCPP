// CPPTestVS.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "math.hpp"

using namespace Math;

int main()
{
	std::cout << "Hello World!\n";
	
	vec3f l_234 = vec3f(2.0f, 3.0f, 4.0f);
	vec3f l_564 = vec3f(5.0f, 6.0f, 4.0f);
	vec3f l_100 = vec3f(1.0f, 0.0f, 0.0f);
	Quaternion l_q = Quaternion(0.15f, 0.74f, 0.32f, 0.55f);
	Quaternion l_q2 = Quaternion(0.65f, 0.14f, 0.22f, 0.65f);

	float l_dot = dot(l_234, l_234);
	Vector<3, float> l_cross = cross(l_234, l_234);
	float l_length = length(l_234);
	Vector<3, float> l_normalize = normalize(l_234);
	Vector<3, float> l_project = project(l_234, l_564);
	float l_distance = distance(l_234, vec3f(5.0f, 6.0f, 4.0f));
	float l_angle = angle(l_234, l_100);
	float l_angleNormalize = angle_normalized(normalize(l_234), normalize(l_100));
	short l_angleSign = anglesign(l_234, l_100, l_100);
	Vector<3, float> l_rotate = rotate(l_234, l_q);

	Quaternion l_normalizeq = normalize(l_q);
	Quaternion l_mul = mul(l_q, l_q);
	Quaternion l_conjugate = conjugate(l_q);
	Quaternion l_rotateAround = rotateAround(l_100, 0.25f);
	Quaternion l_crossq = cross(l_q, l_q2);
	Quaternion l_fromDirection = fromDirection(l_234);
	Quaternion l_fromEulerAngle = fromEulerAngle(l_234);
	Quaternion l_fromTo = fromTo(l_234, l_100);
}