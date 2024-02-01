#pragma once
#include <concepts.hpp>
#include <types.hpp>

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
namespace serial {

// information that is stored in flash(of efuse)
using raw_serial_data_t = uint32_t;

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

    EventGroupHandle_t uart_event_group = xEventGroupCreate();
    constexpr static uint32_t UART_DATA_BIT = BIT0;

    void uart_event_task(void *args);
    void uart_send(::std::size_t size, const char *data);
    bool uart_receive(::std::size_t size, char *data);

  public:
    Uart(gpio_num_t tx_pin, gpio_num_t rx_pin)
        : TX_PIN(tx_pin)
        , RX_PIN(rx_pin) {
    }
    void uart_init();
    void reset();
    void flush();

    // for serial trait
    traits::SerialError send(const network::raw_data_t &data);
    traits::SerialError receive(network::raw_data_t &data);
};
static_assert(traits::serial<Uart>);


}  // namespace serial