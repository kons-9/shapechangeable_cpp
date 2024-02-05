#pragma once
#include <concepts.hpp>
#include <types.hpp>
#include <flit.hpp>

namespace test {
class SerialMock {
  public:
    //  should be queue
    std::vector<network::raw_data_t> raw_data;
    void reset() {
        raw_data.clear();
    }
    // for serial trait
    traits::SerialError send(const network::raw_data_t &data) {
        raw_data.push_back(data);
        return traits::SerialError::Ok;
    }
    traits::SerialError receive(network::raw_data_t &data) {
        if (raw_data.empty()) {
            return traits::SerialError::GenericError;
        }
        data = raw_data.front();
        raw_data.erase(raw_data.begin());
        if (raw_data.empty()) {
            reset();
        }
        return traits::SerialError::Ok;
    }
    traits::SerialError send(const network::Flit &flit) {
        network::raw_data_t raw_data;
        std::visit([&raw_data](auto &&arg) { arg.to_rawdata(raw_data); }, flit);
        return send(raw_data);
    }
    traits::SerialError receive(network::Flit &flit, uint8_t channel = 0) {
        network::raw_data_t raw_data;
        if (receive(raw_data) != traits::SerialError::Ok) {
            return traits::SerialError::GenericError;
        }
        flit = network::decoder(raw_data).value();
        return traits::SerialError::Ok;
    }
    // for test
};

static_assert(traits::serial<SerialMock>);
}  // namespace test