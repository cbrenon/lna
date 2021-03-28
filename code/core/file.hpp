#ifndef _LNA_CORE_FILE_HPP_
#define _LNA_CORE_FILE_HPP_

namespace lna
{
    struct file
    {
        uint32_t    content_size;
        char*       content;
    };

    struct memory_pool;

    //! used during development. Dot not use for retail (prefer future file system)
    void file_debug_load(
        file& f,
        const char* filename,
        bool binary,
        memory_pool& mem_pool
        );
}

#endif // _LNA_CORE_FILE_HPP_
