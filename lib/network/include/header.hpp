#pragma once

#include <config/basic_types.hpp>

namespace network {

enum class Header : header_t {
    None = 0,
    Data,
    COORDINATE_ESTIMATION,
    COORDINATE_ESTIMATION_RSP,
};

bool is_ack(Header &header);

}  // namespace network