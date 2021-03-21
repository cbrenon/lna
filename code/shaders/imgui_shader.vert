#version 450

layout (push_constant) uniform push_constants {
	vec2 scale;
	vec2 translate;
} in_push_constants;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;
layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 frag_uv;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

void main() 
{
	frag_uv = in_uv;
	frag_color = in_color;
	gl_Position = vec4(in_position * in_push_constants.scale + in_push_constants.translate, 0.0, 1.0);
}
