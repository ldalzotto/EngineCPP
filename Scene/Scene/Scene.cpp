
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

struct SceneNode
{
	NTree<SceneNode>* scenetree_ptr;

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

	inline mat4f& get_localtoworld()
	{
		this->updatematrices_if_necessary();
		return this->localtoworld;
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
				mat4f& l_parent_localtoworld = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent)).element->get_localtoworld();
				this->localtoworld = mul(this->localtoworld, l_parent_localtoworld);
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
		NTreeResolve<SceneNode> l_root_reference = this->tree.resolve(l_root);
		*l_root_reference.element = SceneNode(Transform(), &this->tree, com::PoolToken<NTreeNode>(l_root.Index));

		{
			auto l_child = this->tree.push_value(SceneNode());
			auto l_ref = this->tree.resolve(l_child);
			*l_ref.element = SceneNode(Transform(), &this->tree, com::PoolToken<NTreeNode>(l_child.Index));
			l_root_reference.element->addchild(l_child);
		}

		l_root_reference.element->set_localposition(vec3f(1.0f, 0.0f, 0.0f));
	}

	inline void free()
	{
		this->tree.free();
	}

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
