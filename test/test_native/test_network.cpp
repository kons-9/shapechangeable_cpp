// make google test
#define CFG_TEST_PUBLIC true

#include <gtest/gtest.h>

#include "config.hpp"
#include "flit.hpp"

TEST(Flit, HeadFlit) {
    {
        flit::Flit flit(1, flit::Header::None, 12, 3, 5);

        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.type, flit::FlitType::Head);
        EXPECT_EQ(flit.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.head.length, 1);
        EXPECT_EQ(flit.head.header, flit::Header::None);
        EXPECT_EQ(flit.head.src, 3);
        EXPECT_EQ(flit.head.dst, 5);
        EXPECT_EQ(flit.head.packetid, 12);
#endif

        EXPECT_EQ(flit.validate(), flit::FlitError::OK);

        flit::raw_data_t raw_data;
        flit.to_rawdata(raw_data);
        flit::Flit flit2 = flit::Flit(raw_data);
        EXPECT_EQ(raw_data[0], flit.version);
        EXPECT_EQ(raw_data[1], static_cast<u_int8_t>(flit.type));

#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit2.type, flit::FlitType::Head);
        EXPECT_EQ(flit2.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit2.head.length, 1);
        EXPECT_EQ(flit2.head.header, flit::Header::None);
        EXPECT_EQ(flit2.head.src, 3);
        EXPECT_EQ(flit2.head.dst, 5);
        EXPECT_EQ(flit2.head.packetid, 12);
#endif

        EXPECT_EQ(flit, flit2);
    }
    {
        flit::Flit flit(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
        flit::Flit flit2(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit, flit2);
    }
    {
        flit::Flit flit(1, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
        flit::Flit flit2(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit2.get_type(), flit::FlitType::Head);
        EXPECT_NE(flit, flit2);
    }
}

TEST(Flit, BodyFlit) {
    {
        flit::message_t data;

        flit::Flit flit(3, std::move(data), false);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);

        EXPECT_EQ(flit.validate(), flit::FlitError::OK);

        flit::raw_data_t raw_data;
        flit.to_rawdata(raw_data);
        flit::Flit flit2 = flit::Flit(raw_data);
        EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

        EXPECT_EQ(flit, flit2);
    }
    {
        flit::message_t data;
        data[0] = 1;
        flit::Flit flit(3, std::move(data), false);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);
        data[0] = 0;
        flit::Flit flit2(3, std::move(data), false);
        EXPECT_NE(flit, flit2);
    }
    {
        flit::message_t data;
        for (size_t i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i] = 1;
        }
        flit::Flit flit(3, std::move(data), false);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.type, flit::FlitType::Body);
        EXPECT_EQ(flit.body.id, 3);
        EXPECT_EQ(flit.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.body.data[0], 1);
        EXPECT_EQ(flit.body.data[CONFIG_MESSAGE_LENGTH - 1], 1);
        EXPECT_EQ(flit.checksum, flit.culculate_checksum());
#endif

        flit::message_t data2;
        for (size_t i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data2[i] = 1;
        }
        flit::Flit flit2(3, std::move(data2), false);
        EXPECT_EQ(flit, flit2);
    }
}

TEST(Flit, TailFlit) {
    flit::message_t data;

    flit::Flit flit(3, std::move(data), true);
    EXPECT_EQ(flit.get_type(), flit::FlitType::Tail);
    EXPECT_EQ(flit.validate(), flit::FlitError::OK);

    flit::raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    flit::Flit flit2 = flit::Flit(raw_data);
    EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

    EXPECT_EQ(flit, flit2);
}

TEST(Flit, NopeFlit) {
    flit::Flit flit = flit::Flit();

    EXPECT_EQ(flit.get_type(), flit::FlitType::Nope);
    EXPECT_EQ(flit.validate(), flit::FlitError::OK);

    flit::raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    flit::Flit flit2 = flit::Flit(raw_data);
    EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

    EXPECT_EQ(flit, flit2);
}

TEST(Packet, general) {
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}