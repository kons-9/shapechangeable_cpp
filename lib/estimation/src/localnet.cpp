#include "estimation_types.hpp"
#include <_log.hpp>

namespace estimation {
int operator==(LocalLocation lhs, LocalLocation rhs) {
    return static_cast<std::underlying_type_t<LocalLocation>>(lhs)
           == static_cast<std::underlying_type_t<LocalLocation>>(rhs);
}
LocalLocation diagonal_location(LocalLocation location) {
    switch (location) {
    case LocalLocation::UpperLeft: return LocalLocation::LowerRight;
    case LocalLocation::UpperRight: return LocalLocation::LowerLeft;
    case LocalLocation::LowerLeft: return LocalLocation::UpperRight;
    case LocalLocation::LowerRight: return LocalLocation::UpperLeft;
    }

    LOGE("estimation", "diagonal_location");
    // assert(false);
    return LocalLocation::UpperLeft;
}

coordinate_t get_root_coordinate(LocalLocation location) {
    switch (location) {
    case LocalLocation::UpperLeft: return std::make_pair(0, 1);
    case LocalLocation::UpperRight: return std::make_pair(1, 1);
    case LocalLocation::LowerLeft: return std::make_pair(0, 0);
    case LocalLocation::LowerRight: return std::make_pair(1, 0);
    }

    LOGE("estimation", "get_root_coordinate");
    // assert(false);
    return std::make_pair(0, 0);
}
LocalLocation rotate_clockwise(LocalLocation location) {
    switch (location) {
    case LocalLocation::UpperLeft: return LocalLocation::UpperRight;
    case LocalLocation::UpperRight: return LocalLocation::LowerRight;
    case LocalLocation::LowerLeft: return LocalLocation::UpperLeft;
    case LocalLocation::LowerRight: return LocalLocation::LowerLeft;
    }
    LOGE("estimation", "rotate_clockwise");
    // assert(false);
    return LocalLocation::UpperLeft;
}
LocalLocation rotate_counterclockwise(LocalLocation location) {
    switch (location) {
    case LocalLocation::UpperLeft: return LocalLocation::LowerLeft;
    case LocalLocation::UpperRight: return LocalLocation::UpperLeft;
    case LocalLocation::LowerLeft: return LocalLocation::LowerRight;
    case LocalLocation::LowerRight: return LocalLocation::UpperRight;
    }
    LOGE("estimation", "rotate_counterclockwise");
    // assert(false);
    return LocalLocation::UpperLeft;
}
}