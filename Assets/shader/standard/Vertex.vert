#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform Camera
{
	mat4 view;
	mat4 projection;
} camera;

layout(set = 1, binding = 0) uniform Model
{
	mat4 model;
} model;

layout(location = 0) out vec2 out_uv;

void main()
{
	gl_Position = camera.projection * (camera.view * (model.model * vec4(pos, 1.0)));
	out_uv = uv;
}

