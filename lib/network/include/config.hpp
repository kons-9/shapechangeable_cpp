#pragma once
#include "error.hpp"

#include <array>
#include <cstdint>

// headflit:
//     version:4:flittype:4:flitid:32:header:8:nodeid:16:nodeid:16:packetid:16:option:16:checksum:16
// bodyflit:
// tailflit:
//     version:4:flittype:4:flitid:32:message:72:checksum:16
static constexpr std::size_t CONFIG_CURRENT_VERSION = 0b0000;

static constexpr std::size_t CONFIG_FLIT_SIZE = 128;

static constexpr std::size_t CONFIG_VERSION_SIZE = 4;
static constexpr std::size_t CONFIG_TYPE_SIZE = 4;
static constexpr std::size_t CONFIG_FLIT_ID_SIZE = 32;
static constexpr std::size_t CONFIG_HEADER_SIZE = 8;
static constexpr std::size_t CONFIG_NODE_ID_SIZE = 16;
static constexpr std::size_t CONFIG_PACKET_ID_SIZE = 16;
static constexpr std::size_t CONFIG_OPTION_SIZE = 16;
static constexpr std::size_t CONFIG_CHECKSUM_SIZE = 16;
static constexpr std::size_t CONFIG_MESSAGE_SIZE = 72;

// byte size
static constexpr std::size_t CONFIG_MESSAGE_LENGTH = CONFIG_MESSAGE_SIZE / 8;
static constexpr std::size_t CONFIG_FLIT_LENGTH = CONFIG_FLIT_SIZE / 8;

// head assert
static_assert(CONFIG_FLIT_SIZE
              == CONFIG_VERSION_SIZE + CONFIG_TYPE_SIZE + CONFIG_FLIT_ID_SIZE + CONFIG_HEADER_SIZE + CONFIG_NODE_ID_SIZE
                     + CONFIG_NODE_ID_SIZE + CONFIG_PACKET_ID_SIZE + CONFIG_OPTION_SIZE + CONFIG_CHECKSUM_SIZE);

// body assert
static_assert(CONFIG_FLIT_SIZE
              == CONFIG_VERSION_SIZE + CONFIG_TYPE_SIZE + CONFIG_FLIT_ID_SIZE + CONFIG_MESSAGE_SIZE
                     + CONFIG_CHECKSUM_SIZE);

// tail assert
static_assert(CONFIG_FLIT_SIZE
              == CONFIG_VERSION_SIZE + CONFIG_TYPE_SIZE + CONFIG_FLIT_ID_SIZE + CONFIG_MESSAGE_SIZE
                     + CONFIG_CHECKSUM_SIZE);

namespace flit {
using raw_data_t = std::array<uint8_t, CONFIG_FLIT_LENGTH>;
static_assert(8 * CONFIG_FLIT_LENGTH == CONFIG_FLIT_SIZE);

using version_t = uint8_t;
using flitid_t = uint32_t;
using node_id_t = uint16_t;
using packetid_t = uint16_t;
using checksum_t = uint16_t;
using option_t = uint16_t;
static_assert(sizeof(version_t) >= CONFIG_VERSION_SIZE / 8);
static_assert(sizeof(flitid_t) == CONFIG_FLIT_ID_SIZE / 8);
static_assert(sizeof(node_id_t) == CONFIG_NODE_ID_SIZE / 8);
static_assert(sizeof(packetid_t) == CONFIG_PACKET_ID_SIZE / 8);
static_assert(sizeof(checksum_t) == CONFIG_CHECKSUM_SIZE / 8);
static_assert(sizeof(option_t) == CONFIG_OPTION_SIZE / 8);

using message_t = std::array<uint8_t, CONFIG_MESSAGE_LENGTH>;
static_assert(8 * CONFIG_MESSAGE_LENGTH == CONFIG_MESSAGE_SIZE);
}  // namespace flit