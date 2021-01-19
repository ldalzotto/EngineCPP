#pragma once

namespace v2
{
	struct SceneNodeComponentType
	{
		uimax id;
		uimax size;

		inline static constexpr SceneNodeComponentType build(const uimax p_id, const uimax p_size)
		{
			return SceneNodeComponentType{ p_id,p_size };
		};
		inline static SceneNodeComponentType build_default()
		{
			return build(-1, 0);
		};
	};
}