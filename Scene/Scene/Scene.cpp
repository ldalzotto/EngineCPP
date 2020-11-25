
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
			this->component_heap.realloc((this->component_heap.memory.Size * 2) + l_allocationsize);
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

struct SceneNodeByTag
{
	size_t tag;
	com::Vector<com::PoolToken> nodes;

	inline void allocate(const size_t p_tag)
	{
		this->tag = p_tag;
		this->nodes.allocate(0);
	};
	inline void free() { this->nodes.free(); }
};

struct Scene
{
	NTree<SceneNode> tree;
	SceneHeap heap;

	com::Vector<SceneNodeByTag> nodes_by_tag;

	Callback<void, ComponentAddedParameter> component_added_callback;
	Callback<void, ComponentRemovedParameter> component_removed_callback;
	Callback<void, ComponentAssetPushParameter> component_asset_push_callback;

	inline void allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback, const Callback<void, ComponentRemovedParameter>& p_componentremoved_callback,
		const Callback<void, ComponentAssetPushParameter>& p_componentasset_push_callback)
	{
		this->component_added_callback = p_componentadded_callback;
		this->component_removed_callback = p_componentremoved_callback;
		this->component_asset_push_callback = p_componentasset_push_callback;

		this->tree.allocate(1);

		this->heap.allocate();
		this->nodes_by_tag.allocate(0);

		auto l_root = this->tree.push_root_value(SceneNode());
		*(this->resolve_node(l_root).element) = SceneNode(Math::Transform(), &this->tree, l_root.Index);
	}

	inline void free()
	{
		this->free_node(com::PoolToken(0));

		this->heap.free();
		this->tree.free();

		for (size_t i = 0; i < this->nodes_by_tag.Size; i++)
		{
			this->nodes_by_tag[i].free();
		}
	}

	inline void end_of_frame()
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

	inline com::Vector<NTreeResolve<SceneNode>> get_nodes_with_component(const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		struct GetNodesWithComponentForeach
		{
			com::Vector<NTreeResolve<SceneNode>> nodes;
			Scene* scene;
			const SceneNodeComponent_TypeInfo* component_type;
			inline GetNodesWithComponentForeach() {};
			inline GetNodesWithComponentForeach(Scene* p_scene, const SceneNodeComponent_TypeInfo* p_component_type) {
				this->scene = p_scene;
				this->component_type = p_component_type;
			};
			inline void foreach(NTreeResolve<SceneNode>& p_node)
			{
				com::Vector<SceneNodeComponentToken>& l_node_components = p_node.element->get_components();
				for (size_t i = 0; i < l_node_components.Size; i++)
				{
					if (this->scene->resolve_component(l_node_components[i])->id == this->component_type->id)
					{
						this->nodes.push_back(p_node);
						break;
					}
				}
			};
		};

		GetNodesWithComponentForeach l_foreach = GetNodesWithComponentForeach(this, &p_component_type_info);
		this->tree.traverse(com::PoolToken(0), l_foreach);
		return l_foreach.nodes;
	};

	inline NTreeResolve<SceneNode> root()
	{
		return this->resolve_node(0);
	};

	inline void feed_with_asset(SceneAsset& p_scene_asset)
	{
		com::Vector<com::PoolToken> l_insertednodes_token;
		{
			for (size_t l_node_index = 0; l_node_index < p_scene_asset.nodes.Size; l_node_index++)
			{
				NodeAsset& l_node = p_scene_asset.nodes[l_node_index];

				com::PoolToken l_parent_node = com::PoolToken(0);
				if (l_node.parent != -1)
				{
					l_parent_node = l_insertednodes_token[l_node.parent];
				}

				com::PoolToken l_node_token = this->add_node(l_parent_node, Math::Transform(l_node.local_position, l_node.local_rotation, l_node.local_scale));
				l_insertednodes_token.push_back(l_node_token);

				for (size_t l_component_index = l_node.components_begin; l_component_index < l_node.components_end; l_component_index++)
				{
					ComponentAsset& l_component = p_scene_asset.components[l_component_index];

					ComponentAssetPushParameter l_component_asset_push;
					l_component_asset_push.component_asset = &l_component;
					l_component_asset_push.node = l_node_token;
					l_component_asset_push.scene = this;
					l_component_asset_push.component_asset_object = p_scene_asset.component_asset_heap.map<void>(l_component.componentasset_heap_index);
					this->component_asset_push_callback.call(&l_component_asset_push);

					// this->add_component(l_node_token, SceneNodeComponent_TypeInfo(l_component.id, l_component_heap_chunk.chunk_size), p_scene_asset.component_asset_heap.memory.Memory + l_component_heap_chunk.offset);
				}
			}
		}
		l_insertednodes_token.free();
	};
};




void SceneHandle::allocate(const Callback<void, ComponentAddedParameter>& p_componentadded_callback, const Callback<void, ComponentRemovedParameter>& p_componentremoved_callback,
	const Callback<void, ComponentAssetPushParameter>& p_componentasset_push_callback)
{
	this->handle = new Scene();
	((Scene*)this->handle)->allocate(p_componentadded_callback, p_componentremoved_callback, p_componentasset_push_callback);
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

void SceneHandle::end_of_frame()
{
	return ((Scene*)this->handle)->end_of_frame();
};

void SceneHandle::feed_with_asset(SceneAsset& p_scene_asset)
{
	return ((Scene*)this->handle)->feed_with_asset(p_scene_asset);
};

com::Vector<NTreeResolve<SceneNode>> SceneHandle::get_nodes_with_component(const SceneNodeComponent_TypeInfo& p_component_type_info)
{
	return ((Scene*)this->handle)->get_nodes_with_component(p_component_type_info);
};
