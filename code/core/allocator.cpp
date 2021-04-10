#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "core/allocator.hpp"
#include "core/assert.hpp"

namespace lna
{
    based_allocator::~based_allocator()
    {
        if (_content)
        {
            VirtualFree(
                _content,
                0,
                MEM_RELEASE
                );
            _content                    = nullptr;
            _content_max_size_in_bytes  = 0;
            _cur_content_offset         = 0;
        }
    }

    void based_allocator::init(size_t max_size_in_bytes)
    {
        LNA_ASSERT(_content == nullptr);
        LNA_ASSERT(_content_max_size_in_bytes == 0);
        LNA_ASSERT(_cur_content_offset == 0);

        _content_max_size_in_bytes  = max_size_in_bytes;
        _content                    = (std::byte*)VirtualAlloc(
            0,
            max_size_in_bytes,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE
            );
        LNA_ASSERT(_content);
    }

    std::byte* based_allocator::alloc(size_t size_in_bytes)
    {
        LNA_ASSERT(_content);
        LNA_ASSERT(size_in_bytes > 0);
        LNA_ASSERT(_cur_content_offset + size_in_bytes <= _content_max_size_in_bytes);

        size_t offset = _cur_content_offset;
        _cur_content_offset += size_in_bytes;
        return (_content + offset);
    }
}