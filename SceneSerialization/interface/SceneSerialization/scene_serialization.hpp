#pragma once

#include "SceneComponents/components.hpp"
#include "Scene/assets.hpp"
#include "Scene/scene.hpp"
#include "Scene/kernel/scene.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Functional/Hash.hpp"
#include "Common/Serialization/binary.hpp"
#include "Math/vector_def.hpp"
#include "Math/serialization.hpp"
#include "Math/quaternion_def.hpp"
#include "AssetServer/asset_server.hpp"


struct ComponentSerializationConstants
{
	struct MeshRenderer
	{
		inline static const char* Mesh = "mesh";
		inline static const char* Material = "material";
	};
	
	struct Camera
	{
		inline static const char* Fov = "fov";
		inline static const char* Near = "near";
		inline static const char* Far = "far";
	};
};

struct SceneSerializationConstants
{
	inline static const char* NodesField = "nodes";
	inline static const char* SceneType = "scene";
	inline static const char* ComponentType = "type";
	inline static const char* ComponentObject = "object";

	struct Node
	{
		inline static const char* LocalPosition = "local_position";
		inline static const char* LocalRotation = "local_rotation";
		inline static const char* LocalScale = "local_scale";
		inline static const char* Components = "components";
		inline static const char* Childs = "childs";
	};
};

template<>
struct JSONDeserializer<MeshRendererAsset>
{
	inline static MeshRendererAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		MeshRendererAsset l_asset;
		p_iterator.next_field(ComponentSerializationConstants::MeshRenderer::Mesh);
		String<> l_mesh_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.mesh = Hash<StringSlice>::hash(l_mesh_str.toSlice());
		l_mesh_str.free();

		p_iterator.next_field(ComponentSerializationConstants::MeshRenderer::Material);
		String<> l_material_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.material = Hash<StringSlice>::hash(l_material_str.toSlice());
		l_material_str.free();

		return l_asset;
	};
};

template<>
struct JSONSerializer<MeshRendererAsset>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const MeshRendererAsset& p_object, AssetServerHandle& p_asset_server)
	{
		String<> l_path_tmp;
		l_path_tmp = p_asset_server.get_path_from_resourcehash(p_object.mesh);
		p_serializer.push_field(ComponentSerializationConstants::MeshRenderer::Mesh, l_path_tmp.toSlice());
		l_path_tmp.free();
		l_path_tmp = p_asset_server.get_path_from_resourcehash(p_object.material);
		p_serializer.push_field(ComponentSerializationConstants::MeshRenderer::Material, l_path_tmp.toSlice());
		l_path_tmp.free();
	};
};

template<>
struct JSONDeserializer<CameraAsset>
{
	inline static CameraAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		CameraAsset l_asset;
		p_iterator.next_field(ComponentSerializationConstants::Camera::Fov, &l_asset.fov);
		p_iterator.next_field(ComponentSerializationConstants::Camera::Near, &l_asset.near_);
		p_iterator.next_field(ComponentSerializationConstants::Camera::Far, &l_asset.far_);

		p_iterator.free();
		return l_asset;
	};
};

template<>
struct JSONSerializer<CameraAsset>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const CameraAsset& p_object)
	{
		p_serializer.push_field(ComponentSerializationConstants::Camera::Fov, p_object.fov);
		p_serializer.push_field(ComponentSerializationConstants::Camera::Near, p_object.near_);
		p_serializer.push_field(ComponentSerializationConstants::Camera::Far, p_object.far_);
	};
};




