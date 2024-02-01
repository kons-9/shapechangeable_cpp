#include <esp32c3/esp32c3.hpp>
#include <stub/stub.hpp>

#include <display.hpp>
#include <types.hpp>

#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr static int16_t DC = 4;
constexpr static int16_t SCLK = 8;
constexpr static int16_t MOSI = 10;
constexpr static int16_t MISO = -1;
constexpr static int16_t RST = 3;
constexpr static int16_t CS = -1;

static constexpr gpio_num_t TX_PIN = GPIO_NUM_21;
static constexpr gpio_num_t RX_PIN = GPIO_NUM_20;

const static char *TAG = "main";

static serial::Uart uart(TX_PIN, RX_PIN);
static display::LGFX lov_display(SCLK, MOSI, DC, CS, RST, MISO);
static fs::SpiFFS spiffs;
static uint16_t image[128 * 128];
static estimation::coordinate_t coordinate;
// static network::ip_address_t node_id;

// TODO
// receive task id
static void responce_estimation_task(void *args) {
    // if receive message, send coordinate
    // data: [true:8 node_id:16, x:16, y:16]
    // const uint8_t responce_data[8] = {
    //     0xFF,
    //     (uint8_t)(node_id >> 8),
    //     (uint8_t)(node_id & 0xFF),
    //     (uint8_t)(coordinate.first >> 8),
    //     (uint8_t)(coordinate.first & 0xFF),
    //     (uint8_t)(coordinate.second >> 8),
    //     (uint8_t)(coordinate.second & 0xFF),
    // };

    // const network::Packet responce_packet = network::Packet(network::Header::COORDINATE_ESTIMATION,
    //                                                         network::SYSTEM_PACKET_ID,
    //                                                         network::BROADCAST_ADDRESS,
    //                                                         network::BROADCAST_ADDRESS,
    //                                                         responce_data,
    //                                                         sizeof(responce_data) / sizeof(responce_data[0]));

    while (true) {
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

    TaskArgs<serial::Uart, fs::SpiFFS> args(uart, lov_display, spiffs, image, 0);

    coordinate = estimation::init_coordinate(args);
    // coordinate = estimation::init_coordinate(uart, spiffs, image, 0);

    const char *basepath = "/spiffs/test_%d_%d.raw";
    char s[100];
    sprintf(s, basepath, coordinate.first, coordinate.second);
    ESP_LOGI(TAG, "file path: %s", s);

    auto len = spiffs.read_image(s, image, sizeof(image) / sizeof(image[0]));
    ESP_LOGI(TAG, "read image len: %d", len);

    lov_display.init();
    lov_display.pushImage(0, 0, 128, 128, image);

    xTaskCreate(responce_estimation_task, "main_task", 4096, NULL, 1, NULL);
}