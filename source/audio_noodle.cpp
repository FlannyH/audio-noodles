#include <cstdio>
#include "mixer.hpp"
#include "midi.hpp"
#include "session.hpp"

int main() {
    Midi::init();
    Session::tracks().push_back(Track{});
    while(1){
        Midi::process();
    };
}
