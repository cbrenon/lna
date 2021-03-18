#ifndef _LNA_PLATFORM_VULKAN_VULKAN_VERTEX_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_VERTEX_HPP_

#include <vulkan.h>

namespace lna
{
    struct vulkan_vertex_description
    {
        enum
        {
            MAX_BINDING     = 1,
            MAX_ATTRIBUTES  = 3,
        };

        VkVertexInputBindingDescription     bindings[MAX_BINDING];
        VkVertexInputAttributeDescription   attributes[MAX_ATTRIBUTES];
    };

    vulkan_vertex_description vulkan_default_vertex_description();
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_VERTEX_HPP_
