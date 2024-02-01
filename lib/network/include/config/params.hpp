#pragma once

#include <cstdint>

namespace network {

namespace header {
static constexpr std::size_t CONFIG_HEADER_SIZE = 16;
}  // namespace header

namespace flit {
// flit
static constexpr std::size_t CONFIG_CURRENT_VERSION = 0b0000;

static constexpr std::size_t CONFIG_FLIT_SIZE = 128;

static constexpr std::size_t CONFIG_VERSION_SIZE = 8;
static constexpr std::size_t CONFIG_TYPE_SIZE = 8;
static constexpr std::size_t CONFIG_FLIT_ID_SIZE = 16;
static constexpr std::size_t CONFIG_HEADER_SIZE = 16;
static constexpr std::size_t CONFIG_NODE_ID_SIZE = 16;
static constexpr std::size_t CONFIG_PACKET_ID_SIZE = 16;
static constexpr std::size_t CONFIG_OPTION_SIZE = 16;
static constexpr std::size_t CONFIG_CHECKSUM_SIZE = 16;
static constexpr std::size_t CONFIG_MESSAGE_SIZE = 80;
static constexpr std::size_t CONFIG_UNDEFINED_SIZE = 96;

// byte size
static constexpr std::size_t CONFIG_MESSAGE_LENGTH = CONFIG_MESSAGE_SIZE / 8;
static constexpr std::size_t CONFIG_FLIT_LENGTH = CONFIG_FLIT_SIZE / 8;
}  // namespace flit


// packet
// version:8:headlength:8:priority:8:packetid:16:nodeid:16:nodeid:16:
// flag:8:fragment:8:headchecksum:16:protocol:8:option:data:max()
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

}  // namespace packet

}  // namespace network