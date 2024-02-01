#include "header.hpp"

#include <cassert>

namespace network {
bool is_ack(Header &header) {
    switch (header) {
    case Header::None:
    case Header::Data:
    case Header::COORDINATE_ESTIMATION:
    case Header::COORDINATE_ESTIMATION_RSP: return false;
    default:  // unreachable
        break;
    };
    assert(false);
}
}  // namespace network