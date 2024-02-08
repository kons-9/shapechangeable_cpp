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
                    const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates)
    -> std::vector<std::pair<network::macaddress_t, coordinate_t>> {
    if (is_root((network::macaddress_t)this_id)) {
        return {std::pair(this_id, get_root_coordinate(get_local_location(this_id)))};
    }
    if (confirmed_coordinates.size() == 0) {
        LOGE(TAG, "no confirmed coordinates");
        assert(false);
    }
    std::vector<std::pair<network::macaddress_t, coordinate_t>> unit_coordinates;

    for (auto i = 0; i < confirmed_coordinates.size(); i++) {
        if (is_same_unit_node(this_id, confirmed_coordinates[i].first)) {
            unit_coordinates.push_back(confirmed_coordinates[i]);
        }
        if (confirmed_coordinates[i].first == this_id) {
            // swap to first
            std::swap(unit_coordinates[0], unit_coordinates[i]);
        }
    }
    if (!unit_coordinates.empty() && unit_coordinates[0].first == this_id) {
        return unit_coordinates;
    }
    unit_coordinates.clear();

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

    coordinate_t this_coordinate;
    coordinate_t same_unit_coordinate;

    if (has_same_x) {
        if (clockwise) {
            if (same_unit_node_has_bigger) {
                // same_unit, diagonal
                // this , neighbor
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first - 1, neighbor_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first - 1, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first - 2, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(rotate_counterclockwise(
                                                             get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first - 2, neighbor_coordinate.second)));
            } else {
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first + 1, neighbor_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first + 1, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first + 2, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(rotate_counterclockwise(
                                                             get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first + 2, neighbor_coordinate.second)));
            }
        } else {
            if (same_unit_node_has_bigger) {
                // diagonal, same_unit
                // neighbor, this
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first + 1, neighbor_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first + 1, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first + 2, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(
                                                             rotate_clockwise(get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first + 2, neighbor_coordinate.second)));
            } else {
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first - 1, neighbor_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first - 1, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first - 2, diagonal_coordinate.second)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(
                                                             rotate_clockwise(get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first - 2, neighbor_coordinate.second)));
            }
        }
    } else {
        if (clockwise) {
            if (same_unit_node_has_bigger) {
                // this, same_unit
                // neighbor, diagonal
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second + 1)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second + 2)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(rotate_counterclockwise(
                                                             get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 2)));

                // return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1);
            } else {
                // diagonal, neighbor
                // same_unit, this
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second - 1)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second - 2)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_counterclockwise(rotate_counterclockwise(
                                                             get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 2)));
                // return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1);
            }
        } else {
            if (same_unit_node_has_bigger) {
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second - 1)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second - 2)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(
                                                             rotate_clockwise(get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 2)));
                // return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second - 1);
            } else {
                unit_coordinates.push_back(
                    std::make_pair(this_id, std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1)));
                unit_coordinates.push_back(
                    std::make_pair(same_unit_id,
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second + 1)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(get_local_location(same_unit_id))),
                                   std::make_pair(diagonal_coordinate.first, diagonal_coordinate.second + 2)));
                unit_coordinates.push_back(
                    std::make_pair(change_local_location(same_unit_id,
                                                         rotate_clockwise(
                                                             rotate_clockwise(get_local_location(same_unit_id)))),
                                   std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 2)));
                // return std::make_pair(neighbor_coordinate.first, neighbor_coordinate.second + 1);
            }
        }
    }
    return unit_coordinates;
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
            // if (data.size() != 1 + 6 * 3) {
            //     return network::NetworkError::DATA_SIZE_ERROR;
            // }
            auto flag = false;
            confirmed_coordinates.clear();
            for (auto iter = data.begin() + 1; iter != data.end(); iter += 6) {
                network::macaddress_t mac_address = ((*iter) << 8) | *(iter + 1);
                coordinate_unit_t x = (*(iter + 2) << 8) | *(iter + 3);
                coordinate_unit_t y = (*(iter + 4) << 8) | *(iter + 5);
                coordinate_t coordinate = std::make_pair(x, y);
                confirmed_coordinates.push_back(std::make_pair(mac_address, coordinate));
                if (mac_address == this_ip_address) {
                    flag = true;
                }
            }
            if (!flag) {
                confirmed_coordinates.clear();
                return network::NetworkError::INVALID_DATA;
            }
            return network::NetworkError::OK;
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

auto make_response_to_same_unit(
    bool is_confirm,
    const network::ip_address_t this_ip_address,
    const std::vector<std::pair<network::macaddress_t, coordinate_t>> &confirmed_coordinates) -> network::Packet {

    auto data = std::vector<uint8_t>();
    data.push_back(is_confirm ? 0xFF : 0x00);

    auto filtered_coordinates = std::vector<std::pair<network::macaddress_t, coordinate_t>>();
    if (is_confirm) {
        // filter same unit node
        for (auto iter = confirmed_coordinates.begin(); iter != confirmed_coordinates.end(); iter++) {
            if (is_same_unit_node(this_ip_address, iter->first)) {
                filtered_coordinates.push_back(*iter);
            }
        }
    } else {
        // filter not same unit node and replace mac address to this ip address
        for (auto iter = confirmed_coordinates.begin(); iter != confirmed_coordinates.end(); iter++) {
            if (!is_same_unit_node(this_ip_address, iter->first)) {
                filtered_coordinates.push_back(std::make_pair(this_ip_address, iter->second));
            }
        }
    }

    for (auto iter = filtered_coordinates.begin(); iter != filtered_coordinates.end(); iter++) {
        data.push_back(static_cast<uint8_t>(iter->first >> 8));
        data.push_back(static_cast<uint8_t>(iter->first));
        data.push_back(static_cast<uint8_t>(iter->second.first >> 8));
        data.push_back(static_cast<uint8_t>(iter->second.first));
        data.push_back(static_cast<uint8_t>(iter->second.second >> 8));
        data.push_back(static_cast<uint8_t>(iter->second.second));
    }

    return network::Packet(network::Header::COORDINATE_ESTIMATION_RSP,
                           network::packetid_t(0),
                           this_ip_address,
                           network::BROADCAST_ADDRESS,
                           std::move(data));
}
auto make_response_to_other_unit(const network::ip_address_t this_ip_address, const coordinate_t confirmed_coordinates)
    -> network::Packet {
    auto data = std::vector<uint8_t>{
        0xFF,
        static_cast<uint8_t>(this_ip_address >> 8),
        static_cast<uint8_t>(this_ip_address),
        static_cast<uint8_t>(confirmed_coordinates.first >> 8),
        static_cast<uint8_t>(confirmed_coordinates.first),
        static_cast<uint8_t>(confirmed_coordinates.second >> 8),
        static_cast<uint8_t>(confirmed_coordinates.second),
    };

    return network::Packet(network::Header::COORDINATE_ESTIMATION_RSP,
                           network::packetid_t(0),
                           this_ip_address,
                           network::BROADCAST_ADDRESS,
                           std::move(data));
}
auto make_request(const network::ip_address_t this_ip_address) -> network::Packet {
    return network::Packet(network::Header::COORDINATE_ESTIMATION, this_ip_address);
}
}  // namespace estimation