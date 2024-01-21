// make google test
#define CFG_TEST_PUBLIC true

#include <gtest/gtest.h>

#include "config.hpp"
#include "flit.hpp"

TEST(Flit, HeadFlit) {
    {
        flit::HeadFlit flit(1, flit::Header::None, 12, 3, 5);

        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.version, flit::CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.length, 1);
        EXPECT_EQ(flit.header, flit::Header::None);
        EXPECT_EQ(flit.src, 3);
        EXPECT_EQ(flit.dst, 5);
        EXPECT_EQ(flit.packetid, 12);
#endif

        EXPECT_EQ(flit.validate(), flit::FlitError::OK);

        flit::raw_data_t raw_data;
        flit.to_rawdata(raw_data);
        auto decoded = flit::decoder(raw_data);
        EXPECT_TRUE(std::holds_alternative<flit::HeadFlit>(decoded));
        auto flit2 = std::get<flit::HeadFlit>(decoded);
        EXPECT_EQ(raw_data[0], flit.version);
        EXPECT_EQ(raw_data[1], static_cast<u_int8_t>(flit::FlitType::Head));

#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit2.version, flit::CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit2.length, 1);
        EXPECT_EQ(flit2.header, flit::Header::None);
        EXPECT_EQ(flit2.src, 3);
        EXPECT_EQ(flit2.dst, 5);
        EXPECT_EQ(flit2.packetid, 12);
#endif

        EXPECT_EQ(flit, flit2);
    }
    {
        flit::HeadFlit flit(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
        flit::HeadFlit flit2(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit, flit2);
    }
    {
        flit::HeadFlit flit(1, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
        flit::HeadFlit flit2(0, flit::Header::None, 0, 0, 0);
        EXPECT_EQ(flit2.get_type(), flit::FlitType::Head);
        EXPECT_NE(flit, flit2);
    }
}

TEST(Flit, BodyFlit) {
    {
        flit::message_t data(flit::CONFIG_MESSAGE_LENGTH, 1);

        flit::BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);

        EXPECT_EQ(flit.validate(), flit::FlitError::OK);

        flit::raw_data_t raw_data;
        flit.to_rawdata(raw_data);

        auto decoded = flit::decoder(raw_data);
        EXPECT_TRUE(std::holds_alternative<flit::BodyFlit>(decoded));
        auto flit2 = std::get<flit::BodyFlit>(decoded);
        EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

        EXPECT_EQ(flit, flit2);
    }
    {
        flit::message_t data(flit::CONFIG_MESSAGE_LENGTH);
        data[0] = 1;
        flit::BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);

        data = flit::message_t(flit::CONFIG_MESSAGE_LENGTH);
        data[0] = 0;
        flit::BodyFlit flit2(3, std::move(data));
        EXPECT_NE(flit, flit2);
    }
    {
        flit::message_t data = flit::message_t(flit::CONFIG_MESSAGE_LENGTH);
        for (size_t i = 0; i < flit::CONFIG_MESSAGE_LENGTH; i++) {
            data[i] = 1;
        }
        flit::BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.id, 3);
        EXPECT_EQ(flit.version, flit::CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.data[0], 1);
        EXPECT_EQ(flit.data[flit::CONFIG_MESSAGE_LENGTH - 1], 1);
        EXPECT_EQ(flit.checksum, flit.culculate_checksum());
#endif

        flit::message_t data2 = flit::message_t(flit::CONFIG_MESSAGE_LENGTH);
        for (size_t i = 0; i < flit::CONFIG_MESSAGE_LENGTH; i++) {
            data2[i] = 1;
        }
        flit::BodyFlit flit2(3, std::move(data2));
        EXPECT_EQ(flit, flit2);
    }
}

TEST(Flit, TailFlit) {
    flit::message_t data(flit::CONFIG_MESSAGE_LENGTH, 1);

    flit::TailFlit flit(3, std::move(data));
    EXPECT_EQ(flit.get_type(), flit::FlitType::Tail);
    EXPECT_EQ(flit.validate(), flit::FlitError::OK);

    flit::raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = flit::decoder(raw_data);
    EXPECT_TRUE(std::holds_alternative<flit::TailFlit>(decoded));
    auto flit2 = std::get<flit::TailFlit>(decoded);
    EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

    EXPECT_EQ(flit, flit2);
}

TEST(Flit, NopeFlit) {
    flit::NopeFlit flit = flit::NopeFlit();

    EXPECT_EQ(flit.get_type(), flit::FlitType::Nope);
    EXPECT_EQ(flit.validate(), flit::FlitError::OK);

    flit::raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = flit::decoder(raw_data);
    EXPECT_TRUE(std::holds_alternative<flit::NopeFlit>(decoded));
    auto flit2 = std::get<flit::NopeFlit>(decoded);
    EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

    EXPECT_EQ(flit, flit2);
}

TEST(Packet, header) {
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}