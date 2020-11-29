#pragma once

#include "component_def.hpp"
#include "Common/Memory/handle.hpp"

#include "serialization.hpp"

#include "Common/Functional/Callback.hpp"
#include "Common/Container/tree.hpp"
#include "Math/transform_def.hpp"
#include "Math/math.hpp"

#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"


struct SceneNodeComponentToken : public com::TPoolToken<GeneralPurposeHeapMemoryChunk>
{
	inline com::TPoolToken<GeneralPurposeHeapMemoryChunk>* cast_to_parent() { return this; }
};


struct SceneNodeComponentHeader
{
	size_t id = -1;
	
	template<class ComponentType>
	inline ComponentType* cast()
	{
		return (ComponentType*)(((char*)this) + sizeof(SceneNodeComponentHeader));
	};
};

struct SceneNodeToken : public com::PoolToken
{
	inline SceneNodeToken() {};
	inline SceneNodeToken(size_t p_index) : com::PoolToken(p_index) {};
	// inline com::TPoolToken<SceneNode>* cast_to_scenenode() { return (com::TPoolToken<SceneNode>*)this; };
	inline com::TPoolToken<NTreeNode>* cast_to_treenode() { return (com::TPoolToken<NTreeNode>*)this; };
};


struct SceneNode
{

	struct State
	{
		bool matrices_mustBe_recalculated = true;
		bool haschanged_thisframe = false;
	} state;

	com::TPoolToken<Optional<com::Vector<SceneNodeComponentToken>>> components;

	//transform
	Math::Transform transform;

	/** This matrix will always be relative to the root Node (a Node without parent). */
	Math::mat4f localtoworld;

	SceneNodeToken scenetree_entry;


public:
	inline SceneNode() {};
};






struct ComponentAddedParameter
{
	SceneNodeToken node_token;
	NTreeResolve<SceneNode> node;
	SceneNodeComponentToken component_token;
	SceneNodeComponentHeader* component;

	ComponentAddedParameter() {};
	inline ComponentAddedParameter(const SceneNodeToken p_node_token, const NTreeResolve<SceneNode>& p_node,const SceneNodeComponentToken& p_component_token, SceneNodeComponentHeader* p_component)
	{
		this->node_token = p_node_token;
		this->node = p_node;
		this->component_token = p_component_token;
		this->component = p_component;
	};
};

struct ComponentRemovedParameter
{
	SceneNodeToken node_token;
	NTreeResolve<SceneNode> node;
	SceneNodeComponentHeader* component;

	ComponentRemovedParameter() {};
	inline ComponentRemovedParameter(const SceneNodeToken p_node_token, const NTreeResolve<SceneNode>& p_node, SceneNodeComponentHeader* p_component)
	{
		this->node_token = p_node_token;
		this->node = p_node;
		this->component = p_component;
	};
};

struct ComponentAssetPushParameter
{
	void* scene;
	SceneNodeToken node;
	ComponentAsset* component_asset;
	void* component_asset_object;
	SceneNodeComponentToken inserted_component;

	ComponentAssetPushParameter() {};
};






struct SceneHeap
{
	GeneralPurposeHeap<> component_heap;

	inline void allocate()
	{
		this->component_heap.allocate(3000); //TODO -> tune
	}

	inline SceneHeap clone()
	{
		SceneHeap l_return;
		l_return.component_heap = this->component_heap.clone();
		return l_return;
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
		SceneNodeComponentToken l_memory_allocated;
		if (!this->component_heap.allocate_element<>(l_allocationsize, l_memory_allocated.cast_to_parent()))
		{
			this->component_heap.realloc((this->component_heap.memory.Size * 2) + l_allocationsize);
			if (!this->component_heap.allocate_element<>(l_allocationsize, l_memory_allocated.cast_to_parent()))
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
	com::OptionalPool<com::Vector<SceneNodeComponentToken>, HeapZeroingAllocator> node_to_components;
	SceneHeap heap;

	Callback<void, ComponentAddedParameter> component_added_callback;
	Callback<void, ComponentRemovedParameter> component_removed_callback;
	Callback<void, ComponentAssetPushParameter> component_asset_push_callback;

	inline void end_of_frame()
	{
		for (size_t i = 0; i < this->tree.Memory.Memory.Size; i++)
		{
			SceneNode& l_node = this->tree.Memory.Memory[i];
			l_node.state.haschanged_thisframe = false;
		}
	}
};
