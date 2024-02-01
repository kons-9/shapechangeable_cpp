#pragma once
#include "types.hpp"
#include "flit.hpp"
#include "routing.hpp"

#include <concepts.hpp>

#include <algorithm>
#include <expected>
#include <optional>


namespace network {

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
    ip_address_t global_src;
    ip_address_t global_dst;
    flag_t flag;
    fragment_t fragment;
    protocol_t protocol;
    // must be last since this use all varialble in initilization
    headchecksum_t headchecksum;
    Header header;

    message_buffer_t data;

    headchecksum_t caluculate_checksum() const;

    flitid_t current_flit_index = 0;
    bool ready() {
        if (data.size() > packet::CONFIG_MTU - 1) {
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
    std::vector<message_element_t> get_data() {
        std::vector<message_element_t> data;
        // move all data
        for (auto &flit : this->data) {
            std::move(flit.begin(), flit.end(), std::back_inserter(data));
        }
        this->data.clear();

        return data;
    }
    Header get_header() const {
        return header;
    }

    ip_address_t get_src() const {
        return global_src;
    }
    // for load flit
    Packet() = default;
    /// @brief make a packet from flits
    /// donot check flits are valid
    Packet(length_t head_length,
           priority_t priority,
           packetid_t packetid,
           ip_address_t src,
           ip_address_t dst,
           flag_t flag)
        : version(packet::CONFIG_CURRENT_VERSION)
        , head_length(head_length)
        , priority(priority)
        , packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , flag(flag)
        , fragment(0)
        , protocol(static_cast<protocol_t>(Protocol::DEFAULT))
        , headchecksum(caluculate_checksum())
        , header(Header::Data) {
        auto header = make_header();
        data.push_back(std::move(header));
    };

    Packet(length_t head_length,
           priority_t priority,
           packetid_t packetid,
           ip_address_t src,
           ip_address_t dst,
           flag_t flag,
           std::vector<message_element_t> &message)
        : version(packet::CONFIG_CURRENT_VERSION)
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
            data.push_back(std::vector<message_element_t>(begin, end));
        }
    }
    std::vector<message_element_t> make_header() const {
        std::vector<message_element_t> header;
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

    NetworkError validate(void) const {
        if (flag & HASFRAGMENT || flag & LASTFRAGMENT || fragment != 0) {
            // currently, unsupported
            return NetworkError::UNSUPPORTED;
        }
        auto length = (data.size() - 1) * flit::CONFIG_MESSAGE_LENGTH + data.back().size();
        if (length > packet::CONFIG_MTU - 1) {
            return NetworkError::OVER_MTU;
        }
        if (headchecksum != caluculate_checksum()) {
            return NetworkError::CHECKSUM_NOT_MATCH;
        }
        if (version > packet::CONFIG_CURRENT_VERSION) {
            return NetworkError::VERSION_UNSUPPORTED;
        }
        return NetworkError::OK;
    }

    // load flit one by one
    std::expected<std::unique_ptr<Flit>, NetworkError> to_flit(const ip_address_t this_id,
                                                               const Routing &routing) noexcept;
    NetworkError load_flit(std::unique_ptr<Flit> &&flit) noexcept;

    template <traits::serial T>
    NetworkError send(T &sender, const ip_address_t this_id, const network::Routing &routing) {
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
        return NetworkError::OK;
    }

    template <traits::serial T> bool receive(T &receiver) {
        while (true) {
            std::unique_ptr<Flit> flit = receive(receiver);
            auto err = load_flit(std::move(flit));
            if (err != NetworkError::OK) {
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

}  // namespace network