#include "header.hpp"
#include <_log.hpp>

namespace network {
bool is_ack(const Header header) noexcept {
    switch (header) {
    case Header::Data: return true;
    case Header::None:
    case Header::COORDINATE_ESTIMATION:
    case Header::COORDINATE_ESTIMATION_RSP: return false;
    default:  // unreachable
        LOGE(TAG, "invalid header %d", (network::header_t)header);
        break;
    };
    return false;
}

bool only_header(const Header header) noexcept {
    switch (header) {
    case Header::None:
    case Header::COORDINATE_ESTIMATION: return true;
    case Header::COORDINATE_ESTIMATION_RSP:
    case Header::Data: return false;
    default:  // unreachable
        LOGE(TAG, "invalid header %d", (network::header_t)header);
        break;
    };
    return true;
}

bool valid_header(const Header header) noexcept {
    switch (header) {
    case Header::None:
    case Header::COORDINATE_ESTIMATION:
    case Header::COORDINATE_ESTIMATION_RSP:
    case Header::Data: return true;
    default:  // unreachable
        LOGE(TAG, "invalid header %d", (network::header_t)header);
        break;
    };
    return false;
}
}  // namespace network