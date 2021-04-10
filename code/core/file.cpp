#include <fstream>
#include "core/file.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    void file::debug_load(
        const char* filename,
        bool binary,
        memory_pool& mem_pool
        )
    {
        LNA_ASSERT(_content.ptr() == nullptr);
        LNA_ASSERT(_content.size() == 0);

        std::ifstream fd(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);
        LNA_ASSERT(fd.is_open());

        _content.init(static_cast<uint32_t>(fd.tellg()), mem_pool);

        fd.seekg(0);
        fd.read(
            _content.ptr(),
            _content.size()
            );
        fd.close();
    }
}