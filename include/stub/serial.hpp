#pragma once
#include <concepts.hpp>

namespace test {
class SerialMock {
  public:
    bool has_value = false;
    network::raw_data_t raw_data;
    bool collision = false;
    void reset() {
        raw_data = {};
        has_value = false;
        collision = false;
    }
    // for serial trait
    traits::SerialError send(const network::raw_data_t &data) {
        if (has_value == true) {
            // collision
            collision = true;
            for (int i = 0; i < raw_data.size(); i++) {
                raw_data[i] = raw_data[i] | data[i];
            }
            return traits::SerialError::GenericError;
        }
        raw_data = data;
        has_value = true;
        return traits::SerialError::Ok;
    }
    traits::SerialError receive(network::raw_data_t &data) {
        if (has_value == false || collision == true) {
            reset();
            return traits::SerialError::GenericError;
        } else {
            data = raw_data;
            reset();
            return traits::SerialError::Ok;
        }
    }
};

static_assert(traits::serial<SerialMock>);
}  // namespace test