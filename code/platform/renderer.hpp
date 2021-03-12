#ifndef _LNA_PLATFORM_RENDERER_HPP_
#define _LNA_PLATFORM_RENDERER_HPP_

namespace lna
{
    struct memory_pool_system;

    template<typename window_api>
    struct renderer_config
    {
        const char* application_name;
        uint8_t     application_major_ver;
        uint8_t     application_minor_ver;
        uint8_t     application_patch_ver;
        const char* engine_name;
        uint8_t     engine_major_ver;
        uint8_t     engine_minor_ver;
        uint8_t     engine_patch_ver;
        bool        enable_validation_layers;
        window_api* window_ptr;
    };

    template<typename renderer_api>
    void renderer_init(
        renderer_api& renderer
        );

    template<typename renderer_api, typename window_api>
    void renderer_configure(
        renderer_api& renderer,
        const renderer_config<window_api>& config
        );

    template<typename renderer_api>
    void renderer_draw_frame(
        renderer_api& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        );

    template<typename renderer_api>
    void renderer_release(
        renderer_api& renderer
        );
}

#endif // _LNA_GRAPHICS_RENDERER_HPP_
