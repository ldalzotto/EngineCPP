
#include "Scene/scene.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"

typedef com::PoolToken SceneNodeComponentToken;

struct SceneHeap
{
	GeneralPurposeHeap<> component_heap;

	inline void allocate()
	{
		this->component_heap.allocate(3000); //TODO -> tune
	}

	inline void free()
	{
		this->component_heap.dispose();
	}

	//store components ?
	inline SceneNodeComponentToken allocate_component(const SceneNodeComponent_TypeInfo& p_type, void* p_initial_value)
	{
		// if(this->component_heap.chunk_total_size <)
		size_t l_allocationsize = sizeof(SceneNodeComponentHeader) + p_type.size;
		com::PoolToken l_memory_allocated;
		if (!this->component_heap.allocate_element<>(l_allocationsize, &l_memory_allocated))
		{
			this->component_heap.realloc((this->component_heap.chunk_total_size * 2) + l_allocationsize);
			if (!this->component_heap.allocate_element<>(l_allocationsize, &l_memory_allocated))
			{
				abort();
			};
		}

		SceneNodeComponentHeader* l_header = this->component_heap.map<SceneNodeComponentHeader>(l_memory_allocated);
		l_header->id = p_type.id;
		memcpy((char*)l_header + sizeof(SceneNodeComponentHeader), p_initial_value, p_type.size);

		return l_memory_allocated;
	}

	inline void free_component(SceneNodeComponentToken& p_component)
	{
		this->component_heap.release_element(p_component);
		p_component.Index = -1;
	}
};

struct Scene
{
	NTree<SceneNode> tree;
	SceneHeap heap;
	Callback<void, ComponentAddedParameter> component_added_callback;
	Callback<void, ComponentRemovedParameter> component_removed_callback;

	inline void allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback, const Callback<void, ComponentRemovedParameter>& p_componentremoved_callback)
	{
		this->component_added_callback = p_componentadded_callback;
		this->component_removed_callback = p_componentremoved_callback;

		this->tree.allocate(1);

		this->heap.allocate();

		auto l_root = this->tree.push_root_value(SceneNode());
		*(this->resolve_node(l_root).element) = SceneNode(Math::Transform(), &this->tree, l_root.Index);
	}

	inline void free()
	{
		this->heap.free();

		if (this->tree.Memory.size() > 0)
		{
			struct NodeDispoeForeach : public NTree<SceneNode>::INTreeForEach<SceneNode>
			{
				inline void foreach(NTreeResolve<SceneNode>& p_resolve)
				{
					p_resolve.element->free();
				};
			};
			this->tree.traverse(com::PoolToken(0), NodeDispoeForeach());
		}


		this->tree.free();
	}

	inline void new_frame()
	{
		for (size_t i = 0; i < this->tree.Memory.Memory.Size; i++)
		{
			SceneNode& l_node = this->tree.Memory.Memory[i];
			l_node.state.haschanged_thisframe = false;
		}
	}

	inline com::PoolToken allocate_node(const Math::Transform& p_initial_local_transform)
	{
		auto l_node = this->tree.push_value(SceneNode());
		*(this->tree.resolve(l_node).element) = SceneNode(p_initial_local_transform, &this->tree, l_node.Index);
		return l_node;
	};

	inline void free_node(com::PoolToken& p_node)
	{

		struct RemoveAllComponents
		{
			Scene* scene;

			inline RemoveAllComponents() {};
			inline RemoveAllComponents(Scene* p_scene) { this->scene = p_scene; };

			inline void foreach(NTreeResolve<SceneNode>& p_node)
			{
				com::Vector<SceneNodeComponentHandle>& l_components = p_node.element->get_components();
				for (size_t i = l_components.Size - 1; i < l_components.Size; i--)
				{
					this->scene->remove_component(p_node, l_components[i]);
				}

				p_node.element->free();
			};
		};

		this->tree.remove(p_node, RemoveAllComponents(this));
	};

	inline com::PoolToken add_node(const com::PoolToken& p_parent, const Math::Transform& p_initial_local_transform)
	{
		auto l_node = this->allocate_node(p_initial_local_transform);
		this->tree.resolve(p_parent).element->addchild(l_node);
		return l_node;
	};

	inline SceneNodeComponentToken allocate_component(const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
	{
		return this->heap.allocate_component(p_component_type_info, p_initial_value);
	};

	inline void free_component(SceneNodeComponentToken& p_component_token)
	{
		this->heap.free_component(p_component_token);
	};

	inline SceneNodeComponentToken add_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
	{
		SceneNodeComponentToken l_component = this->allocate_component(p_component_type_info, p_initial_value);
		NTreeResolve<SceneNode> l_node = this->resolve_node(p_node);
		l_node.element->addcomponent(SceneNodeComponentHandle(l_component.Index));
		ComponentAddedParameter l_param = ComponentAddedParameter(p_node, l_node, this->resolve_component(l_component));
		this->component_added_callback.call(&l_param);
		return SceneNodeComponentToken(l_component.Index);
	};

	inline void remove_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		com::Vector<SceneNodeComponentHandle>& l_components = this->resolve_node(p_node).element->get_components();
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = this->resolve_component(l_components[i]);
			if (l_component_header->id == p_component_type_info.id)
			{
				this->remove_component(p_node, l_components[i]);
				return;
			}
		}
	};

	inline void remove_component(NTreeResolve<SceneNode>& p_node, SceneNodeComponentToken& p_component_token)
	{
		p_node.element->removecomponent(SceneNodeComponentHandle(p_component_token));
		ComponentRemovedParameter l_component_removed = ComponentRemovedParameter(p_node.node->index, p_node, this->resolve_component(p_component_token));
		this->component_removed_callback.call(&l_component_removed);
		this->free_component(p_component_token);
	};


	inline void remove_component(const com::PoolToken p_node, SceneNodeComponentToken& p_component_token)
	{
		NTreeResolve<SceneNode> l_node = this->resolve_node(p_node);
		this->remove_component(l_node, p_component_token);
	};

	inline NTreeResolve<SceneNode> resolve_node(const com::PoolToken p_node)
	{
		return this->tree.resolve(p_node);
	};

	inline SceneNodeComponentHeader* resolve_component(const SceneNodeComponentToken p_component)
	{
		return this->heap.component_heap.map<SceneNodeComponentHeader>(p_component);
	};

	inline SceneNodeComponentHeader* get_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		com::Vector<SceneNodeComponentHandle>& l_components = this->resolve_node(p_node).element->get_components();
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = this->resolve_component(l_components[i]);
			if (l_component_header->id == p_component_type_info.id)
			{
				return l_component_header;
			};
		}
		return nullptr;
	};

	inline NTreeResolve<SceneNode> root()
	{
		return this->resolve_node(0);
	};
};

