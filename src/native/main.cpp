

#include <stdio.h>
#include <stub/stub.hpp>

static LGFX display(1080, 1920, 2);

int main_func(bool *running) {
    display.init();
    while (*running) {
        display.printf("Hello, world!\n");
        sleep(10);
    }
}

int main(void) {

    printf("Hello, world!\n");
    return lgfx::Panel_sdl::main(main_func);
}