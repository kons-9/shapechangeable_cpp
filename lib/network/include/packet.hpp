#pragma once

#include "packet_config.hpp"
#include "error.hpp"
#include "flit.hpp"
#include "routing.hpp"

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
    length_t head_length;
    priority_t priority;
    packetid_t packetid;
    src_t global_src;
    dst_t global_dst;
    flag_t flag;
    fragment_t fragment;
    protocol_t protocol;
    // must be last since this use all varialble in initilization
    headchecksum_t headchecksum;

    message_buffer_t data;

    headchecksum_t caluculate_checksum() const;

    flit::flitid_t current_flit_index = 0;
    flit::flitid_t flit_length = 0;
    bool is_ready = false;
    bool ready() {
        if (is_ready) {
            return true;
        }
        if (data.size() > CONFIG_MTU - 1) {
            return false;
        }
        // EOF
        data.push_back(flit::FLIT_EOF);
        auto rem = data.size() % flit::CONFIG_MESSAGE_LENGTH;
        if (rem != 0) {
            // push 0 to last
            data.insert(data.end(), flit::CONFIG_MESSAGE_LENGTH - rem, 0);
        }
        flit_length = data.size() / flit::CONFIG_MESSAGE_LENGTH;
        is_ready = true;
        return true;
    }

  public:
    packetid_t get_packet_id() const {
        return packetid;
    }
    fragment_t get_fragment() const {
        return fragment;
    }
    message_buffer_t &&get_data() {
        return std::move(data);
    }
    // for load flit
    Packet() = default;
    /// @brief make a packet from flits
    /// donot check flits are valid
    Packet(length_t head_length,
           priority_t priority,
           packetid_t packetid,
           src_t src,
           dst_t dst,
           flag_t flag,
           message_buffer_t &&data)
        : version(CONFIG_CURRENT_VERSION)
        , head_length(head_length)
        , priority(priority)
        , packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , flag(flag)
        , fragment(0)
        , protocol(static_cast<protocol_t>(Protocol::DEFAULT))
        , headchecksum(caluculate_checksum())
        , data(std::move(data)) {
    }

    PacketError validate(void) const {
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
    PacketError to_flit(Flit &flit, const src_t &this_id, const network::Routing &routing);
    PacketError load_flit(Flit &&flit);

    bool operator<(const Packet &rhs) const {
        if (packetid != rhs.packetid) {
            return priority < rhs.priority;
        }

        return fragment < rhs.fragment;
    }
};

}  // namespace packet