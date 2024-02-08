#include "estimation_types.hpp"

namespace estimation {
network::ip_address_t get_unit_node_id(network::macaddress_t macaddress) {
    return (macaddress & NODE_ID_MASK) >> NODE_ID_POSITION;
}

LocalLocation get_local_location(network::macaddress_t macaddress) {
    const uint8_t local_location = (macaddress & LOCAL_LOCATION_MASK) >> LOCAL_LOCATION_POSITION;
    return static_cast<LocalLocation>(local_location);
}

bool is_root(network::macaddress_t macaddress) {
    return (macaddress & IS_ROOT_MASK) >> IS_ROOT_POSITION;
}

bool is_same_unit_node(network::macaddress_t macaddress1, network::macaddress_t macaddress2) {
    return get_unit_node_id(macaddress1) == get_unit_node_id(macaddress2);
}

network::ip_address_t macaddress_to_ip_address(network::macaddress_t macaddress) {
    return macaddress & std::numeric_limits<network::ip_address_t>::max();
}

network::macaddress_t change_local_location(network::macaddress_t macaddress, LocalLocation local_location) {
    return (macaddress & ~LOCAL_LOCATION_MASK)
           | (static_cast<network::macaddress_t>(local_location) << LOCAL_LOCATION_POSITION);
}
}  // namespace estimation