struct ComponentAssetSerializer
{
	inline static bool deserializeJSON(StringSlice& p_component_type, Deserialization::JSON::JSONObjectIterator& p_component_object_iterator,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap, ComponentAsset* out_component_asset)
	{
		if (p_component_type.equals(MeshRenderer::TypeName))
		{
			out_component_asset->id = MeshRenderer::Id;

			allocate_component_asset<MeshRendererAsset>(p_compoent_asset_heap, &out_component_asset->componentasset_heap_index);

			MeshRendererAsset* l_mesh_renderer_asset = p_compoent_asset_heap.map<MeshRendererAsset>(out_component_asset->componentasset_heap_index);
			*l_mesh_renderer_asset = JSONDeserializer<MeshRendererAsset>::deserialize(p_component_object_iterator);

			return true;
		}
		else if (p_component_type.equals(Camera::TypeName))
		{
			out_component_asset->id = Camera::Id;
			allocate_component_asset<CameraAsset>(p_compoent_asset_heap, &out_component_asset->componentasset_heap_index);

			CameraAsset* l_camera_asset = p_compoent_asset_heap.map<CameraAsset>(out_component_asset->componentasset_heap_index);
			*l_camera_asset = JSONDeserializer<CameraAsset>::deserialize(p_component_object_iterator);

			return true;
		}

		return false;
	};

	inline static void serializeJSON(ComponentAsset& p_component_asset, Serialization::JSON::Deserializer& p_serializer, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap, AssetServerHandle p_asset_server)
	{
		StringSlice l_component_type;
		bool l_component_detected = false;
		if (p_component_asset.id == MeshRenderer::Id)
		{
			l_component_type = StringSlice(MeshRenderer::TypeName);
			l_component_detected = true;
		}
		else if (p_component_asset.id == Camera::Id)
		{
			l_component_type = StringSlice(Camera::TypeName);
			l_component_detected = true;
		}

		if (l_component_detected)
		{
			void* l_component = p_compoent_asset_heap.map<void>(p_component_asset.componentasset_heap_index);

			p_serializer.push_field(SceneSerializationConstants::ComponentType, l_component_type);
			p_serializer.start_object(SceneSerializationConstants::ComponentObject);

			switch (p_component_asset.id)
			{
			case MeshRenderer::Id:
			{
				JSONSerializer<MeshRendererAsset>::serialize(p_serializer, *(MeshRendererAsset*)l_component, p_asset_server);
			}
			break;
			case Camera::Id:
			{
				JSONSerializer<CameraAsset>::serialize(p_serializer, *(CameraAsset*)l_component);
			}
			break;
			}

			p_serializer.end_object();
		}
	};

	template<class WithComponent>
	inline static void ComponentAsset_to_Component(ComponentAsset* p_component_asset, void* p_component_asset_object, WithComponent& p_with_component)
	{
		switch (p_component_asset->id)
		{
		case MeshRenderer::Id:
		{
			MeshRendererAsset* l_asset = (MeshRendererAsset*)p_component_asset_object;
			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.initialize(l_asset->material, l_asset->mesh);
			p_with_component.with_component(&l_mesh_renderer, MeshRenderer::Type);
		}
		break;
		case Camera::Id:
		{
			CameraAsset* l_asset = (CameraAsset*)p_component_asset_object;
			Camera l_camera = Camera(*l_asset);
			p_with_component.with_component(&l_camera, Camera::Type);
		}
		break;
		}
	};

	inline static bool Component_to_ComponentAsset(SceneNodeComponentHeader* p_component_header, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap, ComponentAsset* out_componentasset)
	{
		switch (p_component_header->id)
		{
		case MeshRenderer::Id:
		{
			MeshRenderer* l_mesh_renderer = p_component_header->cast<MeshRenderer>();
			allocate_component_asset<MeshRendererAsset>(p_compoent_asset_heap, &out_componentasset->componentasset_heap_index);
			MeshRendererAsset* l_meshrenderer_asset = p_compoent_asset_heap.map<MeshRendererAsset>(out_componentasset->componentasset_heap_index);
			l_meshrenderer_asset->material = l_mesh_renderer->material.key;
			l_meshrenderer_asset->mesh = l_mesh_renderer->model.key;	
			out_componentasset->id = MeshRenderer::Id;
			return true;
		}
		break;
		case Camera::Id:
		{
			Camera* l_camera = p_component_header->cast<Camera>();
			allocate_component_asset<Camera>(p_compoent_asset_heap, &out_componentasset->componentasset_heap_index);
			CameraAsset* l_camera_asset = p_compoent_asset_heap.map<CameraAsset>(out_componentasset->componentasset_heap_index);
			l_camera_asset->far_ = l_camera->far_;
			l_camera_asset->fov = l_camera->fov;
			l_camera_asset->near_ = l_camera->near_;
			out_componentasset->id = Camera::Id;
			return true;
		}
		break;
		}

		return false;
	};

private:
	template<class ComponentAssetType>
	inline static bool allocate_component_asset(GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap, com::PoolToken* out_index)
	{
		return p_compoent_asset_heap.allocate_element(sizeof(ComponentAssetType), out_index);
	};
};


