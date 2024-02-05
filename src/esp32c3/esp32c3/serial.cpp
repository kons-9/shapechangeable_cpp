#include "esp32c3/serial.hpp"
#include "esp_random.h"

#define BLOCKING

namespace serial {

// information that is stored in flash(of efuse)
using raw_serial_data_t = uint32_t;

void Link::uart_init() {
    // set timer
    gptimer_config_t timer_config = {.clk_src = GPTIMER_CLK_SRC_DEFAULT,
                                     .direction = GPTIMER_COUNT_UP,
                                     .resolution_hz = 1 * 1000 * 1000,  // 1MHz, 1 tick = 1us
                                     .flags = {
                                         .intr_shared = true,
                                     }};
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer_handle));
    ESP_ERROR_CHECK(gptimer_enable(timer_handle));
    ESP_ERROR_CHECK(gptimer_start(timer_handle));

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
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, UART_BUFFER_SIZE, 0, UART_QUEUE_SIZE, &uart_queue, 0));

    // uart_signal_inv_t::UART_SIGNAL_INV_DISABLE;

    // set uart interrupt
    uart_enable_rx_intr(UART_NUM);
    reset();
    xTaskCreate([](void *args) { static_cast<Link *>(args)->uart_event_task(args); },
                "uart_event_task",
                4096,
                this,
                5,
                NULL);
}

void Link::uart_event_task(void *args) {
    ESP_LOGI("uart", "uart_event_task start");
    uart_event_t event;
    uint64_t last_receive_time = 0;
    gptimer_get_raw_count(timer_handle, &last_receive_time);
    while (true) {
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                // normal event
#ifdef BLOCKING
                if (sending) {
                    // ignore
                    // flushを最初にやると、連続で来たときに
                    // flush -> sending = true -> uart event -> sending = false
                    // となる可能性があるので、flushを後にやる
                    // このことで自分の送信データが消えることが保証されるが、
                    // タイミングによっては相手のデータが消える可能性がある
                    // ackを使うなどして対処する
                    ESP_LOGI("uart", "sending ignore");
                    sending = false;
                    flush();
                    break;
                }
#endif
                received_data_size = uart_read_bytes(UART_NUM, received_data, event.size, 100);
                xEventGroupSetBits(uart_event_group, UART_DATA_BIT);
                ESP_LOGD("uart", "uart_event_task: received_data_size: %d", received_data_size);
                ESP_LOGD("uart", "uart_event_task: received_data: %s", received_data);
                wait_delta = 1;
                break;
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                // overflow, reset queue
                ESP_LOGW("uart", "overflow");
                flush();
                break;
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                // collision detect
                ESP_LOGW("uart", "collision detect");
                update_wait_delta();
                flush();
                break;
            case UART_BREAK:
            case UART_PATTERN_DET:
            default:
                // unhandeled event
                ESP_LOGW("uart", "uart event type: %d", event.type);
                update_wait_delta();
                reset();
                break;
            }
        }
        gptimer_get_raw_count(timer_handle, &last_receive_time);
    }
}

void Link::uart_send(::std::size_t size, const char *data) {
    uint64_t now;
    gptimer_get_raw_count(timer_handle, &now);
    // change
    while (now - last_receive_time < (SEND_INTERVAL_MS + wait_delta) * 1000) {
        ESP_LOGV("uart", "wait now: %lld, last_send_time: %lld", now, last_send_time);
        // wait 10ms
        vTaskDelay(SEND_INTERVAL_MS / 10 / portTICK_PERIOD_MS);
        gptimer_get_raw_count(timer_handle, &now);
    }
    sending = true;
    // tx ring buffer is 0 so, this function is blocking
    auto len = uart_write_bytes(UART_NUM, data, size);
    if (len != size) {
        ESP_LOGW("uart", "uart_send: len(%d) != size(%d)", len, size);
        return;
    }
    // uart tx and rx is directlly connected, so flush
    ESP_LOGI("uart", "uart_send: %s", data);
}

bool Link::uart_receive(::std::size_t size, char *data) {
    // TODO: more information in return value
    if (xEventGroupWaitBits(uart_event_group, UART_DATA_BIT, pdTRUE, pdFALSE, TIMEOUT_MS / portTICK_PERIOD_MS)
        != UART_DATA_BIT) {
        ESP_LOGD("uart", "uart_receive: timeout");
        return false;
    }
    // copy received_data;
    if (received_data_size > size) {
        ESP_LOGW("uart", "uart_receive: received_data_size(%d) > size(%d)", received_data_size, size);
        update_wait_delta();
        return false;
    }
    if (received_data_size != size) {
        ESP_LOGW("uart", "uart_receive: received_data_size(%d) != size(%d)", received_data_size, size);
        update_wait_delta();
        return false;
    }
    memcpy(data, received_data, size);
    // flush();

    ESP_LOGD("uart", "uart_receive: %s", data);
    return true;
}


void Link::reset() {
    flush();
    xQueueReset(uart_queue);
    gptimer_set_raw_count(timer_handle, 0);
}

void Link::flush() {
    uart_flush(UART_NUM);
    received_data_size = 0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void Link::set_ip_address(network::ip_address_t ip_address) {
    this->ip_address = ip_address;
}

// for serial trait
traits::SerialError Link::send(const network::raw_data_t &data) {

    uart_send(data.size(), (char *)data.data());
    return traits::SerialError::Ok;
};

traits::SerialError Link::send(const network::Flit &flit) {
    network::raw_data_t raw_data;
    std::visit([&raw_data](auto &&arg) { arg.to_rawdata(raw_data); }, flit);
    return send(raw_data);
}

traits::SerialError Link::receive(network::raw_data_t &data) {
    if (uart_receive(data.size(), (char *)data.data()) != true) {
        return traits::SerialError::GenericError;
    }
    return traits::SerialError::Ok;
};

traits::SerialError Link::receive(network::Flit &flit, uint8_t channel) {
    network::raw_data_t raw_data;

    uint64_t now;
    gptimer_get_raw_count(timer_handle, &now);
    uint64_t start = now;
    while (now - start < TIMEOUT_MS * 1000) {
        gptimer_get_raw_count(timer_handle, &now);
        auto err = receive(raw_data);
        if (err != traits::SerialError::Ok) {
            LOGW("uart", "receive error %d", err);
            continue;
        }
        auto value = network::decoder(raw_data);
        if (!value.has_value()) {
            LOGW("uart", "flit decode error");
            continue;
        }
        flit = value.value();
        if (std::holds_alternative<network::HeadFlit>(flit)) {
            // check address
            auto headflit = std::get<network::HeadFlit>(flit);
            if (headflit.get_src() == this->ip_address) {
                LOGW("uart", "flit src error");
                return traits::SerialError::GenericError;
            }
            if (headflit.get_dst() != network::BROADCAST_ADDRESS && headflit.get_dst() != ip_address) {
                LOGW("uart", "flit dst error");
                return traits::SerialError::GenericError;
            }
        }
        return traits::SerialError::Ok;
    }
    LOGW("uart", "timeout");
    return traits::SerialError::GenericError;
}

static void send_ack() {
    // todo
}

};  // namespace serial