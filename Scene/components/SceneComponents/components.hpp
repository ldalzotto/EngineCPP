#pragma once

#include <string>
#include "Scene/component_def.hpp"
#include <Render/assets.hpp>

struct MeshRenderer
{
	inline static const size_t Id = 0;
	static const SceneNodeComponent_TypeInfo Type;

	size_t material;
	size_t model;

	inline void initialize(const size_t& p_material, const size_t& p_model)
	{
		this->material = p_material;
		this->model = p_model;
	}


	inline void initialize(const StringSlice& p_material, const StringSlice& p_model)
	{
		this->initialize(Hash<StringSlice>::hash(p_material), Hash<StringSlice>::hash(p_model));
	}
};

inline const SceneNodeComponent_TypeInfo MeshRenderer::Type = SceneNodeComponent_TypeInfo(MeshRenderer::Id, sizeof(MeshRenderer));


struct MeshRendererAsset
{
	size_t mesh;
	size_t material;
};


