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
	std::string texture;

	inline void initialize(const std::string& p_vertex_shader, const std::string& p_fragment_shader, const std::string& p_model, const std::string& p_texture)
	{
		this->vertex_shader = p_vertex_shader;
		this->fragment_shader = p_fragment_shader;
		this->model = p_model;
		this->texture = p_texture;
	}
};

inline const SceneNodeComponent_TypeInfo MeshRenderer::Type = SceneNodeComponent_TypeInfo(MeshRenderer::Id, sizeof(MeshRenderer));