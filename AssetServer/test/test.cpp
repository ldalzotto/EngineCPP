
#include  "AssetServer/asset_server.cpp"

struct RProv
{
	inline static std::string ResourceName = "shader";
};

void main(int argc, char** argv)
{
	AssetServer l_server;
	l_server.allocate(std::string(argv[0]));

	GenericAssetQuery l_shaderasset_query;
	l_shaderasset_query.allocate<RProv>(l_server.asset_path, l_server.connection);

	com::Vector<char> l_source;
	File::read_bytes(l_server.asset_path.asset_folder_path + "shader/TriFrag.spv", l_source);
	l_shaderasset_query.insert_or_update("shader/TriFrag.spv");
	// l_shaderasset_query.insert_or_update("shader/TriFrag.spv");
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
}