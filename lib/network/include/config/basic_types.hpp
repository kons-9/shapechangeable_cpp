#pragma once
#include "params.hpp"

#include <vector>
#include <array>
#include <cstdint>
#include <numeric>

namespace network {
using macaddress_t = uint32_t;

using raw_data_t = std::array<uint8_t, flit::CONFIG_FLIT_LENGTH>;

// flit
using flittype_t = uint8_t;
using header_t = uint16_t;
using version_t = uint8_t;
using flitid_t = uint16_t;
using ip_address_t = uint16_t;
using packetid_t = uint16_t;
using checksum_t = uint16_t;
using option_t = uint16_t;

using message_element_t = uint8_t;
using message_t = std::vector<message_element_t>;
namespace flit {
static constexpr message_element_t FLIT_EOF = std::numeric_limits<message_element_t>::max();
static constexpr ip_address_t BROADCAST_ADDRESS = std::numeric_limits<ip_address_t>::max();
static constexpr packetid_t SYSTEM_PACKET_ID = 0;
static_assert(FLIT_EOF == 0xFF);
}  // namespace flit

// packet
using length_t = uint8_t;
using priority_t = uint8_t;
using flag_t = uint8_t;
using fragment_t = uint8_t;
using headchecksum_t = uint16_t;
using protocol_t = uint8_t;

using message_buffer_t = std::vector<message_t>;
// flit assertion

namespace flit {
static_assert(8 * CONFIG_FLIT_LENGTH == CONFIG_FLIT_SIZE);
static_assert(sizeof(flittype_t) == CONFIG_TYPE_SIZE / 8);
static_assert(sizeof(header_t) == CONFIG_HEADER_SIZE / 8);
static_assert(sizeof(version_t) == CONFIG_VERSION_SIZE / 8);
static_assert(sizeof(flitid_t) == CONFIG_FLIT_ID_SIZE / 8);
static_assert(sizeof(ip_address_t) == CONFIG_NODE_ID_SIZE / 8);
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


// packet assertion
static_assert(sizeof(version_t) == packet::CONFIG_VERSION_SIZE / 8);
static_assert(sizeof(length_t) == packet::CONFIG_LEN_FILED_SIZE / 8);
static_assert(sizeof(priority_t) == packet::CONFIG_PRIORITY_SIZE / 8);
static_assert(sizeof(packetid_t) == packet::CONFIG_ID_SIZE / 8);
static_assert(sizeof(ip_address_t) == packet::CONFIG_SRC_SIZE / 8);
static_assert(sizeof(ip_address_t) == packet::CONFIG_DST_SIZE / 8);
static_assert(sizeof(flag_t) == packet::CONFIG_FLAG_SIZE / 8);
static_assert(sizeof(fragment_t) == packet::CONFIG_FRAGMENT_SIZE / 8);
static_assert(sizeof(headchecksum_t) == packet::CONFIG_HEAD_CHECKSUM_SIZE / 8);
static_assert(sizeof(protocol_t) == packet::CONFIG_PROTOCOL_SIZE / 8);

static_assert(packet::CONFIG_MTU <= flit::CONFIG_MESSAGE_SIZE << (flit::CONFIG_FLIT_ID_SIZE));

}  // namespace network