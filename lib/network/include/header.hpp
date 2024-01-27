#pragma once

#include "config.hpp"

namespace header {
enum class Header : header_t {
    None = 0,
    Data,
    COORDINATE_ESTIMATION,
};

bool is_ack(Header &header);

}  // namespace header