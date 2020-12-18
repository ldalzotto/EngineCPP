#pragma once

#include "Scene/scene.hpp"

#define NodeKernel_InputParams SceneNode* thiz, Scene* p_scene
#define NodeKernel_InputValues thiz, p_scene

#if SCENE_BOUND_TEST
#define CHECK_SCENE_BOUND(ConditionFn, b) \
if(ConditionFn == b) \
{ \
	abort(); \
}
#else
#define CHECK_SCENE_BOUND(ConditionFn, b) ConditionFn
#endif

struct SceneKernel
{

	inline static void allocate_scene(Scene* thiz, const Callback<void, ComponentAddedParameter>& p_componentadded_callback, const Callback<void, ComponentRemovedParameter>& p_componentremoved_callback,
		const Callback<void, ComponentAssetPushParameter>& p_componentasset_push_callback)
	{
		thiz->component_added_callback = p_componentadded_callback;
		thiz->component_removed_callback = p_componentremoved_callback;
		thiz->component_asset_push_callback = p_componentasset_push_callback;

		thiz->tree.allocate(1);
		thiz->node_to_components.allocate(1);
		thiz->heap.allocate();

		add_node(thiz, Math::Transform());
	}

	inline static Scene clone(Scene* thiz)
	{
		Scene l_return;

		l_return.tree = thiz->tree.clone();

		l_return.heap = thiz->heap.clone();
		l_return.node_to_components = thiz->node_to_components.clone();

		for (size_t i = 0; i < l_return.node_to_components.size(); i++)
		{
			l_return.node_to_components[i] = l_return.node_to_components[i].clone();
		}

		l_return.component_added_callback = thiz->component_added_callback;
		l_return.component_asset_push_callback = thiz->component_asset_push_callback;
		l_return.component_removed_callback = thiz->component_removed_callback;

		return l_return;
	}

	inline static void free_scene(Scene* thiz)
	{
		remove_node(thiz, com::PoolToken(0));

		thiz->heap.free();
		thiz->tree.free();
		for (size_t i = 0; i < thiz->node_to_components.size(); i++)
		{
			thiz->node_to_components[i].free();
		}
		thiz->node_to_components.free();
	};

	struct FeedWithAsset_AddNodeCallback
	{
		inline SceneNodeToken add_node(Scene* thiz, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform, const size_t p_nodeaddet_index)
		{
			return SceneKernel::add_node(thiz, p_parent, p_initial_local_transform);
		};
	};
	
	template<class AddNode = FeedWithAsset_AddNodeCallback>
	inline static void feed_with_asset(Scene* thiz, SceneAsset& p_scene_asset, SceneNodeToken p_parent_node = SceneNodeToken(0), AddNode& p_add_node = AddNode())
	{
		com::Vector<SceneNodeToken> l_insertednodes_token;
		{
			for (size_t l_node_index = 0; l_node_index < p_scene_asset.nodes.Size; l_node_index++)
			{
				NodeAsset& l_node = p_scene_asset.nodes[l_node_index];

				SceneNodeToken l_parent_node = p_parent_node;
				if (l_node.parent != -1)
				{
					l_parent_node = l_insertednodes_token[l_node.parent];
				}

				SceneNodeToken l_node_token = p_add_node.add_node(thiz, l_parent_node, Math::Transform(l_node.local_position, l_node.local_rotation, l_node.local_scale), l_node_index);
				l_insertednodes_token.push_back(l_node_token);

				for (size_t l_component_index = l_node.components_begin; l_component_index < l_node.components_end; l_component_index++)
				{
					ComponentAsset& l_component = p_scene_asset.components[l_component_index];

					ComponentAssetPushParameter l_component_asset_push;
					l_component_asset_push.component_asset = &l_component;
					l_component_asset_push.node = l_node_token;
					l_component_asset_push.scene = thiz;
					l_component_asset_push.component_asset_object = p_scene_asset.component_asset_heap.map<void>(l_component.componentasset_heap_index);
					thiz->component_asset_push_callback.call(&l_component_asset_push);
				}
			}
		}
		l_insertednodes_token.free();
	};


	inline static void free_component(Scene* thiz, SceneNodeComponentToken& p_component_token)
	{
		thiz->heap.free_component(p_component_token);
	};

