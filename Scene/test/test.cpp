#include "Scene/Scene.cpp"

#include "Scene/component_def.hpp"
#include "Common/Container/resource_map.hpp"

using namespace Math;

struct ComponentTest
{
	static const size_t Id;
	static const SceneNodeComponent_TypeInfo Type;
	int zd;
};

size_t const ComponentTest::Id = 10;
SceneNodeComponent_TypeInfo const ComponentTest::Type = SceneNodeComponent_TypeInfo(ComponentTest::Id, sizeof(ComponentTest));

/*
struct OnComponentTestAddedFn : public IOnComponentAddedFn<ComponentTest>
{
	inline void execute(NTreeResolve<SceneNode>& p_node, ComponentTest* p_component)
	{
	};
};
*/

void component_added_cb(void* p_clos, ComponentAddedParameter* p_par)
{

};

/*
#include <functional>

template<>
struct Hash<vec3f>
{
	inline static size_t hash(const vec3f& p_vec)
	{
		return std::hash<float>{}(p_vec.x) ^ (std::hash<float>{}(p_vec.y) ^ (std::hash<float>{}(p_vec.z) << 1) << 1);
	}
};
*/

void main()
{
	Scene l_scene = Scene();
	l_scene.allocate(Callback<void, ComponentAddedParameter>(nullptr, component_added_cb));

	com::PoolToken<SceneNode> l_root_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));
	com::PoolToken<SceneNode> l_child_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));

	NTreeResolve<SceneNode> l_root = l_scene.resolve_node(l_root_token);
	NTreeResolve<SceneNode> l_child = l_scene.resolve_node(l_child_token);

	l_scene.root().element->addchild(l_root_token);
	l_root.element->addchild(l_child_token);
	mat4f l_localtoworld = l_root.element->get_localtoworld();
	mat4f l_worldtolocal = l_root.element->get_worldtolocal();

	mat4f l_child_localtoworld = l_child.element->get_localtoworld();
	mat4f l_child_worldtolocal = l_child.element->get_worldtolocal();

	vec3f l_child_worldposition = l_child.element->get_worldposition();
	quat l_child_worldrotation = l_child.element->get_worldrotation();
	vec3f l_child_scalefactor = l_child.element->get_worldscalefactor();

	l_child.element->set_worldposition(vec3f(0.0f, 0.0f, 0.0f));
	l_child.element->set_worldrotation(quat(0.0f, 0.0f, 0.0f, 1.0f));
	l_child.element->set_worldscale(vec3f(1.0f, 1.0f, 1.0f));

	l_child_worldposition = l_child.element->get_worldposition();
	l_child_worldrotation = l_child.element->get_worldrotation();
	l_child_scalefactor = l_child.element->get_worldscalefactor();



	SceneHandle l_scene_handle;
	l_scene_handle.handle = &l_scene;

	com::PoolToken<ComponentTest> l_comp = l_scene_handle.add_component<ComponentTest>(l_child_token);
	ComponentTest* l_c = l_scene_handle.resolve_component(l_comp);
	// com::PoolToken<ComponentTest> l_comp = l_scene_handle.allocate_component<ComponentTest>();


	l_scene.free();
}