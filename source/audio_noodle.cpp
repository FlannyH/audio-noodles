#include "graph.hpp"
#include <ui/panel.hpp>
#include <ui/components.hpp>
#include <ui/panel_manager.hpp>
#include <graphics/renderer.hpp>

int main() {
    Gfx::init(Gfx::RenderAPI::OpenGL, 1280, 720, "Audio Noodles");
    Graph::create_track();
    Graph::create_output();

    while (Gfx::should_stay_open()) {
        Gfx::set_cursor_mode(Gfx::CursorMode::Arrow);
        Input::update();

        Gfx::begin_frame();
        Gfx::set_view_offset_2d({0, 0});
        Gfx::set_view_scale_2d({1, 1});
        UI::panel_input();
        UI::panel_render();
        Graph::run();
        Gfx::end_frame();
    };
}
