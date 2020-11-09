#pragma once

#include <string>
#include "Scene/component_def.hpp"

struct MeshRenderer
{
	inline static const size_t Id = 0;
	static const SceneNodeComponent_TypeInfo Type;

	std::string vertex_shader;
	std::string fragment_shader;
	std::string model;
};

inline const SceneNodeComponent_TypeInfo MeshRenderer::Type = SceneNodeComponent_TypeInfo(MeshRenderer::Id, sizeof(MeshRenderer));