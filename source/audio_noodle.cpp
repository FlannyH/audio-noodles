#include <cstdio>
#include "midi.hpp"
#include "mixer.hpp"
#include "session.hpp"
#include "ui/scene.hpp"
#include "ui/panel.hpp"
#include "ui/components.hpp"
#include "ui/panel_manager.hpp"
#include "graphics/renderer.hpp"

int main() {
    Midi::init();
    Mixer::init();
    Gfx::init(Gfx::RenderAPI::OpenGL, 1280, 720, "Audio Noodles");
    Session::tracks().push_back(Track{});

    while (Gfx::should_stay_open()) {
        Gfx::set_cursor_mode(Gfx::CursorMode::Arrow);
        Input::update();

        Gfx::begin_frame();
        UI::panel_input();
        UI::panel_render();
        Gfx::end_frame();

        // todo: move this to separate thread
        Midi::process();
    };
}