struct SceneSerializer2
{
	inline static SceneAsset JSON_to_SceneAsset(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		SceneAsset l_asset;
		l_asset.allocate();

		p_iterator.next_field("type");

		Deserialization::JSON::JSONObjectIterator l_node_array_iterator;
		Deserialization::JSON::JSONObjectIterator l_node_iterator;
		p_iterator.next_array(SceneSerializationConstants::NodesField, &l_node_array_iterator);
		while (l_node_array_iterator.next_array_object(&l_node_iterator))
		{
			JSONSceneNode_pushTo_SceneAsset(l_node_iterator, -1, l_asset.nodes, l_asset.components, l_asset.component_asset_heap);
		}

		p_iterator.free();

		return l_asset;
	};

	inline static com::Vector<char> JSON_to_Binary(const com::Vector<char>& p_json_scene)
	{
		String<> p_json_scene_as_string;
		p_json_scene_as_string.Memory = p_json_scene;
		Deserialization::JSON::JSONObjectIterator l_scene_root_json = Deserialization::JSON::StartDeserialization(p_json_scene_as_string);
		SceneAsset l_scene_asset = SceneSerializer2::JSON_to_SceneAsset(l_scene_root_json);

		com::Vector<char> l_binary_scene;
		l_scene_asset.serialize(l_binary_scene);

		l_scene_asset.free();

		return l_binary_scene;
	};

	inline static com::Vector<char> SceneAsset_to_JSON(SceneAsset& p_scene_asset, AssetServerHandle p_asset_server)
	{
		Serialization::JSON::Deserializer l_json_seralizer;
		l_json_seralizer.allocate();

		l_json_seralizer.start();
		l_json_seralizer.push_field("type", StringSlice(SceneSerializationConstants::SceneType));
		l_json_seralizer.start_array(SceneSerializationConstants::NodesField);

		for (size_t i = 0; i < p_scene_asset.nodes.Size; i++)
		{
			if (p_scene_asset.nodes[i].parent == -1)
			{
				l_json_seralizer.start_object();
				NodeAsset_to_JSONSceneNode(l_json_seralizer, p_scene_asset.nodes[i], p_scene_asset, p_asset_server);
				l_json_seralizer.end_object();
			}
		}

		l_json_seralizer.end_array();
		l_json_seralizer.end();

		return l_json_seralizer.output.Memory;
	};

	template<class SceneNodeFilter = SceneKernel::SceneIterationFilter_Default>
	inline static SceneAsset Scene_to_SceneAsset(Scene& p_scene, SceneNodeFilter& p_scenenode_filer = SceneKernel::SceneIterationFilter_Default())
	{
		com::Vector<NTreeResolve<SceneNode>> l_first_scenetree_childs;

		com::Vector<com::TPoolToken<NTreeNode>>& l_first_scenetree_childs_raw = p_scene.tree.get_childs(p_scene.tree.resolve(com::PoolToken(0)));
		for (size_t i = 0; i < l_first_scenetree_childs_raw.Size; i++)
		{
			l_first_scenetree_childs.push_back(p_scene.tree.resolve(l_first_scenetree_childs_raw[i]));
		}

		SceneAsset l_scene_asset;


		for (size_t i = 0; i < l_first_scenetree_childs.Size; i++)
		{
			SceneNode_pushTo_sceneAsset(l_first_scenetree_childs[i], -1, p_scene, l_scene_asset, p_scenenode_filer);
		}

		l_first_scenetree_childs.free();
		return l_scene_asset;
	};

