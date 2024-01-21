#include "packet.hpp"
#include <variant>

namespace packet {
using Flit = flit::Flit;
using FlitType = flit::FlitType;

headchecksum_t Packet::caluculate_checksum() {
    // todo
}

std::optional<Flit> Packet::to_flit() {
    // todo
}

PacketError Packet::load_flit(Flit &&flit) {
    // todo
}
}  // namespace packet