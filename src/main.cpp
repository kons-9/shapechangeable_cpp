#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_log.h>

#include "flit.hpp"

extern "C" void app_main() {
    flit::raw_data_t raw_data = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
    auto expect = flit::decoder(raw_data);
    auto flit = std::move(expect.value());
    ESP_LOGI("test", "flit type: %d", (uint8_t)flit->get_type());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}