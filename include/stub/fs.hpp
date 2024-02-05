#pragma once
#include <concepts.hpp>
#include "_log.hpp"

#include <vector>

namespace test {
class FsMock {
  public:
    FsMock(network::macaddress_t address, std::vector<uint16_t> image)
        : mac_address(address)
        , image(image) {
    }
    network::macaddress_t mac_address;
    std::vector<uint16_t> image;
    void set_macaddress(network::macaddress_t address) {
        mac_address = address;
    }
    void set_image(uint16_t *data, int len) {
        image = std::vector<uint16_t>(data, data + len);
    }

    // for file_system trait
    auto read_macaddress(void) -> std::expected<network::macaddress_t, traits::FsError> {
        return mac_address;
    }
    std::size_t read_image(const char *path, uint16_t *data, std::size_t len) {
        if (len < image.size()) {
            LOGE("read_image", "read_image: buffer size is too small");
            return 0;
        }
        std::copy(image.begin(), image.end(), data);
        return image.size();
    }
};
static_assert(traits::file_system<FsMock>);
}  // namespace test