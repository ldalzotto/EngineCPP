#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 0) uniform Color
{
	vec4 _val;
} color;

void main()
{
	outColor = texture(texSampler, uv) * color._val;
}