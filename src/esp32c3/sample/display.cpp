#include <esp32c3/esp32c3.hpp>
#include "esp32c3/display.hpp"
#include <concepts.hpp>

constexpr static int16_t DC = 4;
constexpr static int16_t SCLK = 8;
constexpr static int16_t MOSI = 10;
constexpr static int16_t MISO = -1;
constexpr static int16_t RST = 3;
constexpr static int16_t CS = 5;

static const char *TAG = "display_sample";

void hello_ferris_task(void *args) {
    LOGI(TAG, "hello_ferris_task");
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &lov_display = task_args->lov_display;
    auto image = task_args->image_buffer;
    auto &spiffs = task_args->spiffs;


    lov_display.printf("Hello World.\nI'm lovyanGFX.\nIt is a super useful library.\n");
    std::size_t len = 0;
    len = spiffs.read_image("/spiffs/ferris.raw", image, 128 * 128);
    lov_display.printf("debug: hello_ferris_task\n");

    if (len <= 0) {
        ESP_LOGE(TAG, "Failed to read image");
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "readed image: %d", len);
    lov_display.printf("readed image: %d\n", len);
    auto disp_width = lov_display.width();
    auto disp_height = lov_display.height();
    auto image_width = 86;
    auto image_height = len / 86;
    lov_display.printf("printing image...\n");

    vTaskDelay(10 / portTICK_PERIOD_MS);

    lov_display.fillScreen(TFT_BLACK);
    lov_display.pushImage((disp_width - image_width) >> 1,
                          (disp_height - image_height) >> 1,
                          image_width,
                          image_height,
                          image);
    ESP_LOGI(TAG, "pushed image");

    // delete
    vTaskDelete(NULL);
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void sample_display_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &spiffs = task_args->spiffs;
    auto &lov_display = task_args->lov_display;
    auto image = task_args->image_buffer;

    lov_display.init();
    lov_display.fillScreen(TFT_BLACK);
    // lov_display.setRotation(3);
    lov_display.setTextSize(1);
    lov_display.setCursor(0, 0);
    lov_display.setFont(&fonts::Font0);
    // lov_display.setTextColor(TFT_RED);
    ESP_LOGI(TAG, "dislay hello world");
    lov_display.printf("Hello World.\nI'm lovyanGFX.\nIt is a super useful library.\n");
    uint32_t macaddress = 0;
    macaddress = spiffs.read_macaddress().value_or(0);
    lov_display.printf("macaddress: %08lx\n", macaddress);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // image
    //
    auto len = spiffs.read_image("/spiffs/ferris.raw", image, 128 * 128);
    if (len <= 0) {
        ESP_LOGE(TAG, "Failed to read image");
        return;
    }
    ESP_LOGI(TAG, "readed image: %d", len);
    lov_display.pushImage(0, 0, 86, len / 86, image);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    len = spiffs.read_image("/spiffs/test_0_0.raw", image, 128 * 128);
    if (len <= 0) {
        ESP_LOGE(TAG, "Failed to read image");
        return;
    }
    ESP_LOGI(TAG, "readed image: %d", len);
    lov_display.pushImage(0, 0, 128, 128, image);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}