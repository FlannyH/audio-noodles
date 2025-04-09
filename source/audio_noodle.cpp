#include <cstdio>
#include "midi.hpp"
#include "mixer.hpp"
#include "session.hpp"
#include "ui/scene.hpp"
#include "ui/components.hpp"
#include "graphics/renderer.hpp"

int main() {
    Midi::init();
    Mixer::init();
    Gfx::init(Gfx::RenderAPI::OpenGL, 1280, 720, "Audio Noodles");
    Session::tracks().push_back(Track{});

    UI::Scene scene;
    UI::create_button(scene, { { -100, -150 }, { 100, -250 }, 0.0f, Gfx::AnchorPoint::Center }, []()
        {
            printf("hi1!\n");
        }, { L"Button1", {2, 2},  { 1, 0, 0, 1 }, Gfx::AnchorPoint::Center, Gfx::AnchorPoint::Center});
    UI::create_text(scene, "debug_text", { { 8, 8 }, {480, 480} }, { L"" });
    UI::create_numberbox(scene, "debug_numberbox", { { 100, 600 }, { 200, 700 } }, { 0.0, 100.0, 1.0, 0, 0 });
    UI::create_wheelknob(scene, "debug_numberbox", { { 300, 600 }, { 200, 700 } }, { 0.0, 100.0, 1.0, 0, 0 });
    UI::create_slider(scene, "debug_numberbox", { { 500, 300 }, { 600, 700 } }, { 0.0, 100.0, 1.0, 50, 0 });
    UI::create_slider(scene, "debug_numberbox", { { 600, 300 }, { 1000, 360 } }, { 0.0, 100.0, 1.0, 50, 0 }, false);
    UI::create_radio_button(scene, "debug_radio_button", { { 700, 400 }, { 1100, 640 } }, std::vector<std::wstring>({
        L"hello",
        L"there",
        L"i am",
        L"a test",
        L"ui element :)",
        }), 0);
    UI::create_combobox(scene, "debug_combobox", { {800, 20}, {1200, 80}, 0.01f }, std::vector<std::wstring>({
        L"Option 1",
        L"Option 2",
        L"Option 3",
        L"Option 4",
        L"Option 5",
        L"Option 6",
        L"Option 7",
        L"Option 8",
        L"Option 9",
        L"Option A",
        L"Option B",
        L"Option C",
        L"Option D",
        L"Option E",
        L"Option F",
        })
    );

    while (Gfx::should_stay_open()) {
        Gfx::begin_frame();
        UI::update_entities(scene, Gfx::get_delta_time());
        Gfx::end_frame();

        // todo: move this to separate thread
        Midi::process();
    };
}
