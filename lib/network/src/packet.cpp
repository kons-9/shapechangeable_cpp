#include "packet.hpp"
#include <variant>
#include <cassert>

namespace packet {
using Flit = flit::Flit;
using FlitType = flit::FlitType;
using FlitError = flit::FlitError;

headchecksum_t Packet::caluculate_checksum() const {
    headchecksum_t checksum = 0;
    checksum += version;
    checksum += head_length;
    checksum += priority;
    checksum += packetid;
    checksum += global_src;
    checksum += global_dst;
    checksum += flag;
    checksum += fragment;
    checksum += protocol;
    return checksum;
}

PacketError Packet::to_flit(Flit &flit, const src_t &this_id, const network::Routing &routing) {
    if (current_flit_index == 0) {
        current_flit_index++;

        if (!ready()) {
            return PacketError::NOT_READY;
        }
        flit::Header header = header::Header::Data;
        auto dst = routing.next(this_id, global_dst);

        flit = Flit(flit_length, header, packetid, this_id, dst);
        assert(flit.validate() == FlitError::OK);
        assert(flit.get_type() == FlitType::Head);
        return PacketError::OK;
    } else if (current_flit_index == flit_length) {
        // make tail flit
        auto id = current_flit_index;
        auto index = (id - 1) * flit::CONFIG_MESSAGE_LENGTH;
        // data of message length
        flit.copy_packet(id, data.begin() + index, true);
        assert(flit.validate() == FlitError::OK);
        assert(flit.get_type() == FlitType::Tail);

        current_flit_index++;
        return PacketError::OK;
    } else if (current_flit_index < flit_length) {
        // make body flit
        auto id = current_flit_index;
        auto index = (id - 1) * flit::CONFIG_MESSAGE_LENGTH;
        flit.copy_packet(id, data.begin() + index, true);
        assert(flit.validate() == FlitError::OK);
        assert(flit.get_type() == FlitType::Body);

        current_flit_index++;
        return PacketError::OK;
    } else {
        return PacketError::ALREADY_FINISHED;
    }
}

PacketError Packet::load_flit(Flit &&flit) {
    // todo
    return PacketError::UNSUPPORTED;
}
}  // namespace packet