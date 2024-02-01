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

template <traits::serial T>
auto get_head_flit(network::raw_data_t &receive_raw_data, T &uart) -> std::optional<network::Packet>;

template <traits::serial T>
auto make_packet(network::Packet &packet, T &uart) -> network::NetworkError;

// test done
bool is_finished(network::ip_address_t this_id,
                 const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates);
auto get_coordinate(network::ip_address_t this_id,
                    const std::vector<std::pair<uint32_t, coordinate_t>> &confirmed_coordinates) -> coordinate_t;

bool is_any_one_distance_neighbor(const std::vector<std::pair<network::macaddress_t, coordinate_t>> &coordinates);

auto process_data(const network::ip_address_t src,
                  const network::ip_address_t this_ip_address,
                  const std::vector<uint8_t> &data,
                  std::vector<std::pair<network::macaddress_t, coordinate_t>> &confirmed_coordinates)
    -> network::NetworkError;

}  // namespace estimation

#include "estimation.tpp"