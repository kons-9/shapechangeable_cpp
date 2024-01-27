#include "display.hpp"
#include "fs.hpp"

display::LGFX lov_display;
fs::SpiFFS spiffs;
uint16_t image[128 * 128 + 1];

void sample_display_task(void *args) {
    lov_display.init();
    lov_display.fillScreen(TFT_BLACK);
    // lov_display.setRotation(3);
    lov_display.setTextSize(1);
    lov_display.setCursor(0, 0);
    lov_display.setFont(&fonts::Font0);
    // lov_display.setTextColor(TFT_RED);
    ESP_LOGI("test", "dislay hello world");
    lov_display.printf("Hello World.\nI'm lovyanGFX.\nIt is a super useful library.\n");
    uint32_t macaddress = 0;
    macaddress = spiffs.read_macaddress().value_or(0);
    lov_display.printf("macaddress: %08lx\n", macaddress);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // image
    //
    auto len = spiffs.read_image("/spiffs/ferris.raw", image, 128 * 128);
    if (len <= 0) {
        ESP_LOGE("test", "Failed to read image");
        return;
    }
    ESP_LOGI("test", "readed image: %d", len);
    lov_display.pushImage(0, 0, 86, len / 86, image);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    len = spiffs.read_image("/spiffs/test_0_0.raw", image, 128 * 128);
    if (len <= 0) {
        ESP_LOGE("test", "Failed to read image");
        return;
    }
    ESP_LOGI("test", "readed image: %d", len);
    lov_display.pushImage(0, 0, 128, 128, image);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}