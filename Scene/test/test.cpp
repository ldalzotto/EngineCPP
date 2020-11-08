#include "Scene/Scene.cpp"

using namespace Math;

void main()
{
	Scene l_scene = Scene();
	l_scene.allocate();

	com::PoolToken<SceneNode> l_root_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));
	com::PoolToken<SceneNode> l_child_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));

	NTreeResolve<SceneNode> l_root = l_scene.resolve(l_root_token);
	NTreeResolve<SceneNode> l_child = l_scene.resolve(l_child_token);

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


	l_scene.free();
}