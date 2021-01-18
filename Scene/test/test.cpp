
#include "Scene/component_def.hpp"
#include "Scene/kernel/scene.hpp"
#include "Common/Container/resource_map.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Serialization/binary.hpp"
#include "Common/Serialization/json.hpp"
#include "Math/serialization.hpp"
#include "Scene/assets.hpp"
#include "Scene/kernel/scene.hpp"
#include "SceneComponents/components.hpp"

using namespace Math;

struct ComponentTest
{
	static const size_t Id;
	static const SceneNodeComponent_TypeInfo Type;
	int a = 1;
	int b = 2;
};

size_t const ComponentTest::Id = 10;
SceneNodeComponent_TypeInfo const ComponentTest::Type = SceneNodeComponent_TypeInfo(ComponentTest::Id, sizeof(ComponentTest));



inline void component_added_cb(void* p_clos, ComponentAddedParameter* p_par)
{

};

inline void component_removed_cb(void* p_clos, ComponentRemovedParameter* p_par)
{

};

inline void component_asset_push_cb(void* p_clos, ComponentAssetPushParameter* p_par)
{
	if (p_par->component_asset->id == ComponentTest::Id)
	{
		SceneKernel::add_component<ComponentTest>((Scene*)p_par->scene, p_par->node, *(ComponentTest*)p_par->component_asset_object);
	}

};

#include <cstdlib>
#include "Common/Clock/clock.hpp"
int main()
{
	
	Scene l_scene;
	SceneKernel::allocate_scene(&l_scene,
		Callback<void, ComponentAddedParameter>(nullptr, component_added_cb),
		Callback<void, ComponentRemovedParameter>(nullptr, component_removed_cb),
		Callback<void, ComponentAssetPushParameter>(nullptr, component_asset_push_cb)
	);

	SceneNodeToken l_node_1 = SceneKernel::add_node(&l_scene, Math::Transform());
	SceneNodeToken l_node_2 = SceneKernel::add_node(&l_scene, l_node_1, Math::Transform(Math::vec3f(1.0f,1.0f, 1.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE));
	SceneNodeToken l_node_3 = SceneKernel::add_node(&l_scene, l_node_1, Math::Transform(Math::vec3f(-1.0f, -1.0f, -1.0f), Math::QuatConst::IDENTITY, Math::VecConst<float>::ONE));

	// Position
	{
		SceneKernel::set_localposition(l_node_1, &l_scene, Math::vec3f(1.0f, 1.0f, 1.0f));
		Math::vec3f l_result_v3f = SceneKernel::get_localposition(l_node_1, &l_scene);
		l_result_v3f = SceneKernel::get_worldposition(SceneKernel::resolve_node(&l_scene, l_node_2), &l_scene);
		l_result_v3f = SceneKernel::get_worldposition(SceneKernel::resolve_node(&l_scene, l_node_3), &l_scene);

		SceneKernel::set_localposition(l_node_2, &l_scene, Math::vec3f(1.0f, 2.0f, 3.0f));
		l_result_v3f = SceneKernel::get_localposition(l_node_2, &l_scene);
		l_result_v3f = SceneKernel::get_worldposition(SceneKernel::resolve_node(&l_scene, l_node_2), &l_scene);

		SceneKernel::set_worldposition(l_node_3, &l_scene, Math::vec3f(-1.0f, -2.0f, 3.0f));
		l_result_v3f = SceneKernel::get_localposition(l_node_3, &l_scene);
		l_result_v3f = SceneKernel::get_worldposition(SceneKernel::resolve_node(&l_scene, l_node_3), &l_scene);

		int zd = 1;
	}

	// Rotations
	{
		SceneKernel::set_localrotation(l_node_1, &l_scene, Math::fromEulerAngle(Math::vec3f(0.32f, 0.9f, 0.7f)));
		Math::quat l_result_quat = SceneKernel::get_localrotation(l_node_1, &l_scene);
		l_result_quat = SceneKernel::get_worldrotation(SceneKernel::resolve_node(&l_scene, l_node_2), &l_scene);
		l_result_quat = SceneKernel::get_worldrotation(SceneKernel::resolve_node(&l_scene, l_node_3), &l_scene);

		SceneKernel::set_localrotation(l_node_2, &l_scene, Math::fromEulerAngle(Math::vec3f(0.32f, -0.9f, -0.7f)));
		l_result_quat = SceneKernel::get_localrotation(l_node_2, &l_scene);
		l_result_quat = SceneKernel::get_worldrotation(SceneKernel::resolve_node(&l_scene, l_node_2), &l_scene);

		SceneKernel::set_worldrotation(l_node_3, &l_scene, Math::fromEulerAngle(Math::vec3f(-1.0f, -2.0f, 3.0f)));
		l_result_quat = SceneKernel::get_localrotation(l_node_3, &l_scene);
		l_result_quat = SceneKernel::get_worldrotation(SceneKernel::resolve_node(&l_scene, l_node_3), &l_scene);

		int zd = 1;
	}

	// Scale
	{
		SceneKernel::set_localscale(l_node_1, &l_scene, Math::vec3f(2.0f, 0.5f, 1.0f));
		Math::vec3f l_result_v3f = SceneKernel::get_localscale(l_node_1, &l_scene);
		l_result_v3f = SceneKernel::get_worldscalefactor(SceneKernel::resolve_node(&l_scene, l_node_2).element, &l_scene);
		l_result_v3f = SceneKernel::get_worldscalefactor(SceneKernel::resolve_node(&l_scene, l_node_3).element, &l_scene);

		SceneKernel::set_localscale(l_node_2, &l_scene, Math::vec3f(1.0f, 2.0f, 3.0f));
		l_result_v3f = SceneKernel::get_localscale(l_node_2, &l_scene);
		l_result_v3f = SceneKernel::get_worldscalefactor(SceneKernel::resolve_node(&l_scene, l_node_2).element, &l_scene);

		SceneKernel::set_worldscale(l_node_3, &l_scene, Math::vec3f(-1.0f, -2.0f, 3.0f));
		l_result_v3f = SceneKernel::get_localscale(l_node_3, &l_scene);
		l_result_v3f = SceneKernel::get_worldscalefactor(SceneKernel::resolve_node(&l_scene, l_node_3).element, &l_scene);

		int zd = 1;
	}

	SceneKernel::free_scene(&l_scene);
}