#pragma once

#include "component_def.hpp"
#include "Common/Memory/handle.hpp"

#include "Common/Functional/Callback.hpp"
#include "Common/Container/tree.hpp"
#include "Math/transform_def.hpp"
#include "Math/math.hpp"

typedef com::PoolToken<size_t> SceneNodeComponentHandle;

struct SceneNode
{
	NTree<SceneNode>* scenetree_ptr;

	// bool haschanged_this_frame;

private:
	//transform
	Math::Transform transform;

	/** This matrix will always be relative to the root Node (a Node without parent). */
	Math::mat4f localtoworld;
	bool matrices_mustBe_recalculated = true;


	//Childs
	com::PoolToken<NTreeNode> scenetree_entry;

	com::Vector<SceneNodeComponentHandle> components;

public:
	inline SceneNode() {};

	inline SceneNode(const Math::Transform& p_transform, NTree<SceneNode>* p_owned_tree, const com::PoolToken<NTreeNode>& p_scenetree_entry)
	{
		this->transform = p_transform;
		this->scenetree_ptr = p_owned_tree;
		this->scenetree_entry = p_scenetree_entry;

		this->mark_for_recalculation();
	}

	inline void free()
	{
		this->components.free();
	};

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

	inline void addcomponent(const SceneNodeComponentHandle p_component)
	{
		this->components.push_back(p_component);
	};

	inline void set_localposition(const Math::vec3f& p_position)
	{
		if (!Math::EqualsVec(this->transform.local_position, p_position))
		{
			this->mark_for_recalculation();
			this->transform.local_position = p_position;
		}
	};

	inline void set_localrotation(const Math::quat& p_rotation)
	{
		if (!Equals(this->transform.local_rotation, p_rotation))
		{
			this->mark_for_recalculation();
			this->transform.local_rotation = p_rotation;
		}
	};

	inline void set_localscale(const Math::vec3f& p_scale)
	{
		if (!Math::EqualsVec(this->transform.local_scale, p_scale))
		{
			this->mark_for_recalculation();
			this->transform.local_scale = p_scale;
		}
	};

	inline void set_worldposition(const Math::vec3f& p_worldposition)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localposition(p_worldposition);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localposition(Math::mul(l_parent.element->get_worldtolocal(), Math::vec4f(p_worldposition, 1.0f)).Vec3);
		}
	};

	inline void set_worldrotation(const Math::quat& p_worldrotation)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localrotation(p_worldrotation);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localrotation(Math::mul(Math::inv(l_parent.element->get_worldrotation()), p_worldrotation));
		}
	};

	inline void set_worldscale(const Math::vec3f& p_worldscale)
	{
		NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			this->set_localscale(p_worldscale);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
			this->set_localscale(Math::mul(p_worldscale, Math::inv(l_parent.element->get_worldscalefactor())));
		}
	};

	inline Math::vec3f get_worldposition()
	{
		return translationVector(this->get_localtoworld());
	};

	inline Math::quat get_worldrotation()
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

	inline Math::vec3f get_worldscalefactor()
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

	inline Math::mat4f& get_localtoworld()
	{
		this->updatematrices_if_necessary();
		return this->localtoworld;
	};

	inline Math::mat4f get_worldtolocal()
	{
		return inv(this->get_localtoworld());
	};

private:
	inline void updatematrices_if_necessary()
	{
		if (this->matrices_mustBe_recalculated)
		{
			this->localtoworld = Math::TRS(this->transform.local_position, Math::extractAxis<float>(this->transform.local_rotation), this->transform.local_scale);
			NTreeResolve<SceneNode> l_current = this->scenetree_ptr->resolve(this->scenetree_entry);
			if (l_current.node->has_parent())
			{
				NTreeResolve<SceneNode> l_parent = this->scenetree_ptr->resolve(com::PoolToken<NTreeNode>(l_current.node->parent));
				this->localtoworld = mul(l_parent.element->get_localtoworld(), this->localtoworld);
			}
			this->matrices_mustBe_recalculated = false;
		}
	};

	inline void mark_for_recalculation()
	{
		struct MarkRecalculationForeach : public NTree<SceneNode>::INTreeForEach<SceneNode>
		{
			inline void foreach(NTreeResolve<SceneNode>& p_resolve)
			{
				p_resolve.element->matrices_mustBe_recalculated = true;
			};
		};

		this->scenetree_ptr->traverse(this->scenetree_entry, MarkRecalculationForeach());
	}
};

struct ComponentAddedParameter
{
	NTreeResolve<SceneNode> node;
	void* component;

	ComponentAddedParameter() {};
	inline ComponentAddedParameter(const NTreeResolve<SceneNode>& p_node, void* p_component)
	{
		this->node = p_node;
		this->component = p_component;
	};
};

struct SceneHandle
{
	void* handle;
	void allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback);
	void free();

	com::PoolToken<SceneNode> allocate_node(const Math::Transform& p_initial_local_transform);
	NTreeResolve<SceneNode> resolve_node(const com::PoolToken<SceneNode> p_node);

	template<class ComponentType>
	ComponentType* resolve_component(const com::PoolToken<ComponentType> p_component)
	{
		return (ComponentType*)resolve_component(SceneNodeComponentHandle(p_component.Index));
	};

	template<class ComponentType>
	inline com::PoolToken<ComponentType> add_component(const com::PoolToken<SceneNode> p_node)
	{
		com::PoolToken<ComponentType> l_component = com::PoolToken<ComponentType>(this->add_component(p_node, ComponentType::Type).Index);
		*this->resolve_component(l_component) = ComponentType();
		return l_component;
	};


private:
	SceneNodeComponentHandle add_component(const com::PoolToken<SceneNode> p_node, const SceneNodeComponent_TypeInfo& p_component_type_info);
	void* resolve_component(const SceneNodeComponentHandle& p_component);
};