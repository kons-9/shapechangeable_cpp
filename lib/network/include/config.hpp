#pragma once

#include "error.hpp"

#include <array>
#include <cstdint>

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

using raw_data_t = std::array<uint8_t, CONFIG_FLIT_LENGTH>;

using flittype_t = uint8_t;
using header_t = uint16_t;
using version_t = uint8_t;
using flitid_t = uint16_t;
using node_id_t = uint16_t;
using packetid_t = uint16_t;
using checksum_t = uint16_t;
using option_t = uint16_t;

using message_element_t = uint8_t;
using message_t = std::array<message_element_t, CONFIG_MESSAGE_LENGTH>;

static_assert(8 * CONFIG_FLIT_LENGTH == CONFIG_FLIT_SIZE);
static_assert(sizeof(flittype_t) == CONFIG_TYPE_SIZE / 8);
static_assert(sizeof(header_t) == CONFIG_HEADER_SIZE / 8);
static_assert(sizeof(version_t) == CONFIG_VERSION_SIZE / 8);
static_assert(sizeof(flitid_t) == CONFIG_FLIT_ID_SIZE / 8);
static_assert(sizeof(node_id_t) == CONFIG_NODE_ID_SIZE / 8);
static_assert(sizeof(packetid_t) == CONFIG_PACKET_ID_SIZE / 8);
static_assert(sizeof(checksum_t) == CONFIG_CHECKSUM_SIZE / 8);
static_assert(sizeof(option_t) == CONFIG_OPTION_SIZE / 8);
static_assert(8 * sizeof(message_element_t) * CONFIG_MESSAGE_LENGTH == CONFIG_MESSAGE_SIZE);

// nope assert
static_assert(CONFIG_FLIT_SIZE
              == CONFIG_VERSION_SIZE + CONFIG_TYPE_SIZE + CONFIG_UNDEFINED_SIZE + CONFIG_CHECKSUM_SIZE);

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
}  // namespace flit
