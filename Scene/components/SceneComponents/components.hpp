#pragma once

#include <string>
#include "Common/Asset/asset_key.hpp"
#include "Scene/component_def.hpp"
#include <Render/assets.hpp>

struct MeshRendererAsset
{
	struct MeshKey : public AssetKey { using AssetKey::AssetKey; } mesh;
	struct MaterialKey : public AssetKey { using AssetKey::AssetKey; } material;

	inline MeshRendererAsset() {};
	inline MeshRendererAsset(const MeshKey& p_mesh_key, const MaterialKey& p_material_key)
	{
		this->mesh = p_mesh_key;
		this->material = p_material_key;
	};
};

struct MeshRenderer
{
	inline static const size_t Id = Hash<ConstString>::hash("MeshRenderer");
	static const SceneNodeComponent_TypeInfo Type;
	inline static constexpr char const* TypeName = "MeshRenderer";

	
	MeshRendererAsset meshrenderer_asset;
	
	com::PoolToken rendererable_object;

	inline void initialize(const MeshRendererAsset& p_asset)
	{
		this->meshrenderer_asset = p_asset;
	}

	inline void initialize(const StringSlice& p_material, const StringSlice& p_model)
	{
		this->initialize(MeshRendererAsset(MeshRendererAsset::MeshKey(p_model), MeshRendererAsset::MaterialKey(p_material)));
	};

	inline void initialize_default()
	{
		this->initialize(MeshRendererAsset(MeshRendererAsset::MeshKey("models/cube.obj"), MeshRendererAsset::MaterialKey("materials/editor_gizmo.json")));
	};
};

inline const SceneNodeComponent_TypeInfo MeshRenderer::Type = SceneNodeComponent_TypeInfo(MeshRenderer::Id, sizeof(MeshRenderer));


struct CameraAsset
{
	float fov = 0.0f;
	float near_ = 0.0f;
	float far_ = 0.0f;
};


struct Camera
{
	inline static const size_t Id = Hash<ConstString>::hash("Camera");
	static const SceneNodeComponent_TypeInfo Type;
	inline static constexpr const char* TypeName = "Camera";

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

struct SceneComponentUtils
{
	inline static bool get_type_from_name(StringSlice& p_component_type_name, SceneNodeComponent_TypeInfo const** out_type)
	{
		size_t l_component_type_name_hash = Hash<StringSlice>::hash(p_component_type_name);
		switch (l_component_type_name_hash)
		{
		case Hash<ConstString>::hash(MeshRenderer::TypeName):
		{
			*out_type = &MeshRenderer::Type;
			return true;
		}
		break;
		case Hash<ConstString>::hash(Camera::TypeName):
		{
			*out_type = &Camera::Type;
			return true;
		}
		break;
		}

		return false;
	};

	inline static bool get_name_from_id(const size_t p_id, const char** out_name)
	{
		switch (p_id)
		{
		case MeshRenderer::Id:
		{
			*out_name = MeshRenderer::TypeName;
			return true;
		}
		break;
		case Camera::Id:
		{
			*out_name = Camera::TypeName;
			return true;
		}
		break;
		}

		return false;
	};
};
