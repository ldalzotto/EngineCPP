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
layout(location = 1) out vec2 out_screen_position;

void main()
{
	vec4 l_position = camera.projection * (camera.view * (model.model * vec4(pos, 1.0)));
	gl_Position = l_position;
	out_uv = uv;
	out_screen_position = (vec2(l_position.x/80, l_position.y/60) );
}

