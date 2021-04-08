#ifndef _LNA_CORE_ALLOCATOR_HPP_
#define _LNA_CORE_ALLOCATOR_HPP_

namespace lna
{
    struct default_allocator
    {
        size_t  current_content_offset;
        size_t  content_max_size_in_bytes;
        void*   content;
    };

    void default_allocator_init(
        default_allocator&  allocator,
        size_t              max_size_in_bytes
        );

    void* default_allocator_alloc(
        default_allocator&  allocator,
        size_t              size_in_bytes
        );

    void default_allocator_release(
        default_allocator&  allocator
        );
}

#endif // _LNA_CORE_ALLOCATOR_HPP_
