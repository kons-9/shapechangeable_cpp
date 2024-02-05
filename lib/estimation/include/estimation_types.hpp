#pragma once
#include <types.hpp>
#include <concepts.hpp>

#include <numeric>

namespace estimation {
using coordinate_unit_t = int16_t;
using coordinate_t = std::pair<coordinate_unit_t, coordinate_unit_t>;

constexpr std::size_t MACADDRESS_SIZE = 32;

constexpr std::size_t NODE_ID_SIZE = 29;
constexpr std::size_t NODE_ID_POSITION = 3;
constexpr std::size_t LOCAL_LOCATION_SIZE = 2;
constexpr std::size_t LOCAL_LOCATION_POSITION = 1;
constexpr std::size_t IS_ROOT_SIZE = 1;
constexpr std::size_t IS_ROOT_POSITION = 0;

constexpr std::size_t NODE_ID_MASK = ((1 << NODE_ID_SIZE) - 1) << NODE_ID_POSITION;
constexpr std::size_t LOCAL_LOCATION_MASK = ((1 << LOCAL_LOCATION_SIZE) - 1) << LOCAL_LOCATION_POSITION;
constexpr std::size_t IS_ROOT_MASK = ((1 << IS_ROOT_SIZE) - 1) << IS_ROOT_POSITION;

enum LocalLocation : uint8_t {
    UpperLeft = 0b00,
    UpperRight = 0b01,
    LowerRight = 0b10,
    LowerLeft = 0b11,
};

int operator==(LocalLocation lhs, LocalLocation rhs);

network::ip_address_t get_unit_node_id(network::macaddress_t macaddress);
LocalLocation get_local_location(network::macaddress_t macaddress);
bool is_root(network::macaddress_t macaddress);
bool is_same_unit_node(network::macaddress_t macaddress1, network::macaddress_t macaddress2);
network::ip_address_t macaddress_to_ip_address(network::macaddress_t macaddress);

LocalLocation diagonal_location(LocalLocation location);
coordinate_t get_root_coordinate(LocalLocation location);
LocalLocation rotate_clockwise(LocalLocation location);
LocalLocation rotate_counterclockwise(LocalLocation location);

}  // namespace estimation