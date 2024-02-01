#include <gtest/gtest.h>

#include <stub/stub.hpp>

// mockが正しく動作するかのテスト

TEST(Mock, serial_normal) {
    auto serial = test::SerialMock();
    network::raw_data_t data = {0x01, 0x02, 0x03};
    serial.send(data);
    ASSERT_EQ(serial.raw_data, data);
    ASSERT_EQ(serial.has_value, true);

    network::raw_data_t data2 = {};
    serial.receive(data2);
    ASSERT_EQ(data2, data);
    ASSERT_EQ(serial.has_value, false);
}

TEST(Mock, serial_collition) {
    auto serial = test::SerialMock();
    network::raw_data_t data = {0x01, 0x02, 0x03};
    serial.send(data);
    ASSERT_EQ(serial.raw_data, data);
    ASSERT_EQ(serial.has_value, true);
    ASSERT_EQ(serial.collision, false);

    network::raw_data_t data2 = {0x04, 0x05, 0x06};
    ASSERT_EQ(serial.send(data2), traits::SerialError::GenericError);
    ASSERT_NE(serial.raw_data, data);
    ASSERT_EQ(serial.has_value, true);
    ASSERT_EQ(serial.collision, true);

    network::raw_data_t data3 = {};
    serial.receive(data3);
    ASSERT_NE(data3, data);
    ASSERT_NE(data3, data2);
    // reset
    ASSERT_EQ(serial.has_value, false);
    ASSERT_EQ(serial.collision, false);
}

TEST(Mock, fs) {
    auto address = network::macaddress_t(0x010203040506);
    std::vector<uint16_t> image = {0x01, 0x02, 0x03};

    auto fs = test::FsMock(address, image);

    auto macaddress = fs.read_macaddress();
    ASSERT_EQ(macaddress, address);
    auto data = std::vector<uint16_t>(image.size());
    auto len = fs.read_image("test", data.data(), data.size());
    ASSERT_EQ(len, image.size());
    ASSERT_EQ(data, image);
}