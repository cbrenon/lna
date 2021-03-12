#include <fstream>
#include "core/file.hpp"
#include "core/assert.hpp"

namespace lna
{
    heap_array<char> file_debug_load(
        const char* filename,
        bool binary,
        memory_pool& pool
        )
    {
        std::ifstream file(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);
        LNA_ASSERT(file.is_open());
        uint32_t file_size = static_cast<uint32_t>(file.tellg());
        heap_array<char> buffer;
        heap_array_init(
            buffer
            );
        heap_array_set_max_element_count(
            buffer,
            pool,
            file_size
            );
        file.seekg(0);
        file.read(
            buffer.elements,
            file_size
            );
        file.close();
        return buffer;
    }
}