	template<class SceneNodeFilter = SceneKernel::SceneIterationFilter_Default>
	inline static SceneAsset SceneSingleNode_to_SceneAsset(Scene& p_scene, SceneNodeToken& p_node_included, SceneNodeFilter& p_scenenode_filer = SceneKernel::SceneIterationFilter_Default())
	{
		SceneAsset l_scene_asset;
		SceneNode_pushTo_sceneAsset(SceneKernel::resolve_node(&p_scene, p_node_included), -1, p_scene, l_scene_asset, p_scenenode_filer);
		return l_scene_asset;
	};
	
	template<class SceneNodeFilter = SceneKernel::SceneIterationFilter_Default>
	inline static void SceneSingleNode_to_SceneAsset(Scene& p_scene, SceneNodeToken& p_node_included, SceneAsset* out_sceneasset, 
		com::Vector<SceneNodeToken>* out_sceneassetnode_to_scenenode, SceneNodeFilter& p_scenenode_filer = SceneKernel::SceneIterationFilter_Default())
	{
		SceneNode_pushTo_sceneAsset(SceneKernel::resolve_node(&p_scene, p_node_included), -1, p_scene, *out_sceneasset, *out_sceneassetnode_to_scenenode, p_scenenode_filer);
	};


	inline static SceneAsset Binary_to_SceneAsset(const com::Vector<char>& p_binary_scene)
	{
		size_t l_cursor = 0;
		return SceneAsset::deserialize(l_cursor, p_binary_scene.Memory);
	};

private:

	template<class PerInputChildNodeType, class NodeAssetBuilder, class NodeAssetChildsBuilder>
	inline static void TSceneAsset_builder_fromexternal(PerInputChildNodeType& p_iterator, size_t p_parent_nodeasset_index, com::Vector<NodeAsset>& p_node_assets, 
			NodeAssetBuilder& p_nodeasset_builder, NodeAssetChildsBuilder& p_nodeasset_childs_builder)
	{
		struct ChildProcessingEntry
		{
			size_t node_asset_index;
			com::Vector<PerInputChildNodeType> childs_iterators;

			inline void free()
			{
				this->childs_iterators.free();
			};
		};

		com::Vector<ChildProcessingEntry> l_insertion_stack;
		com::Vector<ChildProcessingEntry> l_insertion_stack_buffer;

		p_node_assets.push_back(p_nodeasset_builder.build(p_iterator, p_parent_nodeasset_index));

		ChildProcessingEntry l_initial_node;
		l_initial_node.node_asset_index = p_node_assets.Size - 1;
		l_initial_node.childs_iterators = p_nodeasset_childs_builder.build(p_iterator);
		l_insertion_stack.push_back(l_initial_node);

		while (l_insertion_stack.Size > 0)
		{
			ChildProcessingEntry& l_current_processed_entry = l_insertion_stack[0];

			p_node_assets[l_current_processed_entry.node_asset_index].childs_begin = p_node_assets.Size;
			p_node_assets[l_current_processed_entry.node_asset_index].childs_end = p_node_assets.Size + l_current_processed_entry.childs_iterators.Size;

			for (size_t i = 0; i < l_current_processed_entry.childs_iterators.Size; i++)
			{
				PerInputChildNodeType& l_child_node_iterator = l_current_processed_entry.childs_iterators[i];
				p_node_assets.push_back(p_nodeasset_builder.build(l_child_node_iterator, l_current_processed_entry.node_asset_index));

				ChildProcessingEntry l_child_node;
				l_child_node.node_asset_index = p_node_assets.Size - 1;
				l_child_node.childs_iterators = p_nodeasset_childs_builder.build(l_child_node_iterator);
				l_insertion_stack_buffer.push_back(l_child_node);
			}

			l_current_processed_entry.free();
			l_insertion_stack.erase_at(0, 1);

			if (l_insertion_stack_buffer.Size > 0)
			{
				l_insertion_stack.insert_at(l_insertion_stack_buffer.to_memoryslice(), l_insertion_stack.Size);
				l_insertion_stack_buffer.clear();
			}
		}

		l_insertion_stack.free();
		l_insertion_stack_buffer.free();

	};

