#include "physical.hpp"

using namespace physical::local;

static const char *TAG = "location";

static constexpr bool USE_EFUSE = true;

uint32_t physical::local::get_mac_address(void) {
#ifdef SHAPE_DEBUG
    return 0;
#endif

    uint32_t mac = 0;
    if constexpr (USE_EFUSE) {
        // do something
    } else {
        // currently not supported
        ESP_LOGE(TAG, "not EFUSE is not supported");
    }
    return mac;
}

bool physical::local::is_root(void) {
#ifdef SHAPE_DEBUG
    return true;
#endif
    bool is_root = false;

    if constexpr (USE_EFUSE) {
        // do something
    } else {
        // currently not supported
        ESP_LOGE(TAG, "not EFUSE is not supported");
    }
    return is_root;
}

physical::local::local_location_t physical::local::get_location(void) {
#ifdef SHAPE_DEBUG
    return local_location_t::UpperRight;
#endif
    local_location_t location = local_location_t::UpperRight;
    if constexpr (USE_EFUSE) {
        // do something
    } else {
        // currently not supported
        ESP_LOGE(TAG, "not EFUSE is not supported");
    }
    return location;
}

uint32_t physical::local::get_ip_address(void) {
#ifdef SHAPE_DEBUG
    return 0;
#endif
    uint32_t ip = 0;
    if constexpr (USE_EFUSE) {
        // do something
    } else {
        // currently not supported
        ESP_LOGE(TAG, "not EFUSE is not supported");
    }
    return ip;
}