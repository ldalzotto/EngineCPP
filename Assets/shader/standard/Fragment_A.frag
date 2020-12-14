#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 screen_position;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 0) uniform Color
{
	vec4 _val;
} color;


mat4 thresholdMatrix = mat4
(
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
);

void main()
{
	vec4 final_color = texture(texSampler, uv) * color._val;
	vec2 l_screen_position = vec2(gl_FragCoord.x, gl_FragCoord.y);

	float l_alpha_test = final_color.w - thresholdMatrix[int(mod(l_screen_position.x, 4))][int(mod(l_screen_position.y, 4))];
	if(l_alpha_test <= 0.0f){ discard; }

	outColor = final_color;
}