#include "estimation.hpp"
static const char *TAG = "estimation";
namespace estimation {

uint16_t is_one_distance(const coordinate_t &coordinate1, const coordinate_t &coordinate2) {
    auto distance = std::abs(coordinate1.first - coordinate2.first) + std::abs(coordinate1.second - coordinate2.second);
    return distance == 1;
}

bool is_any_one_distance_neighbor(const std::vector<std::pair<network::macaddress_t, coordinate_t>> &coordinates) {
    for (auto iter = coordinates.begin(); iter != coordinates.end(); iter++) {
        for (auto iter2 = iter + 1; iter2 != coordinates.end(); iter2++) {
            if (is_one_distance(iter->second, iter2->second)) {
                return true;
            }
        }
    }
    return false;
}

bool is_finished(network::ip_address_t this_id,
                 const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates) {
    if (is_root((network::macaddress_t)this_id)) {
        return true;
    }
    if (confirmed_coordinates.size() == 0) {
        return false;
    }
    if (confirmed_coordinates.size() == 1) {
        return confirmed_coordinates[0].first == this_id;
    }

    return is_any_one_distance_neighbor(confirmed_coordinates);
}
auto get_coordinate(network::ip_address_t this_id,
                    const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates) -> coordinate_t {
    if (is_root((network::macaddress_t)this_id)) {
        return get_root_coordinate(get_local_location(this_id));
    }
    if (confirmed_coordinates.size() == 0) {
        LOGE(TAG, "no confirmed coordinates");
        assert(false);
    }
    if (confirmed_coordinates.size() == 1) {
        // must be my coordinate
        assert(confirmed_coordinates[0].first == this_id);
        return confirmed_coordinates[0].second;
    }

    // find_one_distance_neighbor
    auto same_unit = confirmed_coordinates.end();
    auto different_unit = confirmed_coordinates.end();
    for (auto iter = confirmed_coordinates.begin(); iter != confirmed_coordinates.end(); iter++) {
        if (is_same_unit_node(this_id, iter->first)) {
            continue;
        }
        for (auto iter2 = iter + 1; iter2 != confirmed_coordinates.end(); iter2++) {
            if (is_one_distance(iter->second, iter2->second)) {
                different_unit = iter;
                same_unit = iter2;
                goto found;
            }
        }
    }
found:
    if (same_unit == confirmed_coordinates.end()) {
        LOGE(TAG, "no one distance neighbor");
        assert(false);
    }

    auto same_unit_id = same_unit->first;
    // arrow direction from same_unit_id to this_id
    bool clockwise = false;
    if (rotate_clockwise(get_local_location(same_unit_id)) == get_local_location(this_id)) {
        clockwise = true;
    } else if (rotate_counterclockwise(get_local_location(same_unit_id)) == get_local_location(this_id)) {
        clockwise = false;
    } else {
        // unreachable
        LOGE(TAG, "unreachable: same unit node must be one distance neighbor");
        assert(false);
    }

    auto neighbor_coordinate = different_unit->second;
    auto diagonal_coordinate = same_unit->second;
    bool same_unit_node_has_bigger;
    bool has_same_x = diagonal_coordinate.first == neighbor_coordinate.first;
    if (diagonal_coordinate.first == neighbor_coordinate.first) {
        assert(diagonal_coordinate.second != neighbor_coordinate.second);
        same_unit_node_has_bigger = diagonal_coordinate.second > neighbor_coordinate.second;
    } else if (diagonal_coordinate.second == neighbor_coordinate.second) {
        assert(diagonal_coordinate.first != neighbor_coordinate.first);
        same_unit_node_has_bigger = diagonal_coordinate.first > neighbor_coordinate.first;
    } else {
        // unreachable
        LOGE(TAG, "unreachable: same unit node must be one distance neighbor");
        assert(false);
    }

    if (has_same_x) {
        // if (clockwise) {
        //     if (same_unit_node_has_bigger) {
        //         // same_unit, diagonal
        //         // this , neighbor
        //         return std::make_pair(neighbor_coordinate.first - 1, neighbor_coordinate.second);
        //     } else {
        //         return std::make_pair(neighbor_coordinate.first + 1, neighbor_coordinate.second);
        //     }
        // } else {
        //     if (same_unit_node_has_bigger) {
        //         // diagonal, same_unit
        //         // neighbor, this
        //         return std::make_pair(neighbor_coordinate.first + 1, neighbor_coordinate.second);
        //     } else {
        //         return std::make_pair(neighbor_coordinate.first - 1, neighbor_coordinate.second);
        //     }
        // }
        auto add_x = (clockwise ^ same_unit_node_has_bigger) + (clockwise ^ !same_unit_node_has_bigger) * -1;
        return std::make_pair(neighbor_coordinate.first + add_x, neighbor_coordinate.second);
    } else {
        // if (clockwise) {
        //     if (same_unit_node_has_bigger) {
        //         // this, same_unit
        //         // neighbor, diagonal
        //         return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1);
        //     } else {
        //         // diagonal, neighbor
        //         // same_unit, this
        //         return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1);
        //     }
        // } else {
        //     if (same_unit_node_has_bigger) {
        //         return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1);
        //     } else {
        //         return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1);
        //     }
        // }
        auto add_y = (clockwise ^ same_unit_node_has_bigger) * -1 + (clockwise ^ !same_unit_node_has_bigger);
        return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + add_y);
    }
}
auto process_data(const network::ip_address_t src,
                  const network::ip_address_t this_ip_address,
                  const std::vector<uint8_t> &data,
                  std::vector<std::pair<network::macaddress_t, coordinate_t>> &confirmed_coordinates)
    -> network::NetworkError {
    if (data.size() < 7) {
        return network::NetworkError::EMPTY;
    }
    auto is_confirm = data[0] == 0xFF;
    network::macaddress_t mac_address = (data[1] << 8) | data[2];

    if (is_confirm) {
        // more than 1 node
        if (is_same_unit_node(this_ip_address, mac_address)) {
            // must contain my coordinate
            // find my ip address

            // must contain 3 coordinates
            // 1 is is_confirm, 6 is mac address and coordinate
            if (data.size() != 1 + 6 * 3) {
                return network::NetworkError::DATA_SIZE_ERROR;
            }
            for (auto iter = data.begin() + 1; iter != data.end(); iter += 6) {
                network::macaddress_t mac_address = ((*iter) << 8) | *(iter + 1);
                coordinate_unit_t x = (*(iter + 2) << 8) | *(iter + 3);
                coordinate_unit_t y = (*(iter + 4) << 8) | *(iter + 5);
                coordinate_t coordinate = std::make_pair(x, y);
                if (mac_address == this_ip_address) {
                    // erase all data
                    confirmed_coordinates.clear();
                    confirmed_coordinates.push_back(std::make_pair(mac_address, coordinate));
                    return network::NetworkError::OK;
                }
            }
            return network::NetworkError::INVALID_DATA;
        }
        coordinate_unit_t x = (data[3] << 8) | data[4];
        coordinate_unit_t y = (data[5] << 8) | data[6];
        coordinate_t coordinate = std::make_pair(x, y);
        // neighbor but not in the same node
        // if not contain
        if (std::find_if(confirmed_coordinates.begin(),
                         confirmed_coordinates.end(),
                         [mac_address](const auto &pair) { return pair.first == mac_address; })
            == confirmed_coordinates.end()) {
            confirmed_coordinates.push_back(std::make_pair(mac_address, coordinate));
        }
        return network::NetworkError::OK;
    } else {
        // not in the same unit node but neighbor
        if (!is_same_unit_node(this_ip_address, mac_address)) {
            // not same unit node, must be confirmed
            return network::NetworkError::INVALID_DATA;
        }
        // must contain one coordinate
        for (auto iter = data.begin() + 1; iter != data.end(); iter += 6) {
            network::macaddress_t mac_address = ((*iter) << 8) | *(iter + 1);
            coordinate_unit_t x = (*(iter + 2) << 8) | *(iter + 3);
            coordinate_unit_t y = (*(iter + 4) << 8) | *(iter + 5);
            coordinate_t coordinate = std::make_pair(x, y);
            // neighbor but not in the same node
            if (std::find_if(confirmed_coordinates.begin(),
                             confirmed_coordinates.end(),
                             [coordinate](const auto &pair) { return pair.second == coordinate; })
                == confirmed_coordinates.end()) {
                confirmed_coordinates.push_back(std::make_pair(mac_address, coordinate));
            }
        }
        return network::NetworkError::OK;
    }
}
}  // namespace estimation