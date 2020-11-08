#pragma once

struct SceneNodeComponent_TypeInfo
{
	size_t id;
	size_t size;

	inline SceneNodeComponent_TypeInfo(const size_t p_id, const size_t p_size) : id{ p_id }, size{ p_size }{};
};