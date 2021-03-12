#include "platform/vulkan/vulkan_vertex.hpp"
#include "graphics/vertex.hpp"

// VkVertexInputBindingDescription lna::vulkan_vertex_binding_description()
// {
//     VkVertexInputBindingDescription binding_description_result{};
//     binding_description_result.binding     = 0;
//     binding_description_result.stride      = sizeof(lna::vertex);
//     binding_description_result.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX;
//     return binding_description_result;
// }

// lna::stack_array<VkVertexInputAttributeDescription, 3> lna::vulkan_vertex_attribute_descriptions()
// {
//     lna::stack_array<VkVertexInputAttributeDescription, 3> attribute_descriptions_result;
//     attribute_descriptions_result.elements[0].binding   = 0;
//     attribute_descriptions_result.elements[0].location  = 0;
//     attribute_descriptions_result.elements[0].format    = VK_FORMAT_R32G32_SFLOAT;
//     attribute_descriptions_result.elements[0].offset    = offsetof(lna::vertex, position);
//     attribute_descriptions_result.elements[1].binding   = 0;
//     attribute_descriptions_result.elements[1].location  = 1;
//     attribute_descriptions_result.elements[1].format    = VK_FORMAT_R32G32B32A32_SFLOAT;
//     attribute_descriptions_result.elements[1].offset    = offsetof(lna::vertex, color);
//     attribute_descriptions_result.elements[2].binding   = 0;
//     attribute_descriptions_result.elements[2].location  = 2;
//     attribute_descriptions_result.elements[2].format    = VK_FORMAT_R32G32_SFLOAT;
//     attribute_descriptions_result.elements[2].offset    = offsetof(lna::vertex, uv);
//     return attribute_descriptions_result;
// }

lna::vulkan_vertex_description lna::vulkan_default_vertex_description()
{
    lna::vulkan_vertex_description result;
    result.bindings[0].binding      = 0;
    result.bindings[0].stride       = sizeof(lna::vertex);
    result.bindings[0].inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;
    result.attributes[0].binding    = 0;
    result.attributes[0].location   = 0;
    result.attributes[0].format     = VK_FORMAT_R32G32_SFLOAT;
    result.attributes[0].offset     = offsetof(lna::vertex, position);
    result.attributes[1].binding    = 0;
    result.attributes[1].location   = 1;
    result.attributes[1].format     = VK_FORMAT_R32G32B32A32_SFLOAT;
    result.attributes[1].offset     = offsetof(lna::vertex, color);
    result.attributes[2].binding    = 0;
    result.attributes[2].location   = 2;
    result.attributes[2].format     = VK_FORMAT_R32G32_SFLOAT;
    result.attributes[2].offset     = offsetof(lna::vertex, uv);
    return result;
}
