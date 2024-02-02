#include "esp32c3/ota.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace ota {

static bool check_version(void) {
    // do something
    return false;
}

static void update(void) {
    // do something
}

static void ota_task(void *args) {
    for (;;) {
        // check version
        if (check_version()) {
            // update
            update();
            // reoboot
            esp_restart();
        }
        // sleep for 1 minutes
        vTaskDelay(1 * 60 * 1000 / portTICK_PERIOD_MS);
    }
}
void ota_init(void) {
    // do something
    xTaskCreate(ota_task, "ota_task", 4096, NULL, 5, NULL);
}
}  // namespace ota