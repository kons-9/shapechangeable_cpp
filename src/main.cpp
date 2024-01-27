#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <ota.hpp>
#include <estimation.hpp>
#include <display.hpp>

#include <esp_log.h>

constexpr static int16_t DC = 4;
constexpr static int16_t SCLK = 8;
constexpr static int16_t MOSI = 10;
constexpr static int16_t MISO = -1;
constexpr static int16_t RST = 3;
constexpr static int16_t CS = -1;

static constexpr gpio_num_t TX_PIN = GPIO_NUM_21;
static constexpr gpio_num_t RX_PIN = GPIO_NUM_20;

const static char *TAG = "main";

static physical::Uart uart(TX_PIN, RX_PIN);
static display::LGFX lov_display(SCLK, MOSI, DC, CS, RST, MISO);

static void estimation_task(void *args) {
    auto coordinate = estimation::init_coordinate(uart);
    while (true) {
        coordinate = estimation::update_coordinate(coordinate);
        ESP_LOGI("test", "coordinate: %d, %d", coordinate.first, coordinate.second);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void log_init(void) {
    esp_log_level_set("uart", ESP_LOG_INFO);

    // wait until uart is ready
    // TODO
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}


extern "C" void app_main() {
    log_init();

    ESP_LOGI(TAG, "app main start!");
    // todo
    ota::ota_init();

    uart.uart_init();

    xTaskCreate(estimation_task, "main_task", 4096, NULL, 1, NULL);
}