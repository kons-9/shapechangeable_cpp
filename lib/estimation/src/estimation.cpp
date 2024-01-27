#include "estimation.hpp"
#include <flit.hpp>

namespace estimation {
static const flit::HeadFlit ESTIMATION_FLIT = flit::HeadFlit(0,
                                                             flit::Header::COORDINATE_ESTIMATION,
                                                             flit::SYSTEM_PACKET_ID,
                                                             physical::local::get_ip_address(),
                                                             flit::BROADCAST_ADDRESS);

coordinate_t init_coordinate(physical::Uart &uart) {
    // send message to broadcast
    flit::raw_data_t raw_data;
    ESTIMATION_FLIT.to_rawdata(raw_data);

    while (true) {
        uart.uart_send(raw_data.size(), (char *)raw_data.data());
        flit::raw_data_t receive_raw_data;
        auto is_success = uart.uart_receive(receive_raw_data.size(), (char *)receive_raw_data.data());
        if (!is_success) {
            continue;
        }
        auto deceded_data = flit::decoder(receive_raw_data);
        if (!deceded_data.has_value()) {
            continue;
        }
        auto flit = std::move(deceded_data.value());
        if (flit->get_type() != flit::FlitType::Head) {
            continue;
        }
        auto head_flit = static_pointer_cast<flit::HeadFlit>(std::move(flit));
        
        
    }

    return std::make_pair(0, 0);
}
coordinate_t update_coordinate(coordinate_t &coordinate) {
    return std::make_pair(coordinate.first + 1, coordinate.second + 1);
}
}  // namespace estimation
