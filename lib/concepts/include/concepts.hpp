#pragma once

#include <concepts>
#include <physical.hpp>
#include <config.hpp>

enum class SendError {
    Ok,
    UartError,
};

enum class ReceiveError {
    Ok,
    UartError,
};

// just define usable concepts
template <typename T>
concept sender = requires(T a, flit::raw_data_t &raw_data) {
    { a.send(raw_data) } -> std::same_as<SendError>;
};

// static_assert(sender<flit::HeadFlit>);

template <typename T>
concept receiver = requires(T a, flit::raw_data_t &return_value) {
    { a.receive(return_value) } -> std::same_as<ReceiveError>;
};