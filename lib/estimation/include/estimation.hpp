#pragma once

#include <cstdint>
#include <utility>
#include <physical.hpp>

namespace estimation {
using coordinate_t = std::pair<uint16_t, uint16_t>;

coordinate_t init_coordinate(physical::Uart &uart);
coordinate_t update_coordinate(coordinate_t &coordinate);
}  // namespace estimation