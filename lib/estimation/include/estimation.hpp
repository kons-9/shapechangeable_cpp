#pragma once

#include "estimation_types.hpp"
#include <task_args.hpp>
#include <types.hpp>
#include <flit.hpp>
#include <packet.hpp>

#include <cstdint>
#include <utility>
#include <vector>

namespace estimation {

template <traits::serial T, traits::file_system F>
coordinate_t init_coordinate(TaskArgs<T, F> &args);

// test done
bool is_finished(network::ip_address_t this_id,
                 const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates);
auto get_coordinate(network::ip_address_t this_id,
                    const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates)
    -> std::vector<std::pair<network::macaddress_t, coordinate_t>>;

bool is_any_one_distance_neighbor(const std::vector<std::pair<network::macaddress_t, coordinate_t>> &coordinates);

auto process_data(const network::ip_address_t src,
                  const network::ip_address_t this_ip_address,
                  const std::vector<uint8_t> &data,
                  std::vector<std::pair<network::macaddress_t, coordinate_t>> &confirmed_coordinates)
    -> network::NetworkError;


auto make_response_to_same_unit(
    bool is_confirm,
    const network::ip_address_t this_ip_address,
    const std::vector<std::pair<network::macaddress_t, coordinate_t>> &confirmed_coordinates) -> network::Packet;
auto make_response_to_other_unit(const network::ip_address_t this_ip_address, const coordinate_t confirmed_coordinates)
    -> network::Packet;
auto make_request(const network::ip_address_t this_ip_address) -> network::Packet;
}  // namespace estimation

#include "estimation.tpp"