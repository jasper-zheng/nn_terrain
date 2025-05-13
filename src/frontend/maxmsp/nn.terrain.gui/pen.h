
#include <filesystem>
#include <chrono>
#include <ctime>

#include "../shared/min_path.h"

#pragma once

using namespace c74::min;
using namespace c74::min::ui;

class pen {
public:
    pen(object_base* an_owner, const function& a_function = nullptr, const function& traj_function = nullptr, const function& play_function = nullptr)
        : m_width{ 600.0 }
        , m_height{ 150.0 }
        , m_ink_callback{ a_function }
        , m_traj_callback{ traj_function }
        , m_play_callback{ play_function } {
    }

    ~pen() {
        if (m_surface) {
            c74::max::jgraphics_surface_destroy(m_surface);
            m_surface = nullptr;
        }
        if (m_history_surface) {
            c74::max::jgraphics_surface_destroy(m_history_surface);
            m_history_surface = nullptr;
        }
    }

    void clear_history() {
        m_history_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, m_width, m_height);
        draw_inks(m_width, m_height);
    }
    
    void draw_trajs(const int width, const int height) {
        auto old_surface = m_traj_surface;
        m_traj_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, width, height);
        m_width = width;
        m_height = height;
        c74::max::t_jgraphics* ctx = jgraphics_create(m_traj_surface);
        atoms a{ { ctx, m_width, m_height} };
        target t{ a };
        
        auto length = m_traj_callback(a, 0)[0];
        
        c74::max::jgraphics_destroy(ctx);
        if (old_surface) {
            c74::max::jgraphics_surface_destroy(old_surface);
        }
    }
    
    void draw_plays(const int width, const int height) {
        auto old_surface = m_traj_surface;
        m_traj_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, width, height);
        m_width = width;
        m_height = height;
        c74::max::t_jgraphics* ctx = jgraphics_create(m_traj_surface);
        atoms a{ { ctx, m_width, m_height} };
        target t{ a };
        
        auto length = m_play_callback(a, 0)[0];
        
        c74::max::jgraphics_destroy(ctx);
        if (old_surface) {
            c74::max::jgraphics_surface_destroy(old_surface);
        }
    }
    
    void draw_trajs_surface(ui::target& t) {
        c74::max::jgraphics_image_surface_draw(t, m_traj_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, m_width, m_height });
    }

    void draw_inks(const int width, const int height) {
        auto old_surface = m_surface;
        m_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, width, height);
        m_width = width;
        m_height = height;
        c74::max::t_jgraphics* ctx = jgraphics_create(m_surface);

        atoms a{ { ctx, m_width, m_height} };
        target t{ a };

        if (m_history_surface) {
            color c{ 1.0, 1.0, 1.0, 1.0 };
            c74::max::jgraphics_set_source_jrgba(ctx, c);
            c74::max::jgraphics_image_surface_draw(ctx, m_history_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, m_width, m_height });
        }

        m_history_ink = m_ink_callback(a, 0)[0];

        c74::max::jgraphics_destroy(ctx);

        if (old_surface) {
            c74::max::jgraphics_surface_destroy(old_surface);
		}
    }
    
    void draw_history(ui::target& t, const double width, const double height) {
        if (m_history_surface) {
            c74::max::jgraphics_image_surface_draw(t, m_history_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, m_width, m_height });
        }
    }
        

    void draw_surface(ui::target& t, const double width, const double height) {
        c74::max::jgraphics_image_surface_draw(t, m_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, width, height });
    }

    void lock_canvas() {
        auto old_surface = m_history_surface;
        m_history_surface = c74::max::jgraphics_image_surface_create(c74::max::JGRAPHICS_FORMAT_ARGB32, m_width, m_height);
        c74::max::t_jgraphics* ctx = jgraphics_create(m_history_surface);
        
        color c{ 1.0, 1.0, 1.0, 1.0 };
        c74::max::jgraphics_set_source_jrgba(ctx, c);
        c74::max::jgraphics_image_surface_draw(ctx, m_surface, { 0.0, 0.0, m_width, m_height }, { 0.0, 0.0, m_width, m_height });
        c74::max::jgraphics_destroy(ctx);
        if (old_surface) {
            c74::max::jgraphics_surface_destroy(old_surface);
        }
	}

    void save_png(symbol file_name, min_path m_log_path, long dpi) {
        c74::max::jgraphics_image_surface_writepng(m_surface, file_name, m_log_path.get_path()/*c74::max::path_desktopfolder()*/, dpi);
    }
    

private:
    double					m_width;
    double					m_height;
    function        	    m_ink_callback;
    function                m_traj_callback;
    function                m_play_callback;
        
    c74::max::t_jsurface*   m_traj_surface{ nullptr };
    
    c74::max::t_jsurface*   m_surface{ nullptr };
    c74::max::t_jsurface*   m_history_surface{ nullptr };
    
    double  			    m_history_ink{ 0.0 };
};