void SceneHandle::allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback, const Callback<void, ComponentRemovedParameter>& p_componentremoved_callback)
{
	this->handle = new Scene();
	((Scene*)this->handle)->allocate(p_componentadded_callback, p_componentremoved_callback);
};

void SceneHandle::free()
{
	((Scene*)this->handle)->free();
	delete ((Scene*)this->handle);
}


com::PoolToken SceneHandle::allocate_node(const Math::Transform& p_initial_local_transform)
{
	return ((Scene*)this->handle)->allocate_node(p_initial_local_transform);
};

void SceneHandle::free_node(com::PoolToken& p_node)
{
	((Scene*)this->handle)->free_node(p_node);
};


com::PoolToken SceneHandle::add_node(const com::PoolToken& p_parent, const Math::Transform& p_initial_local_transform)
{
	return ((Scene*)this->handle)->add_node(p_parent, p_initial_local_transform);
};

NTreeResolve<SceneNode> SceneHandle::resolve_node(const com::PoolToken p_node)
{
	return ((Scene*)this->handle)->resolve_node(p_node);
};

SceneNodeComponentHandle SceneHandle::add_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
{
	return SceneNodeComponentHandle(((Scene*)this->handle)->add_component(p_node, p_component_type_info, p_initial_value).Index);
};

void SceneHandle::remove_component(const com::PoolToken p_node, SceneNodeComponentHandle& p_component)
{
	((Scene*)this->handle)->remove_component(p_node, p_component);
};

void SceneHandle::remove_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
{
	((Scene*)this->handle)->remove_component(p_node, p_component_type_info);
};

SceneNodeComponentHeader* SceneHandle::get_component(const com::PoolToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
{
	return ((Scene*)this->handle)->get_component(p_node, p_component_type_info);
};

SceneNodeComponentHeader* SceneHandle::resolve_componentheader(const SceneNodeComponentHandle& p_component)
{
	return ((Scene*)this->handle)->resolve_component(SceneNodeComponentToken(p_component.Index));
};

com::PoolToken SceneHandle::root()
{
	return 0;
};

void SceneHandle::new_frame()
{
	return ((Scene*)this->handle)->new_frame();
};