	inline static void JSONSceneNode_pushTo_SceneAsset(Deserialization::JSON::JSONObjectIterator& p_iterator, size_t p_parent_nodeasset_index, com::Vector<NodeAsset>& p_node_assets,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap)
	{
		struct NodeAssetBuilder
		{
			com::Vector<ComponentAsset>* component_assets;
			GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>* compoent_asset_heap;

			inline NodeAssetBuilder(com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap)
			{
				this->component_assets = &p_component_assets;
				this->compoent_asset_heap = &p_compoent_asset_heap;
			};

			inline NodeAsset build(Deserialization::JSON::JSONObjectIterator& p_child_node_iterator, size_t p_parent_nodeasset_index)
			{
				return JSONSceneNode_to_NodeAsset_withoutchilds(p_child_node_iterator, p_parent_nodeasset_index, *this->component_assets, *this->compoent_asset_heap);
			};
		};

		struct NodeAssetChildsBuilder
		{
			inline com::Vector<Deserialization::JSON::JSONObjectIterator> build(Deserialization::JSON::JSONObjectIterator& p_iterator)
			{
				return JSONSceneNode_get_childs(p_iterator);
			};
		};

		TSceneAsset_builder_fromexternal(p_iterator, p_parent_nodeasset_index, p_node_assets,
			NodeAssetBuilder(p_component_assets, p_compoent_asset_heap), NodeAssetChildsBuilder());

		p_iterator.free();
	};

	inline static NodeAsset JSONSceneNode_to_NodeAsset_withoutchilds(Deserialization::JSON::JSONObjectIterator& p_iterator, size_t p_parent_nodeasset_index,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap)
	{
		NodeAsset l_asset;
		l_asset.parent = (int)p_parent_nodeasset_index;

		Deserialization::JSON::JSONObjectIterator l_object_iterator;

		p_iterator.next_object(SceneSerializationConstants::Node::LocalPosition, &l_object_iterator);
		l_asset.local_position = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_object(SceneSerializationConstants::Node::LocalRotation, &l_object_iterator);
		l_asset.local_rotation = JSONDeserializer<Math::quat>::deserialize(l_object_iterator);

		p_iterator.next_object(SceneSerializationConstants::Node::LocalScale, &l_object_iterator);
		l_asset.local_scale = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_array(SceneSerializationConstants::Node::Components, &l_object_iterator);
		Deserialization::JSON::JSONObjectIterator l_component_iterator;
		size_t l_componentasset_count = 0;
		while (l_object_iterator.next_array_object(&l_component_iterator))
		{
			l_component_iterator.next_field(SceneSerializationConstants::ComponentType);
			StringSlice l_component_type = l_component_iterator.get_currentfield().value;

			Deserialization::JSON::JSONObjectIterator l_component_object_iterator;
			l_component_iterator.next_object(SceneSerializationConstants::ComponentObject, &l_component_object_iterator);

			ComponentAsset l_component_asset;
			if (ComponentAssetSerializer::deserializeJSON(l_component_type, l_component_object_iterator, p_component_assets, p_compoent_asset_heap, &l_component_asset))
			{
				p_component_assets.push_back(l_component_asset);
				l_componentasset_count += 1;
			}

			l_component_object_iterator.free();
		};

		l_asset.components_begin = p_component_assets.Size - l_componentasset_count;
		l_asset.components_end = p_component_assets.Size;

		return l_asset;
	};

	inline static com::Vector<Deserialization::JSON::JSONObjectIterator> JSONSceneNode_get_childs(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		com::Vector<Deserialization::JSON::JSONObjectIterator> l_childs_iterators;

		Deserialization::JSON::JSONObjectIterator l_object_iterator;
		if (p_iterator.next_array(SceneSerializationConstants::Node::Childs, &l_object_iterator))
		{
			Deserialization::JSON::JSONObjectIterator l_childs_iterator;
			while (l_object_iterator.next_array_object(&l_childs_iterator))
			{
				l_childs_iterators.push_back(l_childs_iterator.clone());
			}
		}
		l_object_iterator.free();

		return l_childs_iterators;
	};

