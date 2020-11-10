
#include  "AssetServer/asset_server.cpp"

void main(int argc, char** argv)
{
	AssetServer l_server;
	l_server.allocate(std::string(argv[0]));

	GenericAssetQuery l_shaderasset_query;
	l_shaderasset_query.allocate(l_server.connection, "shader");


	l_shaderasset_query.insert_or_update(l_server.asset_path, l_server.connection, "shader/TriVert.spv");
	l_shaderasset_query.insert_or_update(l_server.asset_path, l_server.connection, "shader/TriFrag.spv");
	com::Vector<char> l_bytes;
	l_shaderasset_query.request(l_server.connection, "shader/TriVert.spv", l_bytes);
	l_bytes.free();

	l_shaderasset_query.free(l_server.connection);

	l_server.free();
}