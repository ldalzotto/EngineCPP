
#include "Scene/scene.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"

typedef com::PoolToken<GeneralPurposeHeapMemoryChunk> SceneNodeComponentToken;

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
	inline SceneNodeComponentToken allocate_component(const SceneNodeComponent_TypeInfo& p_type)
	{
		// if(this->component_heap.chunk_total_size <)
		com::PoolToken<GeneralPurposeHeapMemoryChunk> l_memory_allocated;
		if (!this->component_heap.allocate_element<>(p_type.size, &l_memory_allocated))
		{
			this->component_heap.realloc((this->component_heap.chunk_total_size * 2) + p_type.size);
			this->component_heap.allocate_element<>(p_type.size, &l_memory_allocated);
		}

		return l_memory_allocated;
	}

	inline void free_component(SceneNodeComponentToken p_component)
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

	inline void allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback)
	{
		this->component_added_callback = p_componentadded_callback;

		this->tree.allocate(1);

		this->heap.allocate();

		auto l_root = this->tree.push_root_value(SceneNode());
		*(this->resolve_node(l_root).element) = SceneNode(Math::Transform(), &this->tree, com::PoolToken<NTreeNode>(l_root.Index));
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
			this->tree.traverse(com::PoolToken<NTreeNode>(0), NodeDispoeForeach());
		}


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

	inline com::PoolToken<SceneNode> allocate_node(const Math::Transform& p_initial_local_transform)
	{
		auto l_node = this->tree.push_value(SceneNode());
		*(this->tree.resolve(l_node).element) = SceneNode(p_initial_local_transform, &this->tree, com::PoolToken<NTreeNode>(l_node.Index));
		return l_node;
	};

	inline SceneNodeComponentToken allocate_component(const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		return this->heap.allocate_component(p_component_type_info);
	};

	inline SceneNodeComponentToken add_component(const com::PoolToken<SceneNode> p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		SceneNodeComponentToken l_component = this->allocate_component(p_component_type_info);
		NTreeResolve<SceneNode> l_node = this->resolve_node(p_node);
		l_node.element->addcomponent(SceneNodeComponentHandle(l_component.Index));
		ComponentAddedParameter l_param = ComponentAddedParameter(l_node, this->resolve_component(l_component));
		this->component_added_callback.call(&l_param);
		return SceneNodeComponentToken(l_component.Index);
	};

	inline NTreeResolve<SceneNode> resolve_node(const com::PoolToken<SceneNode> p_node)
	{
		return this->tree.resolve(p_node);
	};
	
	inline void* resolve_component(const SceneNodeComponentToken p_component)
	{
		return this->heap.component_heap.resolve<void>(p_component);
	};

	inline NTreeResolve<SceneNode> root()
	{
		return this->resolve_node(com::PoolToken<SceneNode>(0));
	};
};

void SceneHandle::allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback)
{
	this->handle = new Scene();
	((Scene*)this->handle)->allocate(p_componentadded_callback);
};

void SceneHandle::free()
{
	((Scene*)this->handle)->free();
	delete ((Scene*)this->handle);
}


com::PoolToken<SceneNode> SceneHandle::allocate_node(const Math::Transform& p_initial_local_transform)
{
	return ((Scene*)this->handle)->allocate_node(p_initial_local_transform);
};

NTreeResolve<SceneNode> SceneHandle::resolve_node(const com::PoolToken<SceneNode> p_node)
{
	return ((Scene*)this->handle)->resolve_node(p_node);
};

SceneNodeComponentHandle SceneHandle::add_component(const com::PoolToken<SceneNode> p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
{
	return SceneNodeComponentHandle(((Scene*)this->handle)->add_component(p_node, p_component_type_info).Index);
};

void* SceneHandle::resolve_component(const SceneNodeComponentHandle& p_component)
{
	return ((Scene*)this->handle)->resolve_component(SceneNodeComponentToken(p_component.Index));
};