	inline static void NodeAsset_to_JSONSceneNode(Serialization::JSON::Deserializer& p_json_deserializer, NodeAsset& p_node_asset, SceneAsset& p_scene_asset, AssetServerHandle p_asset_server)
	{
		p_json_deserializer.start_object(SceneSerializationConstants::Node::LocalPosition);
		JSONSerializer<Math::vec3f>::serialize(p_json_deserializer, p_node_asset.local_position);
		p_json_deserializer.end_object();
		p_json_deserializer.start_object(SceneSerializationConstants::Node::LocalRotation);
		JSONSerializer<Math::quat>::serialize(p_json_deserializer, p_node_asset.local_rotation);
		p_json_deserializer.end_object();
		p_json_deserializer.start_object(SceneSerializationConstants::Node::LocalScale);
		JSONSerializer<Math::vec3f>::serialize(p_json_deserializer, p_node_asset.local_scale);
		p_json_deserializer.end_object();

		p_json_deserializer.start_array(SceneSerializationConstants::Node::Components);
		for (size_t l_component_index = p_node_asset.components_begin; l_component_index < p_node_asset.components_end; l_component_index++)
		{
			p_json_deserializer.start_object();
			ComponentAssetSerializer::serializeJSON(p_scene_asset.components[l_component_index], p_json_deserializer, p_scene_asset.component_asset_heap, p_asset_server);
			p_json_deserializer.end_object();
		}
		p_json_deserializer.end_array();

		p_json_deserializer.start_array(SceneSerializationConstants::Node::Childs);
		for (size_t l_child_index = p_node_asset.childs_begin; l_child_index < p_node_asset.childs_end; l_child_index++)
		{
			p_json_deserializer.start_object();
			NodeAsset_to_JSONSceneNode(p_json_deserializer, p_scene_asset.nodes[l_child_index], p_scene_asset, p_asset_server);
			p_json_deserializer.end_object();
		}
		p_json_deserializer.end_array();
	}

	template<class SceneNodeFilter>
	inline static void SceneNode_pushTo_sceneAsset(NTreeResolve<SceneNode>& p_scene_node, size_t p_parent_sceneasset_node_index, Scene& p_scene, SceneAsset& in_out_sceneasset, SceneNodeFilter& p_scenenode_filter)
	{
		struct NodeAssetBuilder
		{
			Scene* scene;
			SceneAsset* scene_asset;

			inline NodeAssetBuilder(Scene& p_scene, SceneAsset& p_scene_asset)
			{
				this->scene = &p_scene;
				this->scene_asset = &p_scene_asset;
			};

			inline NodeAsset build(NTreeResolve<SceneNode>& p_scene_node, size_t p_parent_nodeasset_index)
			{
				return SceneNode_to_NodeAsset_withoutchilds(p_scene_node, p_parent_nodeasset_index, this->scene, this->scene_asset->components, this->scene_asset->component_asset_heap);
			};
		};

		struct NodeAssetChildsBuilder
		{
			Scene* scene;
			SceneNodeFilter* scenenode_filter;

			inline NodeAssetChildsBuilder(Scene& p_scene, SceneNodeFilter& p_scenenode_filter)
			{
				this->scene = &p_scene;
				this->scenenode_filter = &p_scenenode_filter;
			};

			inline com::Vector<NTreeResolve<SceneNode>> build(NTreeResolve<SceneNode>& p_scene_node)
			{
				return SceneNode_get_childs(p_scene_node, this->scene, *this->scenenode_filter);
			};
		};

		TSceneAsset_builder_fromexternal(p_scene_node, p_parent_sceneasset_node_index, in_out_sceneasset.nodes,
			NodeAssetBuilder(p_scene, in_out_sceneasset),
			NodeAssetChildsBuilder(p_scene, p_scenenode_filter));
	}

