#ifndef _LNA_CORE_FILE_HPP_
#define _LNA_CORE_FILE_HPP_

namespace lna
{
    class memory_pool;

    class file
    {
        public:

            file() = default;
            ~file() = default;

            //! used during development. Dot not use for retail (prefer future file system)
            void debug_load(const char* filename, bool binary, memory_pool& mem_pool);

            inline auto size() const { return _content_size; }
            inline auto content() const { return _content; }
        
        private:

            uint32_t    _content_size   { 0 };
            char*       _content        { nullptr };
    };
}

#endif // _LNA_CORE_FILE_HPP_
