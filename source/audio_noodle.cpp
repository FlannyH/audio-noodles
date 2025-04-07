#include <cstdio>
#include "midi.hpp"
#include "mixer.hpp"
#include "session.hpp"
#include "graphics/renderer.hpp"

int main() {
    Midi::init();
    Mixer::init();
    Gfx::init(Gfx::RenderAPI::OpenGL, 1280, 720, "Audio Noodles");
    Session::tracks().push_back(Track{});
    while (Gfx::should_stay_open()) {
        Gfx::begin_frame();
        Gfx::end_frame();

        // todo: move this to separate thread
        Midi::process();
    };
}
