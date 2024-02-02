#pragma once
#include "estimation.hpp"
// #include "estimation_types.hpp"

#include <concepts.hpp>
#include <_log.hpp>
#include <optional>
#include <types.hpp>

namespace estimation {
static const char *TAG = "estimation";

template <traits::serial T, traits::file_system F>
auto init_coordinate(TaskArgs<T, F> &args) -> coordinate_t {
    T &uart = args.uart;
    lgfx::LGFX_Device &lov_display = args.lov_display;
    F &spiffs = args.spiffs;

    // note that mac address is not same as node id that represents unit node
    auto this_macaddress = spiffs.read_macaddress();
    if (!this_macaddress.has_value()) {
        LOGE(TAG, "read mac address error");
        assert(false);
    }
    auto this_ip_address = macaddress_to_ip_address(this_macaddress.value());

    const network::HeadFlit ESTIMATION_FLIT = network::HeadFlit(0,
                                                                network::Header::COORDINATE_ESTIMATION,
                                                                network::SYSTEM_PACKET_ID,
                                                                this_ip_address,
                                                                network::BROADCAST_ADDRESS);

    // send message to broadcast
    network::raw_data_t raw_data;
    ESTIMATION_FLIT.to_rawdata(raw_data);
    // (mac_address, coordinate)
    auto confirmed_coordinates = std::vector<std::pair<uint32_t, coordinate_t>>();
    auto cnt = rand() % 10;

    while (is_finished(this_ip_address, confirmed_coordinates)) {
        // when send message, queue is flushed
        if (cnt == 10) {
            uart.send(raw_data);
            cnt = rand() % 10;
        }
        cnt++;

        // 1. make request
        auto packet = make_request(this_ip_address);
        auto err = packet.send(uart, this_ip_address, network::DefaultRouting());
        if (err != network::NetworkError::OK) {
            LOGE(TAG, "get head flit error");
            continue;
        }

        // 2. receive packet
        packet = network::Packet();
        auto berr = packet.receive(uart, this_ip_address);
        if (!berr) {
            LOGE(TAG, "receive packet error");
            continue;
        }

        // 3. validate packet
        err = packet.validate();
        if (err != network::NetworkError::OK) {
            lov_display.printf("packet validate error %d\n", err);
            LOGE(TAG, "packet validate error: %d", err);
            continue;
        }
        lov_display.printf("receive packet\n");
        LOGI(TAG, "receive packet");

        // check packet header
        switch (packet.get_header()) {
        case network::Header::COORDINATE_ESTIMATION: {
            if (confirmed_coordinates.size() == 0) {
                continue;
            }
            auto packet = make_response_to_same_unit(false, this_ip_address, confirmed_coordinates);
            packet.send(uart, this_ip_address, network::DefaultRouting());
            break;
        }
        case network::Header::COORDINATE_ESTIMATION_RSP: {
            // 4. get data
            auto data = std::move(packet.get_data());
            err = process_data(packet.get_src(), this_ip_address, data, confirmed_coordinates);
            if (err != network::NetworkError::OK) {
                LOGE(TAG, "process data error: %d", err);
                continue;
            }
        }
        default: {
            LOGI(TAG, "invalid header %d", (network::header_t)packet.get_header());
            continue;
        }
        }
    }

    return get_coordinate(this_ip_address, confirmed_coordinates);
}
}  // namespace estimation
