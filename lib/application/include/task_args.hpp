#pragma once

#include <concepts.hpp>
#include <LovyanGFX.hpp>

using task_id_t = uint8_t;

// TODO: add lock
template <traits::serial T, traits::file_system F>
class TaskArgs {
  public:
    T &uart;
    lgfx::LGFX_Device &lov_display;
    F &spiffs;
    uint16_t *image_buffer;
    task_id_t task_id;

    TaskArgs(T &uart, lgfx::LGFX_Device &lov_display, F &spiffs, uint16_t *image_buffer, uint8_t task_id)
        : uart(uart)
        , lov_display(lov_display)
        , spiffs(spiffs)
        , image_buffer(image_buffer)
        , task_id(task_id) {
    }
};