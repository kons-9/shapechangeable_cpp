#include <esp32c3/esp32c3.hpp>
#include <task_args.hpp>
#include <flit.hpp>
#include <types.hpp>

void sample_uart_rx_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &spiffs = task_args->spiffs;
    network::ip_address_t address = spiffs.read_macaddress().value_or(0);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        network::raw_data_t raw_data;
        auto is_success = uart.receive(raw_data);
        if (is_success != traits::SerialError::Ok) {
            ESP_LOGW("test", "uart receive error");
            continue;
        }
        auto deceded_data = network::decoder(raw_data);
        if (!deceded_data.has_value()) {
            ESP_LOGW("test", "flit decode error");
            continue;
        }
        auto value = deceded_data.value();
        if (!std::holds_alternative<network::HeadFlit>(value)) {
            ESP_LOGW("test", "flit type error");
            continue;
        }
        auto flit = std::get<network::HeadFlit>(std::move(deceded_data.value()));
        // ESP_LOGI("test",
        //          "head_flit: %d, %d, %d, %d, %d",
        //          (int)head_flit->get_type(),
        //          (int)head_flit->get_header(),
        //          (int)head_flit->get_packet_id(),
        //          (int)head_flit->get_src(),
        //          (int)head_flit->get_dst());
        if (flit.get_type() == network::FlitType::Head && flit.get_header() == network::Header::COORDINATE_ESTIMATION
            && flit.get_packet_id() == network::SYSTEM_PACKET_ID && flit.get_dst() == network::BROADCAST_ADDRESS
            && flit.get_src() == address) {
            ESP_LOGI("test", "receive estimation flit");
            continue;
        }
        ESP_LOGI("test",
                 "receive flit: %d, %d, %d, %d, %d",
                 (int)flit.get_type(),
                 (int)flit.get_header(),
                 (int)flit.get_packet_id(),
                 (int)flit.get_src(),
                 (int)flit.get_dst());
    }
}

void sample_uart_tx_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &spiffs = task_args->spiffs;

    network::ip_address_t address = spiffs.read_macaddress().value_or(0);
    while (true) {
        network::HeadFlit head_flit(0,
                                    network::Header::COORDINATE_ESTIMATION,
                                    network::SYSTEM_PACKET_ID,
                                    address,
                                    network::BROADCAST_ADDRESS);
        network::raw_data_t raw_data;
        head_flit.to_rawdata(raw_data);
        uart.send(raw_data);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void sample_packet_uart_rx_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &spiffs = task_args->spiffs;
    network::ip_address_t address = spiffs.read_macaddress().value_or(0);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        network::Packet packet;
        // auto err = packet.receive(uart, address);
        auto err = uart.receive(packet, address);
        if (err != network::NetworkError::OK) {
            ESP_LOGE("test", "send error: %d", (int)err);
        }

        if (packet.get_header() == network::Header::COORDINATE_ESTIMATION
            && packet.get_packet_id() == network::SYSTEM_PACKET_ID && packet.get_dst() == network::BROADCAST_ADDRESS
            && packet.get_src() != address) {
            ESP_LOGI("test", "receive estimation flit");
            continue;
        }
        ESP_LOGW("test",
                 "unknown: receive flit: header: %d, packetid: %d, src: %d, dst: %d, address: %d",
                 (network::header_t)packet.get_header(),
                 packet.get_packet_id(),
                 packet.get_src(),
                 packet.get_dst(),
                 address);
    }
}

void sample_packet_uart_tx_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &spiffs = task_args->spiffs;

    network::ip_address_t address;
    address = spiffs.read_macaddress().value_or(0);
    auto routing = network::DefaultRouting();
    while (true) {
        network::Packet packet = network::Packet(network::Header::COORDINATE_ESTIMATION, address);
        uart.send(packet, address);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}