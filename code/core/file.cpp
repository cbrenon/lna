#include <fstream>
#include "core/file.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    void file_debug_load(
        file& f,
        const char* filename,
        bool binary,
        memory_pool& mem_pool
        )
    {
        LNA_ASSERT(f.content == nullptr);
        LNA_ASSERT(f.content_size == 0);

        std::ifstream fd(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);
        LNA_ASSERT(fd.is_open());

        f.content_size  = static_cast<uint32_t>(fd.tellg());
        f.content       = LNA_ALLOC(
            mem_pool,
            char,
            f.content_size
            );
        LNA_ASSERT(f.content);

        fd.seekg(0);
        fd.read(
            f.content,
            f.content_size
            );
        fd.close();
    }
}