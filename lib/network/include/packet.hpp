#pragma once

#include "packet_config.hpp"
#include "error.hpp"
#include "flit.hpp"

#include <algorithm>
#include <variant>
#include <optional>

namespace packet {
using Flit = flit::Flit;

enum class Protocol : protocol_t {
    DEFAULT,
};

enum BitFlag : flag_t {
    NONE = 1 << 0,

    HASFRAGMENT = 1 << 1,
    LASTFRAGMENT = 1 << 2,
};

class Packet {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    length_t length;
    priority_t priority;
    packetid_t packetid;
    src_t src;
    dst_t dst;
    flag_t flag;
    fragment_t fragment;
    protocol_t protocol;
    // must be last since this use all varialble in initilization
    headchecksum_t headchecksum;

    message_buffer_t data;

    headchecksum_t caluculate_checksum();

    std::size_t current_flit_index = 0;

  public:
    packetid_t get_packet_id() {
        return packetid;
    }
    fragment_t get_fragment() {
        return fragment;
    }
    message_buffer_t &&get_data() {
        return std::move(data);
    }
    Packet() = default;
    /// @brief make a packet from flits
    /// donot check flits are valid
    Packet(length_t length,
           priority_t priority,
           packetid_t packetid,
           src_t src,
           dst_t dst,
           flag_t flag,
           message_buffer_t &&data)
        : version(CONFIG_CURRENT_VERSION)
        , length(length)
        , priority(priority)
        , packetid(packetid)
        , src(src)
        , dst(dst)
        , flag(flag)
        , fragment(0)
        , protocol(static_cast<protocol_t>(Protocol::DEFAULT))
        , headchecksum(caluculate_checksum())
        , data(std::move(data)) {
    }

    PacketError validate() {
        if (flag & HASFRAGMENT || flag & LASTFRAGMENT || fragment != 0) {
            // currently, unsupported
            return PacketError::UNSUPPORTED;
        }
        if (data.size() > CONFIG_MTU) {
            return PacketError::OVER_MTU;
        }
        if (headchecksum != caluculate_checksum()) {
            return PacketError::CHECKSUM_NOT_MATCH;
        }
        if (version > CONFIG_CURRENT_VERSION) {
            return PacketError::VERSION_UNSUPPORTED;
        }
        return PacketError::OK;
    }

    // load flit one by one
    std::optional<Flit> to_flit();
    PacketError load_flit(Flit &&flit);

    bool operator<(const Packet &rhs) const {
        if (packetid != rhs.packetid) {
            return priority < rhs.priority;
        }

        return fragment < rhs.fragment;
    }
};

}  // namespace packet