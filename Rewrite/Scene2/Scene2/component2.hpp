#pragma once

namespace v2
{
	struct SceneNodeComponentType
	{
		size_t id;
		size_t size;

		inline static SceneNodeComponentType build(const size_t p_id, const size_t p_size)
		{
			return SceneNodeComponentType{ p_id,p_size };
		};
		inline static SceneNodeComponentType build_default()
		{
			return build(-1, 0);
		};
	};
}