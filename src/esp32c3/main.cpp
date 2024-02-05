#include <esp32c3/log.hpp>
#include <esp32c3/esp32c3.hpp>
// #include <stub/stub.hpp>
#include <esp32c3/display.hpp>
#include <types.hpp>
#include <estimation.hpp>

#include "esp32c3/sample_task.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr static int16_t DC = 4;
constexpr static int16_t SCLK = 8;
constexpr static int16_t MOSI = 10;
constexpr static int16_t MISO = -1;
constexpr static int16_t RST = 3;
constexpr static int16_t CS = 5;

static constexpr gpio_num_t TX_PIN = GPIO_NUM_21;
static constexpr gpio_num_t RX_PIN = GPIO_NUM_20;

const static char *TAG = "main";

static serial::Link uart(TX_PIN, RX_PIN);
static display::LGFX lov_display(SCLK, MOSI, DC, CS, RST, MISO);
static fs::SpiFFS spiffs;
static uint16_t image[128 * 128 + 1];

static TaskArgs<serial::Link, fs::SpiFFS> arg(uart, lov_display, spiffs, image, 1);

static void log_init(void) {
    // wait until uart is ready
    // TODO
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void printf_task(void *args) {
    for (;;) {
        ESP_LOGI(TAG, "Hello, world!");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

#define USER1

extern "C" void app_main() {
    lov_display.init();
    log_init();

    ESP_LOGI(TAG, "app main start!");
    // todo
    ota::ota_init();
    uart.uart_init();

    // xTaskCreate(sample_uart_rx_task, "sample_uart_rx_task", 8096, &arg, 1, NULL);
    // xTaskCreate(sample_uart_tx_task, "sample_uart_tx_task", 8096, &arg, 5, NULL);
    xTaskCreate(hello_ferris_task, "hello_ferris_task", 8096, &arg, 1, NULL);
    // xTaskCreate(sample_display_task, "sample_display_task", 8096, &arg, 1, NULL);

    // xTaskCreate(responce_estimation_task, "main_task", 8096, &arg, 1, NULL);
    // xTaskCreate(printf_task, "printf_task", 2048, NULL, 1, NULL);
    //
#ifdef USER1
    xTaskCreate(sample_estimation_task, "sample_estimation_task", 8096, &arg, 1, NULL);
#else
    xTaskCreate(sample_other_confirmed_estimaiton_task, "sample_other_confirmed_task", 8096, &arg, 1, NULL);
#endif
}