#pragma once

#include "Scene/scene.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/tree.hpp"
#include "Math/transform_def.hpp"
#include "Math/math.hpp"

using namespace Math;

struct Scene;

struct SceneHeap
{
	//store components ?
};

/*
//TODO -> Adding components to SceneNode. Components are data only objects.
They are allocated by SceneHeap with a generic purpose allocator (generalizing the one developped in Render).
Components are typed by a size_t readonly unique identifier.
A component table is passed as an input of the SceneTree. The component table associate the allocation size for every id.

For every SceneNode component layout chage, the "attached/detached" callbacks associated to the component type is called.
This allow to trigger logic when a specific set of components is present, or not.

Ex : when the MeshRenderer component is attached, we will call the Render middleware to queue creation of render resources.

The goal here is to perform component synchronization callback synchronously. But it is up to the middleware to decide if operations are deferred.
*/
struct SceneNode
{
	NTree<SceneNode>* scenetree_ptr;

	// bool haschanged_this_frame;

private:
	//transform
	Transform transform;

	/** This matrix will always be relative to the root Node (a Node without parent). */
	mat4f localtoworld;
	bool matrices_mustBe_recalculated;
	

	//Childs
	com::PoolToken<NTreeNode> scenetree_entry;

	//vector<components>

public:
	inline SceneNode() {};

	inline SceneNode(const Transform& p_transform, NTree<SceneNode>* p_owned_tree, const com::PoolToken<NTreeNode>& p_scenetree_entry)
	{
		this->transform = p_transform;
		this->scenetree_ptr = p_owned_tree;
		this->scenetree_entry = p_scenetree_entry;

		this->mark_for_recalculation();
	}

	inline void mark_for_recalculation()
	{
		NTreeTraversalIterator<SceneNode, NTree<SceneNode>> l_tree_iterator;
		l_tree_iterator.allocate(this->scenetree_ptr, this->scenetree_entry);
		{
			while (l_tree_iterator.move_next())
			{
				l_tree_iterator.current_resolve.element->matrices_mustBe_recalculated = true;
			}
		}
		l_tree_iterator.free();
	}

	inline void addchild(com::PoolToken<SceneNode>& p_newchild)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		NTreeResolve<SceneNode> l_newchild = this->scenetree_ptr->resolve(p_newchild);


		if (l_newchild.node->parent != this->scenetree_entry.Index)
		{
			if (l_newchild.node->has_parent())
			{
				NTreeResolve<SceneNode> l_newchild_parent = this->scenetree_ptr->resolve(com::PoolToken<SceneNode>(l_newchild.node->parent));
				for (size_t i = 0; i < l_newchild_parent.node->childs.Size; i++)
				{
					if (l_newchild_parent.node->childs[i] == l_newchild.node->parent)
					{
						l_newchild_parent.node->childs.erase_at(i);
						break;
					}
				}
			}

			l_newchild.node->parent = this->scenetree_entry.Index;
			l_current.node->childs.push_back(l_newchild.element->scenetree_entry.Index);

			l_newchild.element->mark_for_recalculation();
		}
		
	}

	inline void set_localposition(const vec3f& p_position)
	{
		if (!EqualsVec(this->transform.local_position, p_position))
		{
			this->mark_for_recalculation();
			this->transform.local_position = p_position;
		}
	};

	inline void set_localrotation(const quat& p_rotation)
	{
		if (!Equals(this->transform.local_rotation, p_rotation))
		{
			this->mark_for_recalculation();
			this->transform.local_rotation = p_rotation;
		}
	};

	inline void set_localscale(const vec3f& p_scale)
	{
		if (!EqualsVec(this->transform.local_scale, p_scale))
		{
			this->mark_for_recalculation();
			this->transform.local_scale = p_scale;
		}
	};

	inline void set_worldposition(const vec3f& p_worldposition)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localposition(p_worldposition);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localposition(mul(l_parent.element->get_worldtolocal(), vec4f(p_worldposition, 1.0f)).Vec3);
		}
	};

	inline void set_worldrotation(const quat& p_worldrotation)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localrotation(p_worldrotation);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localrotation(mul(inv(l_parent.element->get_worldrotation()), p_worldrotation));
		}
	};

	inline void set_worldscale(const vec3f& p_worldscale)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localscale(p_worldscale);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localscale(mul(p_worldscale, inv(l_parent.element->get_worldscalefactor())));
		}
	};

	inline vec3f get_worldposition()
	{
		return translationVector(this->get_localtoworld());
	};

	inline quat get_worldrotation()
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			return this->transform.local_rotation;
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			return mul(l_parent.element->get_worldrotation(), this->transform.local_rotation);
		}
	};

	inline vec3f get_worldscalefactor()
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			return this->transform.local_scale;
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			return mul(l_parent.element->get_worldscalefactor(), this->transform.local_scale);
		}
	};

	inline mat4f& get_localtoworld()
	{
		this->updatematrices_if_necessary();
		return this->localtoworld;
	};

	inline mat4f get_worldtolocal()
	{
		return inv(this->get_localtoworld());
	};

private:
	inline void updatematrices_if_necessary()
	{
		if (this->matrices_mustBe_recalculated)
		{
			this->localtoworld = TRS(this->transform.local_position, extractAxis<float>(this->transform.local_rotation), this->transform.local_scale);
			NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
			if (l_current.node->has_parent())
			{
				NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
				this->localtoworld = mul(l_parent.element->get_localtoworld(), this->localtoworld);
			}
			this->matrices_mustBe_recalculated = false;
		}
	}
};

struct Scene
{
	NTree<SceneNode> tree;

	inline void allocate()
	{
		this->tree.allocate(1);

		auto l_root = this->tree.push_root_value(SceneNode());
		*(this->resolve(l_root).element) = SceneNode(Transform(), &this->tree, com::PoolToken<NTreeNode>(l_root.Index));
	}

	inline void free()
	{
		this->tree.free();
	}

	/*
	inline void frame_start()
	{
		for (size_t i = 0; i < this->tree.Memory.Memory.Size; i++)
		{
			SceneNode& l_node = this->tree.Memory.Memory[i];
			l_node.haschanged_this_frame = false;
		}
	}
	*/

	inline com::PoolToken<SceneNode> allocate_node(const Transform& p_initial_local_transform)
	{
		auto l_node = this->tree.push_value(SceneNode());
		*(this->tree.resolve(l_node).element) = SceneNode(p_initial_local_transform, &this->tree, com::PoolToken<NTreeNode>(l_node.Index));
		return l_node;
	}

	inline NTreeResolve<SceneNode> resolve(const com::PoolToken<SceneNode> p_node)
	{
		return this->tree.resolve(p_node);
	};

	inline NTreeResolve<SceneNode> root()
	{
		return this->resolve(com::PoolToken<SceneNode>(0));
	};
};


void SceneHandle::allocate()
{
	this->handle = new Scene();
	((Scene*)this->handle)->allocate();
};

void SceneHandle::free()
{
	((Scene*)this->handle)->free();
	delete ((Scene*)this->handle);
}
