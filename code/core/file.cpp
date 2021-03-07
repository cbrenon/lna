#include <fstream>
#include "core/file.hpp"
#include "core/assert.hpp"

namespace lna
{
    std::vector<char> file_debug_load(
        const char* filename,
        bool binary
        )
    {
        std::ifstream file(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);
        LNA_ASSERT(file.is_open());
        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);
        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();
        return buffer;
    }
}