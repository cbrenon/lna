#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniform_buffer_object
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 frag_uv;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_position;

void main()
{
    frag_normal     = mat3(transpose(inverse(ubo.model))) * in_normal;
    frag_position   = vec3(ubo.model * vec4(in_position, 1.0));
    frag_color      = in_color;
    frag_uv         = in_uv;

    gl_Position     = ubo.projection * ubo.view * vec4(frag_position, 1.0);
}
