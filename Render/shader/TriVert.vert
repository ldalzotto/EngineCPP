#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(set = 0, binding = 0) uniform Camera
{
	mat4 view;
	mat4 projection;
} camera;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = vec4(pos, 1.0);
	vec4 l_color = camera.view * vec4(color, 1.0);
	fragColor = vec3(l_color);
}

