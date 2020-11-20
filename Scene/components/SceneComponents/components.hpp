#pragma once

#include <string>
#include "Scene/component_def.hpp"
#include <Render/assets.hpp>

struct MeshRenderer
{
	inline static const size_t Id = 0;
	static const SceneNodeComponent_TypeInfo Type;

	MaterialAsset material;
	std::string model;

	inline void initialize(const MaterialAsset& p_material, const std::string& p_model)
	{
		this->material = p_material;
		this->model = p_model;
	}
};

inline const SceneNodeComponent_TypeInfo MeshRenderer::Type = SceneNodeComponent_TypeInfo(MeshRenderer::Id, sizeof(MeshRenderer));