	template<class SceneNodeFilter>
	inline static void SceneNode_pushTo_sceneAsset(NTreeResolve<SceneNode>& p_scene_node, size_t p_parent_sceneasset_node_index, Scene& p_scene, SceneAsset& in_out_sceneasset, 
			com::Vector<SceneNodeToken>& in_out_sceneassetnode_to_scenenode, SceneNodeFilter& p_scenenode_filter)
	{
		struct NodeAssetBuilder
		{
			Scene* scene;
			SceneAsset* scene_asset;
			com::Vector<SceneNodeToken>* in_out_sceneasset_to_scenenode;

			inline NodeAssetBuilder(Scene& p_scene, SceneAsset& p_scene_asset, com::Vector<SceneNodeToken>& p_in_out_sceneasset_to_scenenode)
			{
				this->scene = &p_scene;
				this->scene_asset = &p_scene_asset;
				this->in_out_sceneasset_to_scenenode = &p_in_out_sceneasset_to_scenenode;
			};

			inline NodeAsset build(NTreeResolve<SceneNode>& p_scene_node, size_t p_parent_nodeasset_index)
			{
				this->in_out_sceneasset_to_scenenode->push_back(SceneNodeToken(p_scene_node.node->index));
				return SceneNode_to_NodeAsset_withoutchilds(p_scene_node, p_parent_nodeasset_index, this->scene, this->scene_asset->components, this->scene_asset->component_asset_heap);
			};
		};

		struct NodeAssetChildsBuilder
		{
			Scene* scene;
			SceneNodeFilter* scenenode_filter;

			inline NodeAssetChildsBuilder(Scene& p_scene, SceneNodeFilter& p_scenenode_filter)
			{
				this->scene = &p_scene;
				this->scenenode_filter = &p_scenenode_filter;
			};

			inline com::Vector<NTreeResolve<SceneNode>> build(NTreeResolve<SceneNode>& p_scene_node)
			{
				return SceneNode_get_childs(p_scene_node, this->scene, *this->scenenode_filter);
			};
		};

		TSceneAsset_builder_fromexternal(p_scene_node, p_parent_sceneasset_node_index, in_out_sceneasset.nodes,
			NodeAssetBuilder(p_scene, in_out_sceneasset, in_out_sceneassetnode_to_scenenode),
			NodeAssetChildsBuilder(p_scene, p_scenenode_filter));
	}

	inline static NodeAsset SceneNode_to_NodeAsset_withoutchilds(NTreeResolve<SceneNode>& p_node, size_t p_parent_sceneasset_node_index, Scene* p_scene,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation>& p_compoent_asset_heap)
	{
		NodeAsset l_node_asset;
		l_node_asset.parent = (int)p_parent_sceneasset_node_index;
		l_node_asset.local_position = SceneKernel::get_localposition(p_node.element);
		l_node_asset.local_rotation = SceneKernel::get_localrotation(p_node.element);
		l_node_asset.local_scale = SceneKernel::get_localscale(p_node.element);


		l_node_asset.components_begin = p_component_assets.Size;
		com::Vector<SceneNodeComponentToken>& l_components = p_scene->node_to_components[p_node.element->components].value;
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_scene_node_component_header = SceneKernel::resolve_component(p_scene, l_components[i]);
			
			ComponentAsset l_component_asset;
			if (ComponentAssetSerializer::Component_to_ComponentAsset(l_scene_node_component_header, p_compoent_asset_heap, &l_component_asset))
			{
				p_component_assets.push_back(l_component_asset);
			};

		}

		l_node_asset.components_end = p_component_assets.Size;

		return l_node_asset;
	};

	template<class SceneNodeFilter>
	inline static com::Vector<NTreeResolve<SceneNode>> SceneNode_get_childs(NTreeResolve<SceneNode>& p_node, Scene* p_scene, SceneNodeFilter& p_scenenode_filter)
	{
		com::Vector<NTreeResolve<SceneNode>> l_return;
		auto& l_node_childs = p_scene->tree.get_childs(p_node);
		for (size_t i = 0; i < l_node_childs.Size; i++)
		{
			NTreeResolve<SceneNode> l_node = p_scene->tree.resolve(l_node_childs[i]);
			if (p_scenenode_filter.evaluate(l_node))
			{
				l_return.push_back(l_node);
			}
		}
		return l_return;
	};
};

