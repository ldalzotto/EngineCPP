#pragma once

#include "components.hpp"


template<>
struct JSONDeserializer<MeshRendererAsset>
{
	static MeshRendererAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		MeshRendererAsset l_asset;
		p_iterator.next_field("mesh");
		String<> l_mesh_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.mesh = Hash<StringSlice>::hash(l_mesh_str.toSlice());
		l_mesh_str.free();

		p_iterator.next_field("material");
		String<> l_material_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.material = Hash<StringSlice>::hash(l_material_str.toSlice());
		l_material_str.free();

		return l_asset;
	}
};


struct ComponentAssetSerializer
{
	inline static bool deserialize(StringSlice& p_component_type, Serialization::JSON::JSONObjectIterator& p_component_object_iterator,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap<>& p_compoent_asset_heap, ComponentAsset* out_component_asset)
	{
		if (p_component_type.equals("MeshRenderer"))
		{
			out_component_asset->id = MeshRenderer::Id;

			allocate_component_asset<MeshRendererAsset>(p_compoent_asset_heap, &out_component_asset->componentasset_heap_index);

			MeshRendererAsset* l_mesh_renderer_asset = p_compoent_asset_heap.map<MeshRendererAsset>(out_component_asset->componentasset_heap_index);
			*l_mesh_renderer_asset = JSONDeserializer<MeshRendererAsset>::deserialize(p_component_object_iterator);

			return true;
		}

		return false;
	};



private:
	template<class ComponentAssetType>
	inline static bool allocate_component_asset(GeneralPurposeHeap<>& p_compoent_asset_heap, com::PoolToken* out_index)
	{
		while (!p_compoent_asset_heap.allocate_element(sizeof(ComponentAssetType), out_index))
		{
			p_compoent_asset_heap.realloc(p_compoent_asset_heap.memory.Capacity == 0 ? 100 : p_compoent_asset_heap.memory.Capacity * 2);
		};
		return true;
	};
};