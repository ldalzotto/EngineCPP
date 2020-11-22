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

struct CameraAsset
{
	float fov = 0.0f;
	float near_ = 0.0f;
	float far_ = 0.0f;
};


struct Camera
{
	inline static const size_t Id = (MeshRenderer::Id + 1);
	static const SceneNodeComponent_TypeInfo Type;

	float fov = 0.0f;
	float near_ = 0.0f;
	float far_ = 0.0f;

	inline Camera(const CameraAsset& p_asset)
	{
		this->fov = p_asset.fov;
		this->near_ = p_asset.near_;
		this->far_ = p_asset.far_;
	};
};

inline const SceneNodeComponent_TypeInfo Camera::Type = SceneNodeComponent_TypeInfo(Camera::Id, sizeof(Camera));

