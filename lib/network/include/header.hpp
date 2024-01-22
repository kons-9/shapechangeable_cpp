#pragma once

#include "config.hpp"

namespace header {
enum class Header : header_t {
    None = 0,
    Data,
    Ack,
};

bool is_ack(Header &header);

}  // namespace header