#ifndef _LNA_BACKENDS_IMGUI_BACKEND_HPP_
#define _LNA_BACKENDS_IMGUI_BACKEND_HPP_

namespace lna
{
    struct imgui_backend;
    struct renderer_backend;
    struct texture_backend;

    struct imgui_backend_config
    {
        renderer_backend*   renderer_backend_ptr;
        texture_backend*    texture_backend_ptr;
        float               window_width;
        float               window_height;
    };

    void imgui_backend_configure(
        imgui_backend& backend,
        imgui_backend_config& config
        );

    void imgui_backend_update(
        imgui_backend& backend
        );

    void imgui_backend_draw_frame(
        imgui_backend& backend
        );

    void imgui_backend_release(
        imgui_backend& backend
        );
}

#endif // _LNA_BACKENDS_IMGUI_BACKEND_HPP_
