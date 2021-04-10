#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 frag_uv;
layout(location = 2) in vec3 frag_normal;
layout(location = 3) in vec3 frag_position;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(binding = 2) uniform light_info_uniform
{
    vec3 light_position;
    vec3 view_position;
    vec3 light_color;
} light_info;

void main()
{
    vec3 norm               = normalize(frag_normal);
    vec3 light_direction    = normalize(light_info.light_position - frag_position);
    vec3 view_direction     = normalize(light_info.view_position - frag_position);
    vec3 reflect_direction  = reflect(-light_direction, norm);

    float ambient_strength  = 0.1;
    vec3 ambient            = ambient_strength * light_info.light_color;

    float diff              = max(dot(norm, light_direction), 0.0);
    vec3 diffuse            = diff * light_info.light_color;

    float specular_strength = 0.5;
    float spec              = pow(max(dot(view_direction, reflect_direction), 0.0), 128);
    vec3 specular           = specular_strength * spec * light_info.light_color;

    vec3 object_color       = frag_color.xyz * texture(texture_sampler, frag_uv).xyz;
    vec3 result_color       = (ambient + diffuse + specular) * object_color;

    out_color = vec4(result_color, 1.0);
}
