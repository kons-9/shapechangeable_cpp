#pragma once
#include "config.hpp"

#include <cassert>

namespace header {
enum class Header : header_t {
    None = 0,
    Data,
    Ack,
};

bool is_ack(Header &header) {
    switch (header) {
    case Header::None:
    case Header::Data: return false;
    case Header::Ack: return true;
    default:  // unreachable
        break;
    };
    assert(false);
}

}  // namespace header