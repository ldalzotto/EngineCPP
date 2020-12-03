#pragma once

#include <stdint.h>
#include <string>
#include <AssetServer/asset_server.hpp>
#include "Render/assets.hpp"
#include "Common/Memory/handle.hpp"
#include "Common/Container/vector_def.hpp"
#include "Common/Container/gptr.hpp"
#include "Math/matrix_def.hpp"
#include "Math/vector_def.hpp"
#include "Render/rdwindow.hpp"

typedef void* RenderHandle;

struct VertexResource
{
	Math::vec3f position;
	Math::vec3f color;
};

struct MeshHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const std::string& p_mesh);
	void allocate(const RenderHandle& p_render, const size_t p_mesh);
	void free(const RenderHandle& p_render);
};

struct TextureHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const std::string& p_texture);
	void allocate(const RenderHandle& p_render, const size_t p_texture);
	void free(const RenderHandle& p_render);
};

struct ShaderHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const std::string& p_sahder_path);
	void allocate(const RenderHandle& p_render, const size_t p_shader_path);
	void free(const RenderHandle& p_render);
};

struct MaterialType
{
	unsigned short int type = -1;

	inline MaterialType() {};
	inline MaterialType(const unsigned short int p_type) { this->type = p_type; };
};

struct MaterialHandle : public Handle
{
	ShaderHandle shader;

	void allocate(const RenderHandle& p_render, const MaterialType p_type, const size_t p_material);
	MaterialType get_type(const RenderHandle& p_render);

	void set_uniform_parameter(const RenderHandle& p_render, const size_t p_parameter_index, const GPtr& p_value);
	void get_uniform_paramter(const RenderHandle& p_render, size_t p_parameter_index, GPtr& out_value);

	void free(const RenderHandle& p_render);
};



struct RenderableObjectHandle : public Handle
{
	MeshHandle mesh;
	MaterialHandle material;

	void allocate(const RenderHandle& p_render, const MaterialHandle p_material, const MeshHandle p_meshhandle);
	void push_trs(const RenderHandle& p_render, const Math::mat4f& p_trs);
	void free(const RenderHandle& p_render);
	void set_material(const RenderHandle& p_render, const MaterialHandle& p_material);
};



RenderHandle create_render(const AssetServerHandle p_assetserver_handle);
void destroy_render(const RenderHandle& p_render);
WindowHandle render_window(const RenderHandle& p_render);
bool render_window_should_close(const RenderHandle& p_render);
void render_window_pool_event(const RenderHandle& p_render);
void render_draw(const RenderHandle& p_render);
void render_push_camera_buffer(const RenderHandle& p_render, const float p_fov, const float p_near, const float p_far, const Math::vec3f& p_world_position, const Math::vec3f& p_world_forward, const Math::vec3f& p_world_up);


enum class MaterialTypeEnum
{
	UNKNOWN = 0,
	DEFAULT = 1
};

template<MaterialTypeEnum TMaterialType>
struct TMaterial{};

template<>
struct TMaterial<MaterialTypeEnum::DEFAULT>
{
	inline static const MaterialTypeEnum Type = MaterialTypeEnum::DEFAULT;
	inline static const size_t COLOR_INDEX = 1;

	MaterialHandle material;

	inline void set_color(const RenderHandle& p_render, Math::vec4f& p_value)
	{
		this->material.set_uniform_parameter(p_render, COLOR_INDEX, GPtr::fromType(&p_value));
	};

	inline Math::vec4f get_color(const RenderHandle& p_render)
	{
		Math::vec4f l_return;
		this->material.get_uniform_paramter(p_render, COLOR_INDEX, GPtr::fromType(&l_return));
		return l_return;
	};
};

template<MaterialTypeEnum TMaterialType>
struct TMaterialCaster
{
	inline static bool cast(MaterialHandle& p_material, RenderHandle& p_render, TMaterial<TMaterialType>* out_material)
	{
		if (p_material.get_type(p_render).type != (unsigned short int)TMaterialType)
		{
			out_material->material = MaterialHandle();
			return false;
		};

		out_material->material = p_material;
		return true;
	};
};