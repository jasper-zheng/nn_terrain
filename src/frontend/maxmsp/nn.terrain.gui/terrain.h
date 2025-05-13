
#include "../shared/min_path.h"

#pragma once


using namespace c74::min::ui;
using namespace c74::min;

class terrain {
public:
    terrain(object_base* an_owner, const double width, const double height, const function& a_function = nullptr)
        : m_width{ width }
        , m_height{ height }
        , m_draw_callback{ a_function } {
    }

    ~terrain() {
        if (m_surface) {
            c74::max::jgraphics_surface_destroy(m_surface);
            m_surface = nullptr;
        }
    }


    void redraw(const int width, const int height) {
        auto old_surface = m_surface;
        m_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, width, height);
        m_width = width;
        m_height = height;
        c74::max::t_jgraphics* ctx = jgraphics_create(m_surface);

        atoms a{ { ctx, m_width, m_height} };

        target t{ a };

        m_draw_callback(a, 0);
        c74::max::jgraphics_image_surface_draw(ctx, m_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, m_width, m_height });

        c74::max::jgraphics_destroy(ctx);

        if (old_surface) {
            c74::max::jgraphics_surface_destroy(old_surface);
        }
    }
    
    void save_png(symbol file_name, min_path m_log_path, long dpi) {
        c74::max::jgraphics_image_surface_writepng(m_surface, file_name, m_log_path.get_path(), dpi);
    }

    void draw_surface(ui::target& t, const double width, const double height) {
        c74::max::jgraphics_image_surface_draw(t, m_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, width, height });
    }

private:
    double					m_width;
    double					m_height;
    function        	    m_draw_callback;
    c74::max::t_jsurface*   m_surface{ nullptr };
};
