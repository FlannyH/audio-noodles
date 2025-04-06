#include <cstdio>
#include "midi.hpp"
#include "mixer.hpp"
#include "session.hpp"

int main() {
    Midi::init();
    Mixer::init();
    Session::tracks().push_back(Track{});
    while (1) {
        Midi::process();
    };
}
