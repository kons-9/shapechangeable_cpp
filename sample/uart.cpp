#include <physical.hpp>
#include <flit.hpp>
#include <types.hpp>

static constexpr gpio_num_t TX_PIN = GPIO_NUM_21;
static constexpr gpio_num_t RX_PIN = GPIO_NUM_20;

static physical::Uart uart(TX_PIN, RX_PIN);

void sample_uart_rx_task(void *args) {
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        network::raw_data_t raw_data;
        auto is_success = uart.uart_receive(raw_data.size(), (char *)raw_data.data());
        if (!is_success) {
            ESP_LOGW("test", "uart receive error");
            continue;
        }
        auto deceded_data = network::decoder(raw_data);
        if (!deceded_data.has_value()) {
            ESP_LOGW("test", "flit decode error");
            continue;
        }
        auto flit = std::move(deceded_data.value());
        if (flit->get_type() != network::FlitType::Head) {
            ESP_LOGW("test", "flit type error");
            continue;
        }
        auto head_flit = network::static_pointer_cast<network::HeadFlit>(std::move(flit));
        // ESP_LOGI("test",
        //          "head_flit: %d, %d, %d, %d, %d",
        //          (int)head_flit->get_type(),
        //          (int)head_flit->get_header(),
        //          (int)head_flit->get_packet_id(),
        //          (int)head_flit->get_src(),
        //          (int)head_flit->get_dst());
        if (head_flit->get_type() == network::FlitType::Head
            && head_flit->get_header() == network::Header::COORDINATE_ESTIMATION
            && head_flit->get_packet_id() == network::SYSTEM_PACKET_ID
            && head_flit->get_dst() == network::BROADCAST_ADDRESS
            && head_flit->get_src() == physical::local::get_ip_address()) {
            ESP_LOGI("test", "receive estimation flit");
        }
    }
}

void sample_uart_tx_task(void *args) {
    while (true) {
        network::HeadFlit head_flit(0,
                                    network::Header::COORDINATE_ESTIMATION,
                                    network::SYSTEM_PACKET_ID,
                                    physical::local::get_ip_address(),
                                    network::BROADCAST_ADDRESS);
        network::raw_data_t raw_data;
        head_flit.to_rawdata(raw_data);
        uart.uart_send(raw_data.size(), (char *)raw_data.data());
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}