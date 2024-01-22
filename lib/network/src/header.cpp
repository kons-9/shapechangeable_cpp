#include "header.hpp"
#include <cassert>
namespace header {
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