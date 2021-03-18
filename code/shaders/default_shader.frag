#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;
layout(binding = 1) uniform sampler2D texture_sampler;

void main()
{
    out_color = vec4(frag_color * texture(texture_sampler, frag_uv));
}
