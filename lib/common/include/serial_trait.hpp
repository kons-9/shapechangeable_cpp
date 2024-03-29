#pragma once

#include <types.hpp>

#include <concepts>

namespace traits {
// for uart send and receive
// because of test
enum SerialError {
    Ok,
    GenericError,
};

// just define usable concepts
template <typename T>
concept serial = requires(T a, const network::raw_data_t &raw_data, network::raw_data_t &return_raw_data,
                          network::Flit &flit, uint8_t channel) {
    { a.send(raw_data) } -> std::same_as<SerialError>;
    { a.receive(return_raw_data) } -> std::same_as<SerialError>;
    { a.send(flit) } -> std::same_as<SerialError>;
    { a.receive(flit, channel) } -> std::same_as<SerialError>;
};

}  // namespace traits
