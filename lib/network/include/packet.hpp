#pragma once
#include "concepts.hpp"

#include "packet_config.hpp"
#include "error.hpp"
#include "flit.hpp"
#include "routing.hpp"

#include <algorithm>
#include <expected>
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
    bool ready() {
        if (data.size() > CONFIG_MTU - 1) {
            return false;
        }
        // EOF
        data.back().push_back(flit::FLIT_EOF);
        auto rem = data.back().size() % flit::CONFIG_MESSAGE_LENGTH;
        if (rem != 0) {
            // push 0 to last
            data.back().insert(data.back().end(), flit::CONFIG_MESSAGE_LENGTH - rem, 0);
        }
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
    Packet(length_t head_length, priority_t priority, packetid_t packetid, src_t src, dst_t dst, flag_t flag)
        : version(CONFIG_CURRENT_VERSION)
        , head_length(head_length)
        , priority(priority)
        , packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , flag(flag)
        , fragment(0)
        , protocol(static_cast<protocol_t>(Protocol::DEFAULT))
        , data(){};

    Packet(length_t head_length,
           priority_t priority,
           packetid_t packetid,
           src_t src,
           dst_t dst,
           flag_t flag,
           std::vector<flit::message_element_t> &message)
        : version(CONFIG_CURRENT_VERSION)
        , head_length(head_length)
        , priority(priority)
        , packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , flag(flag)
        , fragment(0)
        , protocol(static_cast<protocol_t>(Protocol::DEFAULT))
        , headchecksum(caluculate_checksum()) {
        // make header
        auto header = make_header();

        // make data
        data = message_buffer_t();
        data.push_back(std::move(header));
        // one flit is CONFIG_MESSAGE_LENGTH
        for (auto i = 0; i < message.size(); i += flit::CONFIG_MESSAGE_LENGTH) {
            auto begin = message.begin() + i;
            auto end = message.begin() + std::min(i + flit::CONFIG_MESSAGE_LENGTH, message.size());
            data.push_back(std::vector<flit::message_element_t>(begin, end));
        }
    }
    std::vector<flit::message_element_t> make_header() const {
        std::vector<flit::message_element_t> header;
        header.push_back(version);
        header.push_back(head_length);
        header.push_back(priority);
        header.push_back(packetid >> 8);
        header.push_back(packetid & 0xff);
        header.push_back(global_src >> 8);
        header.push_back(global_src & 0xff);
        header.push_back(global_dst >> 8);
        header.push_back(global_dst & 0xff);
        header.push_back(flag);
        header.push_back(fragment);
        header.push_back(headchecksum >> 8);
        header.push_back(headchecksum & 0xff);
        header.push_back(protocol);
        return header;
    }

    PacketError validate(void) const {
        if (flag & HASFRAGMENT || flag & LASTFRAGMENT || fragment != 0) {
            // currently, unsupported
            return PacketError::UNSUPPORTED;
        }
        auto length = (data.size() - 1) * flit::CONFIG_MESSAGE_LENGTH + data.back().size();
        if (length > CONFIG_MTU - 1) {
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
    std::expected<std::unique_ptr<Flit>, PacketError> to_flit(const src_t this_id,
                                                              const network::Routing &routing) noexcept;
    PacketError load_flit(std::unique_ptr<flit::Flit> &&flit) noexcept;

    template <sender T> PacketError send(T &sender, const flit::node_id_t this_id, const network::Routing &routing) {
        // send all flits
        while (true) {
            auto flit = to_flit(this_id, routing);
            auto is_success = flit.has_value();
            if (!is_success) {
                return flit.error();
            }
            auto flit_ptr = std::move(flit.value());
            flit_ptr->send(sender);
        }
        return PacketError::OK;
    }

    template <receiver T> bool receive(T &receiver) {
        while (true) {
            std::unique_ptr<flit::Flit> flit = flit::receive(receiver);
            auto err = load_flit(std::move(flit));
            if (err != PacketError::OK) {
                return false;
            }
        }
    }


    bool operator<(const Packet &rhs) const {
        if (packetid != rhs.packetid) {
            return priority < rhs.priority;
        }

        return fragment < rhs.fragment;
    }
};

}  // namespace packet