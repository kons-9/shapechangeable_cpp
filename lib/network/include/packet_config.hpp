#pragma once
// packet
// version:8:headlength:8:priority:8:packetid:16:nodeid:16:nodeid:16:
// flag:8:fragment:8:headchecksum:16:protocol:8:option:data:max()
#include "error.hpp"
#include "config.hpp"
#include "vector"

#include <cstdint>

namespace packet {
static constexpr std::size_t CONFIG_CURRENT_VERSION = 0b0000;

static constexpr std::size_t CONFIG_VERSION_SIZE = 8;
static constexpr std::size_t CONFIG_LEN_FILED_SIZE = 8;
static constexpr std::size_t CONFIG_PRIORITY_SIZE = 8;
static constexpr std::size_t CONFIG_ID_SIZE = 16;
static constexpr std::size_t CONFIG_SRC_SIZE = 16;
static constexpr std::size_t CONFIG_DST_SIZE = 16;
static constexpr std::size_t CONFIG_FLAG_SIZE = 8;
static constexpr std::size_t CONFIG_FRAGMENT_SIZE = 8;
static constexpr std::size_t CONFIG_HEAD_CHECKSUM_SIZE = 16;
static constexpr std::size_t CONFIG_PROTOCOL_SIZE = 8;

// byte size
// must be multiple of flit::CONFIG_MESSAGE_LENGTH
static constexpr std::size_t CONFIG_MTU = 1500;
static_assert(CONFIG_MTU % flit::CONFIG_MESSAGE_LENGTH == 0);

using version_t = uint8_t;
using length_t = uint8_t;
using priority_t = uint8_t;
using packetid_t = uint16_t;
using src_t = uint16_t;
using dst_t = uint16_t;
using flag_t = uint8_t;
using fragment_t = uint8_t;
using headchecksum_t = uint16_t;
using protocol_t = uint8_t;

using message_element_t = uint8_t;
using message_buffer_t = std::vector<message_element_t>;

static_assert(sizeof(version_t) == CONFIG_VERSION_SIZE / 8);
static_assert(sizeof(length_t) == CONFIG_LEN_FILED_SIZE / 8);
static_assert(sizeof(priority_t) == CONFIG_PRIORITY_SIZE / 8);
static_assert(sizeof(packetid_t) == CONFIG_ID_SIZE / 8);
static_assert(sizeof(src_t) == CONFIG_SRC_SIZE / 8);
static_assert(sizeof(dst_t) == CONFIG_DST_SIZE / 8);
static_assert(sizeof(flag_t) == CONFIG_FLAG_SIZE / 8);
static_assert(sizeof(fragment_t) == CONFIG_FRAGMENT_SIZE / 8);
static_assert(sizeof(headchecksum_t) == CONFIG_HEAD_CHECKSUM_SIZE / 8);
static_assert(sizeof(protocol_t) == CONFIG_PROTOCOL_SIZE / 8);

static_assert(sizeof(packetid_t) == sizeof(flit::packetid_t));
static_assert(CONFIG_MTU <= flit::CONFIG_MESSAGE_SIZE * 1 << (flit::CONFIG_FLIT_ID_SIZE));
static_assert(sizeof(src_t) == sizeof(flit::node_id_t));
static_assert(sizeof(dst_t) == sizeof(flit::node_id_t));
static_assert(sizeof(message_element_t) == sizeof(flit::message_element_t));

}  // namespace packet
