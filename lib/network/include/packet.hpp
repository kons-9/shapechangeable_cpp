#pragma once
#include "types.hpp"
#include "flit.hpp"
#include "routing.hpp"

#include <concepts.hpp>

#include <algorithm>
#include <expected>
#include <optional>
#include <cassert>


namespace network {

enum class Protocol : protocol_t {
    DEFAULT = 1,
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
    version_t version = packet::CONFIG_CURRENT_VERSION;
    length_t head_length = packet::CONFIG_NORMAL_HEADER_SIZE;
    priority_t priority = 3;
    packetid_t packetid;
    ip_address_t global_src;
    ip_address_t global_dst = BROADCAST_ADDRESS;
    flag_t flag = BitFlag::NONE;
    fragment_t fragment = 0;
    protocol_t protocol = static_cast<protocol_t>(Protocol::DEFAULT);
    // must be last since this use all varialble in initilization
    headchecksum_t headchecksum;
    Header header = Header::Data;

    message_t data;
    message_t head_raw_data;
    std::size_t data_index = 0;    // for send
    std::size_t header_index = 0;  // for send
    bool header_finished = false;  // for receive

    flitid_t current_flit_index = 0;
    flitid_t flit_length = 1;

    headchecksum_t caluculate_checksum() const;
    bool send_ready(void);
    NetworkError parse_header(const message_t &data, size_t &last_index);
    std::vector<message_element_t> make_header() const;
    flitid_t calculate_flit_length(const message_t &data) const noexcept;

  public:
    bool is_send_finished(void) {
        return data_index == data.size();
    }
    bool is_receive_finished(void) {
        return header_finished && flit_length == current_flit_index;
    }
    packetid_t get_packet_id(void) const noexcept {
        return packetid;
    }
    fragment_t get_fragment(void) const noexcept {
        return fragment;
    }
    std::vector<message_element_t> get_data(void) {
        // move all data
        // must execute after ready or receive
        // find eof
        while (data.back() != flit::FLIT_EOF) {
            auto val = data.back();
            data.pop_back();
        }
        data.pop_back();
        auto data = std::move(this->data);
        this->data.clear();
        return data;
    }
    Header get_header(void) const noexcept {
        return header;
    }

    ip_address_t get_src(void) const noexcept {
        return global_src;
    }
    flitid_t get_flit_length(void) const noexcept {
        return flit_length;
    }

    std::vector<message_element_t> get_header_raw_data(void) const noexcept {
        return head_raw_data;
    }

    // for load flit
    Packet() = default;
    Packet(Header header, packetid_t packetid, ip_address_t src, ip_address_t dst)
        : packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , headchecksum(caluculate_checksum())
        , header(header) {
        head_raw_data = make_header();
    };
    // Header::Data
    Packet(packetid_t packetid, ip_address_t src, ip_address_t dst, std::vector<message_element_t> message)
        : packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , headchecksum(caluculate_checksum()) {
        flit_length = calculate_flit_length(message);
        head_raw_data = make_header();
        data = std::move(message);
    };
    Packet(Header header,
           packetid_t packetid,
           ip_address_t src,
           ip_address_t dst,
           std::vector<message_element_t> message)
        : packetid(packetid)
        , global_src(src)
        , global_dst(dst)
        , headchecksum(caluculate_checksum())
        , header(header) {
        flit_length = calculate_flit_length(message);
        head_raw_data = make_header();
        data = std::move(message);
    };

    NetworkError validate(void) const;

    // load flit one by one
    std::expected<Flit, NetworkError> to_flit(const ip_address_t this_id, const Routing &routing) noexcept;
    NetworkError load_flit(const network::ip_address_t this_id, Flit &&flit) noexcept;

    template <traits::serial T>
    NetworkError send(T &sender, const ip_address_t this_id, const network::Routing &routing) {
        // send all flits
        while (is_send_finished() == false) {
            auto exp_flit = to_flit(this_id, routing);
            auto is_success = exp_flit.has_value();
            if (!is_success) {
                return exp_flit.error();
            }
            auto flit = std::move(exp_flit.value());
            auto err = std::visit([&](auto &&flit) { return flit.send(sender); }, flit);
            if (err != NetworkError::OK) {
                return err;
            }
        }
        return NetworkError::OK;
    }

    template <traits::serial T>
    bool receive(T &receiver, macaddress_t this_id) {
        while (true) {
            Flit flit;
            auto serial_err = receive(receiver, flit);
            if (serial_err != traits::SerialError::Ok) {
                return false;
            }
            auto err = load_flit(this_id, std::move(flit));
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