	inline static void attach_component_to_node(Scene* thiz, const SceneNodeToken p_node, SceneNodeComponentToken p_component)
	{
		NTreeResolve<SceneNode> l_node = resolve_node(thiz, p_node);
		thiz->node_to_components[*l_node.element->scenetree_entry.cast_to_componentstoken()].push_back(p_component);
		ComponentAddedParameter l_param = ComponentAddedParameter(p_node, l_node, p_component, resolve_component(thiz, p_component));
		thiz->component_added_callback.call(&l_param);
	};

	inline static SceneNodeComponentToken add_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
	{
		SceneNodeComponentToken l_component = thiz->heap.allocate_component(p_component_type_info, p_initial_value);
		attach_component_to_node(thiz, p_node, l_component);
		return l_component;
	};

	template<class ComponentType>
	inline static SceneNodeComponentToken add_component(Scene* thiz, const SceneNodeToken p_node, const ComponentType& p_initialvalue = ComponentType())
	{
		return add_component(thiz, p_node, ComponentType::Type, (void*)&p_initialvalue);
	};

	inline static void remove_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[*resolve_node(thiz, p_node).element->scenetree_entry.cast_to_componentstoken()];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = resolve_component(thiz, l_components[i]);
			if (l_component_header->type->id == p_component_type_info.id)
			{
				remove_component(thiz, p_node, l_components[i]);
				return;
			}
		}
	};

	inline static void remove_component(Scene* thiz, NTreeResolve<SceneNode>& p_node, SceneNodeComponentToken& p_component_token)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[*p_node.element->scenetree_entry.cast_to_componentstoken()];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			if (l_components[i].Index == p_component_token.Index)
			{
				l_components.erase_at(i, 1);
				break;
			}
		}

		ComponentRemovedParameter l_component_removed = ComponentRemovedParameter(SceneNodeToken(p_node.node->index), p_node, resolve_component(thiz, p_component_token));
		thiz->component_removed_callback.call(&l_component_removed);
		free_component(thiz, p_component_token);
	};


	inline static void remove_component(Scene* thiz, const SceneNodeToken p_node, SceneNodeComponentToken& p_component_token)
	{
		NTreeResolve<SceneNode> l_node = resolve_node(thiz, p_node);
		remove_component(thiz, l_node, p_component_token);
	};

	template<class ComponentType>
	inline static void remove_component(Scene* thiz, const com::PoolToken p_node)
	{
		remove_component(thiz, p_node, ComponentType::Type);
	};

	inline static SceneNodeComponentHeader* resolve_component(Scene* thiz, const SceneNodeComponentToken p_component)
	{
		return thiz->heap.component_heap.map<SceneNodeComponentHeader>(p_component);
	};


	inline static com::Vector<SceneNodeComponentToken>& get_components(Scene* thiz, const SceneNodeToken p_node)
	{
		return thiz->node_to_components[*resolve_node(thiz, p_node).element->scenetree_entry.cast_to_componentstoken()];
	};

	inline static bool get_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info, SceneNodeComponentHeader** out_component_header)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[*resolve_node(thiz, p_node).element->scenetree_entry.cast_to_componentstoken()];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = resolve_component(thiz, l_components[i]);
			if (l_component_header->type->id == p_component_type_info.id)
			{
				*out_component_header = l_component_header;
				return true;
			};
		}
		return false;
	};

	template<class ComponentType>
	inline static bool get_component(Scene* thiz, const SceneNodeToken& p_node, ComponentType** out_component)
	{
		SceneNodeComponentHeader* l_component_header;
		if (get_component(thiz, p_node, ComponentType::Type, &l_component_header))
		{
			*out_component = l_component_header->cast<ComponentType>();
			return true;
		};
		return false;
	};

	template<class ComponentType>
	inline static ComponentType& get_component(Scene* thiz, const SceneNodeToken p_node)
	{
		SceneNodeComponentHeader* l_component_header;
		CHECK_SCENE_BOUND(get_component(thiz, p_node, ComponentType::Type, &l_component_header), false);
		return *l_component_header->cast<ComponentType>();
	};


	inline static com::Vector<NTreeResolve<SceneNode>> get_nodes_with_component(Scene* thiz, const SceneNodeComponent_TypeInfo& p_component_type_info)
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
				com::Vector<SceneNodeComponentToken>& l_node_components = this->scene->node_to_components[*p_node.element->scenetree_entry.cast_to_componentstoken()];
				for (size_t i = 0; i < l_node_components.Size; i++)
				{
					if (SceneKernel::resolve_component(this->scene, l_node_components[i])->type->id == this->component_type->id)
					{
						this->nodes.push_back(p_node);
						break;
					}
				}
			};
		};

		GetNodesWithComponentForeach l_foreach = GetNodesWithComponentForeach(thiz, &p_component_type_info);
		thiz->tree.traverse(com::PoolToken(0), l_foreach);
		return l_foreach.nodes;
	};

	template<class ComponentType>
	inline static com::Vector<NTreeResolve<SceneNode>> get_nodes_with_component(Scene* thiz)
	{
		return get_nodes_with_component(thiz, ComponentType::Type);
	};

	inline static NTreeResolve<SceneNode> resolve_node(Scene* thiz, const SceneNodeToken p_node)
	{
#if SCENE_BOUND_TEST
		if (!check_scenetoken_validity(thiz, p_node)) { abort(); }
#endif
		return thiz->tree.resolve(p_node);
	};


	inline static SceneNodeToken add_node(Scene* thiz, const Math::Transform& p_initial_local_transform)
	{
		SceneNodeToken l_node = SceneNodeToken(thiz->tree.push_value(SceneNode()).Index);
		thiz->node_to_components.alloc_element(com::Vector<SceneNodeComponentToken>());
		SceneNode* l_allocated_node = thiz->tree.resolve(l_node).element;
		l_allocated_node->allocate(p_initial_local_transform, l_node.Index, com::MemorySlice<SceneNodeTag>());
		mark_for_recalculation(l_allocated_node, thiz);
		return l_node;
	};

	inline static SceneNodeToken add_node(Scene* thiz, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform)
	{
		SceneNodeToken l_node = add_node(thiz, p_initial_local_transform);
		add_child(resolve_node(thiz, p_parent).element, thiz, l_node);
		return l_node;
	};

	/* Add a new node at the specified p_node. Adding is performed only if the p_node token points to a free Node. */
	inline static bool add_node_at_freenode(Scene* thiz, const SceneNodeToken& p_node, const Math::Transform& p_initial_local_transform)
	{
		const com::TPoolToken<com::Vector<SceneNodeComponentToken>>* p_node_to_component_token = p_node.cast_to_componentstoken();
		if (thiz->tree.set_value_at_freenode(com::TPoolToken<NTreeNode>(p_node.Index), SceneNode()))
		{
			for (size_t i = 0; i < thiz->node_to_components.FreeBlocks.Size; i++)
			{
				if (thiz->node_to_components.FreeBlocks[i] == p_node.Index)
				{
					thiz->node_to_components.FreeBlocks.erase_at(i, 1);
					thiz->node_to_components[*p_node_to_component_token].free();
					break;
				}
			}

			SceneNode* l_free_node = thiz->tree.resolve(p_node).element;
			l_free_node->allocate(p_initial_local_transform, p_node.Index, com::MemorySlice<SceneNodeTag>());
			mark_for_recalculation(l_free_node, thiz);

			return true;
		}

		return false;
	};

	inline static void add_node_at_freenode(Scene* thiz, const SceneNodeToken& p_free_node_token, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform)
	{
		CHECK_SCENE_BOUND(add_node_at_freenode(thiz, p_free_node_token, p_initial_local_transform), false);
		add_child(resolve_node(thiz, p_parent).element, thiz, p_free_node_token);
	};

	/* Add a new node by constraining it's position in the tree memory layout. */
	inline static SceneNodeToken add_node_memoryposition_constrained(Scene* thiz, const Math::Transform& p_initial_local_transform, const size_t p_minimum_pool_position)
	{
		if (thiz->tree.Memory.size() - 1 > p_minimum_pool_position)
		{
			return add_node(thiz, p_initial_local_transform);
		}
		else
		{
			SceneNodeToken l_node_token = SceneNodeToken(p_minimum_pool_position);
			if (!add_node_at_freenode(thiz, l_node_token, p_initial_local_transform))
			{
				return add_node(thiz, p_initial_local_transform);
			}
			return l_node_token;
		}
	};

	inline static SceneNodeToken add_node_memoryposition_constrained(Scene* thiz, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform,
		const size_t p_minimum_treememory_position)
	{
		SceneNodeToken l_node = add_node_memoryposition_constrained(thiz, p_initial_local_transform, p_minimum_treememory_position);
		add_child(resolve_node(thiz, p_parent).element, thiz, l_node);
		return l_node;
	};

	//TODO -> it's the whole tree with "p_node" as root that must be duplciated.
	inline static SceneNodeToken duplicate_node(Scene* thiz, const SceneNodeToken& p_node)
	{
		NTreeResolve<SceneNode> l_node_to_duplicate = SceneKernel::resolve_node(thiz, p_node);
		SceneNodeToken l_node = SceneKernel::add_node(thiz,
			l_node_to_duplicate.node->parent,
			Math::Transform(SceneKernel::get_localposition(l_node_to_duplicate), SceneKernel::get_localrotation(l_node_to_duplicate), SceneKernel::get_localscale(l_node_to_duplicate))
		);

		com::Vector<SceneNodeComponentToken>& l_components_to_duplicate = SceneKernel::get_components(thiz, p_node);
		for (size_t i = 0; i < l_components_to_duplicate.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = SceneKernel::resolve_component(thiz, l_components_to_duplicate[i]);
			
			SceneKernel::add_component(thiz, l_node, *l_component_header->type, l_component_header->get_component_object());
		}
		return l_node;
	}

	inline static void remove_node(Scene* thiz, com::PoolToken& p_node)
	{
		struct RemoveAllComponents
		{
			Scene* scene;

			inline RemoveAllComponents() {};
			inline RemoveAllComponents(Scene* p_scene) { this->scene = p_scene; };

			inline void foreach(NTreeResolve<SceneNode>& p_node)
			{
				com::Vector<SceneNodeComponentToken>& l_components = SceneKernel::get_components(this->scene, p_node.element->scenetree_entry);
				for (size_t i = l_components.Size - 1; i < l_components.Size; i--)
				{
					SceneKernel::remove_component(this->scene, p_node, l_components[i]);
				}

				l_components.free();
				this->scene->node_to_components.release_element(*p_node.element->scenetree_entry.cast_to_componentstoken());
				p_node.element->free();
			};
		};

		thiz->tree.remove(p_node, RemoveAllComponents(thiz));
		p_node.reset();
	};

	inline static void add_child(NodeKernel_InputParams, const SceneNodeToken& p_newchild)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		NTreeResolve<SceneNode> l_newchild = SceneKernel::resolve_node(p_scene, p_newchild);

		if (l_newchild.node->parent != thiz->scenetree_entry.Index)
		{
			if (l_newchild.node->has_parent())
			{
				NTreeResolve<SceneNode> l_newchild_parent = SceneKernel::resolve_node(p_scene, l_newchild.node->parent);
				com::Vector<com::TPoolToken<NTreeNode>>& l_newchild_parent_childs = p_scene->tree.get_childs(l_newchild_parent);
				for (size_t i = 0; i < l_newchild_parent_childs.Size; i++)
				{
					if (l_newchild_parent_childs[i].Index == l_newchild.node->parent)
					{
						l_newchild_parent_childs.erase_at(i, 1);
						break;
					}
				}
			}

			l_newchild.node->parent = thiz->scenetree_entry.Index;
			p_scene->tree.get_childs(l_current).push_back(*l_newchild.element->scenetree_entry.cast_to_treenode());

			mark_for_recalculation(l_newchild.element, p_scene);
		}
	};

	inline static void add_child(Scene* thiz, const SceneNodeToken& p_parent, const SceneNodeToken& p_newchild)
	{
		add_child(resolve_node(thiz, p_parent).element, thiz, p_newchild);
	}

	struct SceneIterationFilter_Default { inline bool evaluate(NTreeResolve<SceneNode>& p_node) { return true; }; };

	template<class SceneInteratorFilter = SceneIterationFilter_Default>
	struct SceneNodeForeach
	{
		SceneInteratorFilter filter;

		inline SceneNodeForeach(SceneInteratorFilter& p_filter = SceneIterationFilter_Default())
		{
			this->filter = p_filter;
		};

		inline void foreach(NTreeResolve<SceneNode>& p_node) {
			if (this->filter.evaluate(p_node))
			{
				this->foreach_internal(p_node);
			}
		};

		virtual void foreach_internal(NTreeResolve<SceneNode>& p_node) = 0;
	};

	template<class SceneNodeForeach>
	inline static void traverse(Scene* thiz, const SceneNodeToken& p_start_node, SceneNodeForeach& p_foreach)
	{
		thiz->tree.traverse(com::PoolToken(p_start_node.Index), p_foreach);
	};

	inline static bool check_scenetoken_validity(Scene* thiz, const SceneNodeToken& p_node)
	{
		if (p_node.Index >= thiz->tree.Memory.size())
		{
			return false;
		};

		for (size_t i = 0; i < thiz->tree.Memory.FreeBlocks.Size; i++)
		{
			if (thiz->tree.Memory.FreeBlocks[i] == p_node.Index)
			{
				return false;
			}
		}
		return true;
	};
	
	inline static void add_tag(SceneNode* thiz, const SceneNodeTag& p_tag)
	{
		thiz->tags.push_back(p_tag);
	};

	inline static void add_tag(Scene* p_scene, SceneNodeToken p_node, const SceneNodeTag& p_tag)
	{
		add_tag(resolve_node(p_scene, p_node).element, p_tag);
	};

	inline static void remove_tag(SceneNode* thiz, const SceneNodeTag& p_tag)
	{
		for (size_t i = 0; i < thiz->tags.Size; i++)
		{
			if (thiz->tags[i].hash == p_tag.hash)
			{
				thiz->tags.erase_at(i, 1);
				break;
			}
		}
	};

	inline static bool contains_tag(SceneNode* thiz, const SceneNodeTag& p_tag)
	{
		for (size_t i = 0; i < thiz->tags.Size; i++)
		{
			if (thiz->tags[i].hash == p_tag.hash)
			{
				return true;
			}
		}
		return false;
	};

	inline static Math::vec3f& get_localposition(SceneNode* thiz)
	{
		return thiz->transform.local_position;
	};

	inline static Math::vec3f& get_localposition(NTreeResolve<SceneNode>& thiz)
	{
		return get_localposition(thiz.element);
	};

	inline static Math::vec3f& get_localposition(SceneNodeToken p_node, Scene* p_scene)
	{
		return get_localposition(resolve_node(p_scene, p_node));
	};

	inline static Math::quat& get_localrotation(SceneNode* thiz)
	{
		return thiz->transform.local_rotation;
	}; 
	
	inline static Math::quat& get_localrotation(NTreeResolve<SceneNode>& thiz)
	{
		return get_localrotation(thiz.element);
	};

	inline static Math::quat& get_localrotation(SceneNodeToken p_node, Scene* p_scene)
	{
		return get_localrotation(resolve_node(p_scene, p_node));
	};

	inline static Math::vec3f& get_localscale(SceneNode* thiz)
	{
		return thiz->transform.local_scale;
	};

	inline static Math::vec3f& get_localscale(NTreeResolve<SceneNode>& thiz)
	{
		return get_localscale(thiz.element);
	};

	inline static Math::vec3f& get_localscale(SceneNodeToken p_node, Scene* p_scene)
	{
		return get_localscale(resolve_node(p_scene, p_node));
	};

	inline static void set_localposition(NodeKernel_InputParams, const Math::vec3f& p_position)
	{
		if (!Math::EqualsVec(thiz->transform.local_position, p_position))
		{
			mark_for_recalculation(NodeKernel_InputValues);
			thiz->transform.local_position = p_position;
		}
	};

	inline static void set_localposition(SceneNodeToken p_node, Scene* p_scene, const Math::vec3f& p_position)
	{
		set_localposition(resolve_node(p_scene, p_node).element, p_scene, p_position);
	};

	inline static void set_localrotation(NodeKernel_InputParams, const Math::quat& p_rotation)
	{
		if (!Equals(thiz->transform.local_rotation, p_rotation))
		{
			mark_for_recalculation(NodeKernel_InputValues);
			thiz->transform.local_rotation = p_rotation;
		}
	};

	inline static void set_localscale(NodeKernel_InputParams, const Math::vec3f& p_scale)
	{
		if (!Math::EqualsVec(thiz->transform.local_scale, p_scale))
		{
			mark_for_recalculation(NodeKernel_InputValues);
			thiz->transform.local_scale = p_scale;
		}
	};

	inline static void set_worldposition(NodeKernel_InputParams, const Math::vec3f& p_worldposition)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			set_localposition(NodeKernel_InputValues, p_worldposition);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = p_scene->tree.resolve(l_current.node->parent);
			set_localposition(NodeKernel_InputValues, Math::mul(get_worldtolocal(l_parent.element, p_scene), Math::vec4f(p_worldposition, 1.0f)).Vec3);
		}
	};

	inline static void set_worldposition(SceneNodeToken p_node, Scene* p_scene, const Math::vec3f& p_worldposition)
	{
		set_worldposition(resolve_node(p_scene, p_node).element, p_scene, p_worldposition);
	};

	inline static void set_worldrotation(NodeKernel_InputParams, const Math::quat& p_worldrotation)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			set_localrotation(NodeKernel_InputValues, p_worldrotation);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = SceneKernel::resolve_node(p_scene, l_current.node->parent);
			set_localrotation(NodeKernel_InputValues, Math::mul(Math::inv(get_worldrotation(l_parent.element, p_scene)), p_worldrotation));
		}
	};

	inline static void set_worldrotation(SceneNodeToken p_node, Scene* p_scene, const Math::quat& p_worldrotation)
	{
		set_worldrotation(resolve_node(p_scene, p_node).element, p_scene, p_worldrotation);
	};

	inline static void set_worldscale(NodeKernel_InputParams, const Math::vec3f& p_worldscale)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			set_localscale(NodeKernel_InputValues, p_worldscale);
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = SceneKernel::resolve_node(p_scene, l_current.node->parent);
			set_localscale(NodeKernel_InputValues, Math::mul(p_worldscale, Math::inv(get_worldscalefactor(l_parent.element, p_scene))));
		}
	};

	inline static Math::vec3f get_worldposition(NodeKernel_InputParams)
	{
		return translationVector(get_localtoworld(NodeKernel_InputValues));
	};

	inline static Math::vec3f get_worldposition(NTreeResolve<SceneNode>& p_node, Scene* p_scene)
	{
		return get_worldposition(p_node.element, p_scene);
	};

	inline static Math::quat get_worldrotation(NodeKernel_InputParams)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			return thiz->transform.local_rotation;
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = SceneKernel::resolve_node(p_scene, l_current.node->parent);
			return mul(get_worldrotation(l_parent.element, p_scene), thiz->transform.local_rotation);
		}
	};

	inline static Math::quat get_worldrotation(NTreeResolve<SceneNode>& p_node, Scene* p_scene)
	{
		return get_worldrotation(p_node.element, p_scene);
	};

	inline static Math::vec3f get_worldscalefactor(NodeKernel_InputParams)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		if (!l_current.node->has_parent())
		{
			return thiz->transform.local_scale;
		}
		else
		{
			NTreeResolve<SceneNode> l_parent = SceneKernel::resolve_node(p_scene, l_current.node->parent);
			return mul(get_worldscalefactor(l_parent.element, p_scene), thiz->transform.local_scale);
		}
	};

	inline static Math::mat4f& get_localtoworld(NodeKernel_InputParams)
	{
		updatematrices_if_necessary(NodeKernel_InputValues);
		return thiz->localtoworld;
	};

	inline static Math::mat4f get_worldtolocal(NodeKernel_InputParams)
	{
		return inv(get_localtoworld(NodeKernel_InputValues));
	};

private:
	inline static void updatematrices_if_necessary(NodeKernel_InputParams)
	{
		if (thiz->state.matrices_mustBe_recalculated)
		{
			thiz->localtoworld = Math::TRS(thiz->transform.local_position, Math::extractAxis<float>(thiz->transform.local_rotation), thiz->transform.local_scale);
			NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
			if (l_current.node->has_parent())
			{
				NTreeResolve<SceneNode> l_parent = SceneKernel::resolve_node(p_scene, l_current.node->parent);
				thiz->localtoworld = mul(get_localtoworld(l_parent.element, p_scene), thiz->localtoworld);
			}
			thiz->state.matrices_mustBe_recalculated = false;
		}
	};

	inline static void mark_for_recalculation(NodeKernel_InputParams)
	{
		struct MarkRecalculationForeach : public NTree<SceneNode>::INTreeForEach<SceneNode>
		{
			inline void foreach(NTreeResolve<SceneNode>& p_resolve)
			{
				p_resolve.element->state.matrices_mustBe_recalculated = true;
				p_resolve.element->state.haschanged_thisframe = true;
			};
		};

		p_scene->tree.traverse(thiz->scenetree_entry, MarkRecalculationForeach());
	}

};