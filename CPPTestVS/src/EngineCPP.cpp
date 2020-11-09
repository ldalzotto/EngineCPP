// CPPTestVS.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "Math/math.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/pool.hpp"
#include <vector>
#include "Engine/engine.hpp"
#include "SceneComponents/components.hpp"

using namespace Math;

bool l_executed = false;

void update(void* p_engine, float p_delta)
{
	if (!l_executed)
	{
		l_executed = true;
		EngineHandle* l_engine = (EngineHandle*)p_engine;
		SceneHandle l_scenehandle = engine_scene(*l_engine);
		/*
		for (int i = 0; i < 100; i++)
		{
			*/
		{
			auto l_node = l_scenehandle.add_node(l_scenehandle.root(), Math::Transform());
			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.vertex_shader = "E:/GameProjects/CPPTestVS/Render/shader/TriVert.spv";
			l_mesh_renderer.fragment_shader = "E:/GameProjects/CPPTestVS/Render/shader/TriFrag.spv";
			l_scenehandle.add_component<MeshRenderer>(l_node, l_mesh_renderer);
		}
		{
			auto l_node = l_scenehandle.add_node(l_scenehandle.root(), Math::Transform(vec3f(3.0f, 3.0f, 0.0f), QuatConst::IDENTITY, VecConst<float>::ONE));
			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.vertex_shader = "E:/GameProjects/CPPTestVS/Render/shader/TriVert.spv";
			l_mesh_renderer.fragment_shader = "E:/GameProjects/CPPTestVS/Render/shader/TriFrag.spv";
			l_scenehandle.add_component<MeshRenderer>(l_node, l_mesh_renderer);
		}
		/*
	}
	*/
	}



	//l_scenehandle.
};

int main()
{
	/*
	std::vector<float> zd;
	std::allocator<float> l_del;
	sizeof(zd);
	std::cout << "Hello World!\n";

	vec3f l_234 = vec3f(2.0f, 3.0f, 4.0f);
	vec3f l_564 = vec3f(5.0f, 6.0f, 4.0f);
	vec3f l_100 = vec3f(1.0f, 0.0f, 0.0f);
	quat l_q = quat(0.15f, 0.74f, 0.32f, 0.55f);
	quat l_q2 = quat(0.65f, 0.14f, 0.22f, 0.65f);

	mat4f l_m1 = mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
	mat4f l_m2 = mat4f(4.0f, 2.0f, 3.0f, 4.0f, 10.0f, 6.0f, 7.0f, 9.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 74.0f, 15.0f, 16.0f);
	mat3f l_m3 = mat3f(4.0f, 2.0f, 3.0f, 4.0f, 10.0f, 6.0f, 7.0f, 9.0f, 9.0f);

	float l_dot = dot(l_234, l_234);
	vec3f l_cross = cross(l_234, l_234);
	float l_length = length(l_234);
	vec3f l_normalize = normalize(l_234);
	vec3f l_project = project(l_234, l_564);
	float l_distance = distance(l_234, vec3f(5.0f, 6.0f, 4.0f));
	float l_angle = angle(l_234, l_100);
	float l_angleNormalize = angle_normalized(normalize(l_234), normalize(l_100));
	short l_angleSign = anglesign(l_234, l_100, l_100);
	vec3f l_rotate = rotate(l_234, l_q);

	quat l_normalizeq = normalize(l_q);
	quat l_mul = mul(l_q, l_q);
	quat l_conjugate = inv(l_q);
	quat l_rotateAround = rotateAround(l_100, 0.25f);
	quat l_crossq = cross(l_q, l_q2);
	mat3f l_axis = extractAxis<float>(l_q2);
	quat l_fromDirection = fromDirection(l_234);
	quat l_fromEulerAngle = fromEulerAngle(l_234);
	quat l_fromAxis = fromAxis(l_m3);
	quat l_fromTo = fromTo(l_234, l_100);

	mat4f l_mult = mul(l_m1, l_m1);
	mat4f l_inv = inv(l_m2);
	mat4f l_trs = TRS(vec3f(1.0f, 2.0f, 3.0f), mat3f(vec3f(4.0f, 5.0f, 6.0f), vec3f(7.0f, 8.0f, 9.0f), vec3f(10.0f, 11.0f, 12.0f)), vec3f(1.0f, 1.5f, 2.0f));
	mat4f l_perspective = perspective<float>(0.25f, 1.96f, 0.02f, 50.0f);
	mat4f l_lookatView = lookAtView(l_234, l_100, VecConst<float>::UP);
	mat4f l_lookatRot = lookAtRotation<4, float>(l_234, l_100, VecConst<float>::UP);



	com::Vector<float> l_vf(0);
	for (size_t i = 0; i < 100; i++)
	{
		l_vf.push_back(i);
	}
	l_vf.erase_at(0);
	l_vf.swap(0, 1);

	com::Vector<float> l_vf2 = l_vf;

	l_vf2 = com::Vector<float>(100);

	com::Pool<float> l_pool(10);
	com::PoolToken<float> l_zd = l_pool.alloc_element(5.0f);
	float& l_f = l_pool.resolve(l_zd);
	*/
	EngineHandle l_engine;
	ExternalHooks l_external_hooks;
	l_external_hooks.ext_update = update;
	l_external_hooks.closure = &l_engine;
	l_engine = engine_create(l_external_hooks);
	engine_mainloop(l_engine);
	engine_destroy(l_engine);

}