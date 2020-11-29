#pragma once

#include "Scene/scene.hpp"

#define NodeKernel_InputParams SceneNode* thiz, Scene* p_scene
#define NodeKernel_InputValues thiz, p_scene

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

		SceneNodeToken l_root = SceneNodeToken(thiz->tree.push_root_value(SceneNode()).Index);
		allocate_node(resolve_node(thiz, l_root).element, thiz, Math::Transform(), l_root.Index, thiz->node_to_components.alloc_element(com::Vector<SceneNodeComponentToken>()));
	}

	inline static Scene clone(Scene* thiz)
	{
		Scene l_return;

		l_return.tree = thiz->tree.clone();
		l_return.heap = thiz->heap.clone();
		l_return.node_to_components = thiz->node_to_components.clone();

		l_return.component_added_callback = thiz->component_added_callback;
		l_return.component_asset_push_callback = thiz->component_asset_push_callback;
		l_return.component_removed_callback = thiz->component_removed_callback;

		return l_return;
	}

	inline static void free_scene(Scene* thiz)
	{
		free_node(thiz, com::PoolToken(0));

		thiz->heap.free();
		thiz->tree.free();
		for (size_t i = 0; i < thiz->node_to_components.size(); i++)
		{
			thiz->node_to_components[i].free();
		}
		thiz->node_to_components.free();
	};

	


	struct FeedWithAssetInsertionCallbacksDefault
	{
		inline void on_node_added(com::PoolToken p_node_token, NodeAsset& p_node_asset) { };
		inline void on_component_added(com::PoolToken p_node_token, SceneNodeComponentToken p_component_token, ComponentAsset* p_component_asset) {};
	};

	template<class FeedWithAssetInsertionCallbacks = FeedWithAssetInsertionCallbacksDefault>
	inline static void feed_with_asset(Scene* thiz, SceneAsset& p_scene_asset, FeedWithAssetInsertionCallbacks& p_insertion_callbacks = FeedWithAssetInsertionCallbacksDefault())
	{
		com::Vector<SceneNodeToken> l_insertednodes_token;
		{
			for (size_t l_node_index = 0; l_node_index < p_scene_asset.nodes.Size; l_node_index++)
			{
				NodeAsset& l_node = p_scene_asset.nodes[l_node_index];

				SceneNodeToken l_parent_node = SceneNodeToken(0);
				if (l_node.parent != -1)
				{
					l_parent_node = l_insertednodes_token[l_node.parent];
				}

				SceneNodeToken l_node_token = add_node(thiz, l_parent_node, Math::Transform(l_node.local_position, l_node.local_rotation, l_node.local_scale));
				p_insertion_callbacks.on_node_added(l_node_token, l_node);
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
					p_insertion_callbacks.on_component_added(l_node_token, l_component_asset_push.inserted_component, &l_component);
				}
			}
		}
		l_insertednodes_token.free();
	};




	inline static SceneNodeComponentToken allocate_component(Scene* thiz, const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
	{
		return thiz->heap.allocate_component(p_component_type_info, p_initial_value);
	};

	inline static void free_component(Scene* thiz, SceneNodeComponentToken& p_component_token)
	{
		thiz->heap.free_component(p_component_token);
	};

	inline static SceneNodeComponentToken add_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info, void* p_initial_value)
	{
		SceneNodeComponentToken l_component = allocate_component(thiz, p_component_type_info, p_initial_value);
		NTreeResolve<SceneNode> l_node = resolve_node(thiz, p_node);
		thiz->node_to_components[l_node.element->components].push_back(l_component);
		ComponentAddedParameter l_param = ComponentAddedParameter(p_node, l_node, l_component, resolve_component(thiz, l_component));
		thiz->component_added_callback.call(&l_param);
		return l_component;
	};


	template<class ComponentType>
	inline static SceneNodeComponentToken add_component(Scene* thiz, const SceneNodeToken p_node, const ComponentType& p_initialvalue = ComponentType())
	{
		return add_component(thiz, p_node, ComponentType::Type, (void*)&p_initialvalue);
	};

	inline static void remove_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[resolve_node(thiz, p_node).element->components];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = resolve_component(thiz, l_components[i]);
			if (l_component_header->id == p_component_type_info.id)
			{
				remove_component(thiz, p_node, l_components[i]);
				return;
			}
		}
	};

	inline static void remove_component(Scene* thiz, NTreeResolve<SceneNode>& p_node, SceneNodeComponentToken& p_component_token)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[p_node.element->components];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			if (l_components[i].Index == p_component_token.Index)
			{
				l_components.erase_at(i);
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


	inline static SceneNodeComponentHeader* get_component(Scene* thiz, const SceneNodeToken p_node, const SceneNodeComponent_TypeInfo& p_component_type_info)
	{
		com::Vector<SceneNodeComponentToken>& l_components = thiz->node_to_components[resolve_node(thiz, p_node).element->components];
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_component_header = resolve_component(thiz, l_components[i]);
			if (l_component_header->id == p_component_type_info.id)
			{
				return l_component_header;
			};
		}
		return nullptr;
	};

	template<class ComponentType>
	inline static ComponentType* get_component(Scene* thiz, const SceneNodeToken p_node)
	{
		SceneNodeComponentHeader* l_component_header = get_component(thiz, p_node, ComponentType::Type);
		if (l_component_header)
		{
			return l_component_header->cast<ComponentType>();
		}
		return nullptr;
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
				com::Vector<SceneNodeComponentToken>& l_node_components = this->scene->node_to_components[p_node.element->components];
				for (size_t i = 0; i < l_node_components.Size; i++)
				{
					if (SceneKernel::resolve_component(this->scene, l_node_components[i])->id == this->component_type->id)
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









	inline static void allocate_node(NodeKernel_InputParams, const Math::Transform& p_transform, const SceneNodeToken& p_scenetree_entry,
		com::TPoolToken<com::Vector<SceneNodeComponentToken>> p_scenecomponents_token)
	{
		thiz->transform = p_transform;
		thiz->scenetree_entry = p_scenetree_entry;
		thiz->components = p_scenecomponents_token;
		mark_for_recalculation(NodeKernel_InputValues);
	};

	inline static void free_node(NodeKernel_InputParams)
	{
	};

	inline static NTreeResolve<SceneNode> resolve_node(Scene* thiz, const SceneNodeToken p_node)
	{
		return thiz->tree.resolve(p_node);
	};


	inline static SceneNodeToken allocate_node(Scene* thiz, const Math::Transform& p_initial_local_transform)
	{
		SceneNodeToken l_node = SceneNodeToken(thiz->tree.push_value(SceneNode()).Index);
		allocate_node(thiz->tree.resolve(l_node).element, thiz, p_initial_local_transform, l_node.Index, thiz->node_to_components.alloc_element(com::Vector<SceneNodeComponentToken>()));
		return l_node;
	};

	inline static void free_node(Scene* thiz, com::PoolToken& p_node)
	{

		struct RemoveAllComponents
		{
			Scene* scene;

			inline RemoveAllComponents() {};
			inline RemoveAllComponents(Scene* p_scene) { this->scene = p_scene; };

			inline void foreach(NTreeResolve<SceneNode>& p_node)
			{
				com::Vector<SceneNodeComponentToken>& l_components = this->scene->node_to_components.resolve(p_node.element->components);
				for (size_t i = l_components.Size - 1; i < l_components.Size; i--)
				{
					SceneKernel::remove_component(this->scene, p_node, l_components[i]);
				}

				l_components.free();
				this->scene->node_to_components.release_element(p_node.element->components);
				free_node(p_node.element, this->scene);
			};
		};

		thiz->tree.remove(p_node, RemoveAllComponents(thiz));
	};

	inline static SceneNodeToken add_node(Scene* thiz, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform)
	{
		SceneNodeToken l_node = allocate_node(thiz, p_initial_local_transform);
		addchild(resolve_node(thiz, p_parent).element, thiz, l_node);
		return l_node;
	};




	inline static void addchild(NodeKernel_InputParams, SceneNodeToken& p_newchild)
	{
		NTreeResolve<SceneNode> l_current = SceneKernel::resolve_node(p_scene, thiz->scenetree_entry);
		NTreeResolve<SceneNode> l_newchild = SceneKernel::resolve_node(p_scene, p_newchild);

		if (l_newchild.node->parent != thiz->scenetree_entry.Index)
		{
			if (l_newchild.node->has_parent())
			{
				NTreeResolve<SceneNode> l_newchild_parent = SceneKernel::resolve_node(p_scene, l_newchild.node->parent);
				for (size_t i = 0; i < l_newchild_parent.node->childs.Size; i++)
				{
					if (l_newchild_parent.node->childs[i] == l_newchild.node->parent)
					{
						l_newchild_parent.node->childs.erase_at(i);
						break;
					}
				}
			}

			l_newchild.node->parent = thiz->scenetree_entry.Index;
			l_current.node->childs.push_back(l_newchild.element->scenetree_entry.Index);

			mark_for_recalculation(l_newchild.element, p_scene);
		}

	}

	inline static Math::vec3f& get_localposition(SceneNode* thiz)
	{
		return thiz->transform.local_position;
	};

	inline static Math::quat& get_localrotation(SceneNode* thiz)
	{
		return thiz->transform.local_rotation;
	};

	inline static void set_localposition(NodeKernel_InputParams, const Math::vec3f& p_position)
	{
		if (!Math::EqualsVec(thiz->transform.local_position, p_position))
		{
			mark_for_recalculation(NodeKernel_InputValues);
			thiz->transform.local_position = p_position;
		}
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