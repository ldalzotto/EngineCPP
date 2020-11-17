#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main()
{
	outColor = texture(texSampler, uv);
	// outColor = vec4(uv.x, uv.y, 0.0, 0.0);
}