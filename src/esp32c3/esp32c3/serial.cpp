#include "esp32c3/serial.hpp"

namespace serial {

// information that is stored in flash(of efuse)
using raw_serial_data_t = uint32_t;

void Uart::uart_init() {
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

void Uart::uart_event_task(void *args) {
    uart_event_t event;
    while (true) {
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                // normal event
                received_data_size = uart_read_bytes(UART_NUM, received_data, event.size, 100);
                xEventGroupSetBits(uart_event_group, UART_DATA_BIT);
                break;
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                // overflow, reset queue
                ESP_LOGI("uart", "overflow");
                reset();
                break;
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                // collision detect
                ESP_LOGI("uart", "collision detect");
                reset();
                break;
            case UART_BREAK:
            case UART_PATTERN_DET:
            default:
                // unhandeled event
                ESP_LOGI("uart", "uart event type: %d", event.type);
                break;
            }
        }
    }
}

void Uart::uart_send(::std::size_t size, const char *data) {
    // tx ring buffer is 0 so, this function is blocking
    uart_write_bytes(UART_NUM, data, size);
    // uart tx and rx is directlly connected, so flush
    uart_flush(UART_NUM);
    ESP_LOGI("uart", "uart_send: %s", data);
}

bool Uart::uart_receive(::std::size_t size, char *data) {
    // TODO: more information in return value
    if (xEventGroupWaitBits(uart_event_group, UART_DATA_BIT, pdFALSE, pdFALSE, 1000 / portTICK_PERIOD_MS)
        != UART_DATA_BIT) {
        ESP_LOGW("uart", "uart_receive: timeout");
        return false;
    }
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


void Uart::reset() {
    flush();
    xQueueReset(uart_queue);
}

void Uart::flush() {
    uart_flush(UART_NUM);
    received_data_size = 0;
}

// for serial trait
traits::SerialError Uart::send(const network::raw_data_t &data) {
    uart_send(data.size(), (char *)data.data());
    return traits::SerialError::Ok;
};
traits::SerialError Uart::receive(network::raw_data_t &data) {
    if (uart_receive(data.size(), (char *)data.data()) != true) {
        return traits::SerialError::GenericError;
    }
    return traits::SerialError::Ok;
};
};  // namespace serial