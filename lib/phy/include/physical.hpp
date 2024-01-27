#pragma once

#include <cstdint>

#include <driver/uart.h>
#include <driver/gpio.h>

#include <esp_intr_alloc.h>
// physical layer
#include <esp_log.h>
#include <esp_timer.h>
#include <memory.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// idf function mashimashi
namespace physical {

// information that is stored in flash(of efuse)
namespace local {
using raw_data_t = uint32_t;

enum local_location_t {
    UpperRight,
    UpperLeft,
    LowerRight,
    LowerLeft,
};

uint32_t get_mac_address(void);
bool is_root(void);
local_location_t get_location(void);
uint32_t get_ip_address(void);

}  // namespace local

class Uart {
    static constexpr uint16_t UART_NUM = UART_NUM_1;
    static constexpr int UART_BUFFER_SIZE = (1024 * 2);
    static constexpr int UART_QUEUE_SIZE = 10;
    const gpio_num_t TX_PIN;
    const gpio_num_t RX_PIN;

    QueueHandle_t uart_queue;
    esp_timer_handle_t timer;
    uint8_t *received_data = (uint8_t *)malloc(UART_BUFFER_SIZE);
    std::size_t received_data_size = 0;

    void uart_event_task(void *args);

  public:
    Uart(gpio_num_t tx_pin, gpio_num_t rx_pin)
        : TX_PIN(tx_pin)
        , RX_PIN(rx_pin) {
    }
    void uart_init() {
        // set gpio
        gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);
        gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);
        // set uart
        uart_config_t config;
        config.baud_rate = 115200;
        config.data_bits = UART_DATA_8_BITS;
        config.parity = UART_PARITY_EVEN;
        config.stop_bits = UART_STOP_BITS_2;
        config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        config.rx_flow_ctrl_thresh = 122;
        config.source_clk = UART_SCLK_APB;
        ESP_ERROR_CHECK(uart_param_config(UART_NUM, &config));
        // don't use rts, cts
        uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

        uart_driver_install(UART_NUM, UART_BUFFER_SIZE, 0, UART_QUEUE_SIZE, &uart_queue, 0);

        // uart_signal_inv_t::UART_SIGNAL_INV_DISABLE;

        // set uart interrupt
        uart_enable_rx_intr(UART_NUM);
        reset();
        xTaskCreate([](void *args) { static_cast<Uart *>(args)->uart_event_task(args); },
                    "uart_event_task",
                    4096,
                    this,
                    5,
                    NULL);
    }

    void uart_send(std::size_t size, const char *data) {
        // tx ring buffer is 0 so, this function is blocking
        uart_write_bytes(UART_NUM, data, size);
        // uart tx and rx is directlly connected, so flush
        uart_flush(UART_NUM);
        ESP_LOGI("uart", "uart_send: %s", data);
    }

    bool uart_receive(std::size_t size, char *data) {
        // copy received_data;
        if (received_data_size > size) {
            ESP_LOGW("uart", "uart_receive: received_data_size > size");
            return false;
        }
        if (received_data_size != size) {
            ESP_LOGW("uart", "uart_receive: received_data_size != size");
            return false;
        }
        memcpy(data, received_data, size);
        flush();

        ESP_LOGD("uart", "uart_receive: %s", data);
        return true;
    }


    void reset() {
        flush();
        xQueueReset(uart_queue);
    }

    void flush() {
        uart_flush(UART_NUM);
        received_data_size = 0;
    }
};
}  // namespace physical