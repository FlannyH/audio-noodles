#include "graph.hpp"
#include <ui/panel.hpp>
#include <ui/components.hpp>
#include <ui/panel_manager.hpp>
#include <graphics/renderer.hpp>

int main() {
    Gfx::init(Gfx::RenderAPI::OpenGL, 1280, 720, "Audio Noodles");
    Graph::create_track();
    Graph::create_output();

    glm::vec2 offset(0.0);
    float scale = 1.0;

    while (Gfx::should_stay_open()) {
        Gfx::set_cursor_mode(Gfx::CursorMode::Arrow);
        Input::update();

        Gfx::begin_frame();
        Gfx::set_view_offset_2d(offset);
        Gfx::set_view_scale_2d({scale, scale});
        if (Input::key_pressed(Input::Key::Up)) scale *= powf(2.0, 0.25);
        if (Input::key_pressed(Input::Key::Down)) scale /= powf(2.0, 0.25);
        if (Input::key_held(Input::Key::W)) offset.y += Gfx::get_delta_time();
        if (Input::key_held(Input::Key::S)) offset.y -= Gfx::get_delta_time();
        if (Input::key_held(Input::Key::A)) offset.x += Gfx::get_delta_time();
        if (Input::key_held(Input::Key::D)) offset.x -= Gfx::get_delta_time();
        UI::panel_input();
        UI::panel_render();
        Graph::run();
        Gfx::end_frame();
    };
}
