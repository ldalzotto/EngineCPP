
#include  "AssetServer/asset_server.cpp"

#include "Common/Serialization/json.hpp"

void main(int argc, char** argv)
{

	size_t One = Hash<StringSlice>::hash(StringSlice("One"));
	size_t Zero = Hash<StringSlice>::hash(StringSlice("Zero"));
	size_t SrcColor = Hash<StringSlice>::hash(StringSlice("SrcColor"));
	size_t SrcAlpha = Hash<StringSlice>::hash(StringSlice("SrcAlpha"));
	size_t DstColor = Hash<StringSlice>::hash(StringSlice("DstColor"));
	size_t DstAlpha = Hash<StringSlice>::hash(StringSlice("DstAlpha"));
	size_t OneMinusSrcColor = Hash<StringSlice>::hash(StringSlice("OneMinusSrcColor"));
	size_t OneMinusSrcAlpha = Hash<StringSlice>::hash(StringSlice("OneMinusSrcAlpha"));
	size_t OneMinusDstColor = Hash<StringSlice>::hash(StringSlice("OneMinusDstColor"));
	size_t OneMinusDstAlpha = Hash<StringSlice>::hash(StringSlice("OneMinusDstAlpha"));


	size_t Add = Hash<StringSlice>::hash(StringSlice("Add"));
	size_t Substract = Hash<StringSlice>::hash(StringSlice("Substract"));
	size_t ReverseSubstract = Hash<StringSlice>::hash(StringSlice("ReverseSubstract"));
	size_t Min = Hash<StringSlice>::hash(StringSlice("Min"));
	size_t Max = Hash<StringSlice>::hash(StringSlice("Max"));


#if 0
	AssetServer l_server;
	l_server.allocate(std::string(argv[0]));

	GenericAssetQuery l_shaderasset_query;
	l_shaderasset_query.allocate(l_server.asset_path, l_server.connection);


	com::Vector<char> l_source;
	FileAlgorithm::read_bytes(l_server.asset_path.asset_folder_path + "shader/TriFrag.spv", l_source);
	l_shaderasset_query.insert_or_update("shader/TriFrag.spv");
	l_shaderasset_query.insert_or_update("shader/TriFrag.spv");
	com::Vector<char> l_bytes;
	l_shaderasset_query.request("shader/TriFrag.spv", l_bytes);


	for (size_t i = 0; i < l_source.Size; i++)
	{
		if (l_source[i] != l_bytes[i])
		{
			abort();
		}
	}

	l_bytes.free();

	l_shaderasset_query.free();

	l_server.free();
#endif
	char* l_json_test = "{\"type\":\"scene\",\"nodes\":[{\"parent\":\"-1\",\"local_position\":{\"x\":\"0.1452f\",\"y\":\"0.1452f\",\"z\":\"0.1452f\",},\"local_rotation\":{},\"local_scale\":{},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]},{\"parent\":\"-1\",\"local_position\":{\"x\":\"-0.1452f\",\"y\":\"-0.1452f\",\"z\":\"-0.1452f\",},\"local_rotation\":{},\"local_scale\":{},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]}]}";

	String<> l_source;
	l_source.from_raw(l_json_test);
	Serialization::JSON::JSONObjectIterator l_root = Serialization::JSON::StartDeserialization(l_source);

	l_root.next_field("type");
	Serialization::JSON::FieldNode& l_field_node = l_root.get_currentfield();
	Serialization::JSON::JSONObjectIterator l_nodes; l_root.next_array("nodes", &l_nodes);
	{
		Serialization::JSON::JSONObjectIterator l_node;
		while (l_nodes.next_array_object(&l_node))
		{
			l_node.next_field("parent");
			auto& l_field = l_node.get_currentfield();
			l_node.next_field("local_position");
			l_field = l_node.get_currentfield();
		}
	}
	l_root.free();
	
}