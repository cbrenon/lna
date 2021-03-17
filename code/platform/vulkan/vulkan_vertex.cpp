#include "platform/vulkan/vulkan_vertex.hpp"
#include "graphics/vertex.hpp"

lna::vulkan_vertex_description lna::vulkan_default_vertex_description()
{
    lna::vulkan_vertex_description result;
    result.bindings[0].binding      = 0;
    result.bindings[0].stride       = sizeof(lna::vertex);
    result.bindings[0].inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;
    result.attributes[0].binding    = 0;
    result.attributes[0].location   = 0;
    result.attributes[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
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
