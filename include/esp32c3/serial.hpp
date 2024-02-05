#pragma once
#include <concepts.hpp>
#include <types.hpp>
#include <flit.hpp>
#include <packet.hpp>
#include <header.hpp>

#include <cstdint>

#include <driver/uart.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>

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

class Link {
    static constexpr uint16_t UART_NUM = UART_NUM_1;
    static constexpr int UART_BUFFER_SIZE = (1024 * 2);
    static constexpr int UART_QUEUE_SIZE = 10;
    static constexpr int TIMEOUT_MS = 1000;
    static constexpr int SEND_INTERVAL_MS = 50;
    static constexpr int MAX_WAIT_DELTA = 256;
    // power of 2
    static_assert((MAX_WAIT_DELTA & (MAX_WAIT_DELTA - 1)) == 0);
    const gpio_num_t TX_PIN;
    const gpio_num_t RX_PIN;

    QueueHandle_t uart_queue;
    gptimer_handle_t timer_handle = NULL;

    uint64_t last_send_time = 0;
    uint64_t last_receive_time = 0;
    uint32_t wait_delta = 1;
    // TODO: channel
    uint8_t *received_data = (uint8_t *)malloc(UART_BUFFER_SIZE);
    std::size_t received_data_size = 0;
    bool sending = false;
    network::ip_address_t ip_address;

    EventGroupHandle_t uart_event_group = xEventGroupCreate();
    constexpr static uint32_t UART_DATA_BIT = BIT0;

    void uart_event_task(void *args);
    void uart_send(::std::size_t size, const char *data);
    bool uart_receive(::std::size_t size, char *data);

  public:
    Link(gpio_num_t tx_pin, gpio_num_t rx_pin, network::ip_address_t ip_address = 0)
        : TX_PIN(tx_pin)
        , RX_PIN(rx_pin)
        , ip_address(ip_address) {
    }

    ~Link() {
        ESP_LOGE("Link", "Link destructor never called");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    void uart_init();
    void reset();
    void flush();

    void update_wait_delta(void) {
        auto tmp = wait_delta << 1;
        wait_delta = wait_delta & (MAX_WAIT_DELTA - 1);
    }

    // for serial trait
    void set_ip_address(network::ip_address_t ip_address);
    traits::SerialError send(const network::raw_data_t &data);
    traits::SerialError send(const network::Flit &flit);
    traits::SerialError send(network::Packet &packet, network::ip_address_t this_ip, uint8_t channel = 0);

    traits::SerialError receive(network::raw_data_t &data);
    traits::SerialError receive(network::Flit &flit, uint8_t channel = 0);
    traits::SerialError receive(network::Packet &flit, uint8_t channel = 0);
};
static_assert(traits::serial<Link>);


}  // namespace serial