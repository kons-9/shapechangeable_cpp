#include "header.hpp"
#include <cassert>
namespace header {
bool is_ack(Header &header) {
    switch (header) {
    case Header::None:
    case Header::Data:
    case Header::COORDINATE_ESTIMATION: return false;
    default:  // unreachable
        break;
    };
    assert(false);
}
}  // namespace header