#include <cstdio>
#include "mixer.hpp"
#include "midi.hpp"

int main() {
    Midi::init();
    printf("hello world\n");
    while(1){
        Midi::process();
    };
}
