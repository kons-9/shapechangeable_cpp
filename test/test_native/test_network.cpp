// make google test
#include <gtest/gtest.h>

#define CFG_TEST_PUBLIC true

#include <expected>
#include <memory>
#include <variant>

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
        EXPECT_EQ(raw_data[0], flit.version);
        EXPECT_EQ(raw_data[1], static_cast<u_int8_t>(flit::FlitType::Head));

        auto decoded = flit::decoder(raw_data);
        EXPECT_TRUE(decoded.has_value());
        auto flit2 = std::move(decoded.value());
        EXPECT_EQ(flit2->get_type(), flit::FlitType::Head);
        EXPECT_EQ(flit2->validate(), flit::FlitError::OK);
        flit::raw_data_t raw_data2;
        flit2->to_rawdata(raw_data2);

        EXPECT_EQ(raw_data, raw_data2);

        auto headflit = static_pointer_cast<flit::HeadFlit>(std::move(flit2));
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(headflit->version, flit::CONFIG_CURRENT_VERSION);
        EXPECT_EQ(headflit->length, 1);
        EXPECT_EQ(headflit->header, flit::Header::None);
        EXPECT_EQ(headflit->src, 3);
        EXPECT_EQ(headflit->dst, 5);
        EXPECT_EQ(headflit->packetid, 12);
#endif

        EXPECT_EQ(flit, *headflit);
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
        EXPECT_TRUE(decoded.has_value());

        auto flit2 = std::move(decoded.value());

        EXPECT_EQ(flit2->validate(), flit::FlitError::OK);
        EXPECT_EQ(flit2->get_type(), flit::FlitType::Body);

        flit::raw_data_t raw_data2;
        flit2->to_rawdata(raw_data2);
        EXPECT_EQ(raw_data, raw_data2);

        auto bodyflit = static_pointer_cast<flit::BodyFlit>(std::move(flit2));

        EXPECT_EQ(flit, *bodyflit);
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
    EXPECT_TRUE(decoded.has_value());

    auto flit2 = std::move(decoded.value());

    EXPECT_EQ(flit2->validate(), flit::FlitError::OK);
    EXPECT_EQ(flit2->get_type(), flit::FlitType::Tail);

    flit::raw_data_t raw_data2;
    flit2->to_rawdata(raw_data2);
    EXPECT_EQ(raw_data, raw_data2);

    auto tailflit = static_pointer_cast<flit::TailFlit>(std::move(flit2));

    EXPECT_EQ(flit, *tailflit);
}

TEST(Flit, NopeFlit) {
    flit::NopeFlit flit = flit::NopeFlit();

    EXPECT_EQ(flit.get_type(), flit::FlitType::Nope);
    EXPECT_EQ(flit.validate(), flit::FlitError::OK);

    flit::raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = flit::decoder(raw_data);
    EXPECT_TRUE(decoded.has_value());
    auto flit2 = std::move(decoded.value());

    EXPECT_EQ(flit2->validate(), flit::FlitError::OK);
    EXPECT_EQ(flit2->get_type(), flit::FlitType::Nope);

    auto nopeflit = static_pointer_cast<flit::NopeFlit>(std::move(flit2));

    EXPECT_EQ(flit, *nopeflit);
}

TEST(Flit, UniquePtr) {
    {
        auto head = std::make_unique<flit::HeadFlit>(1, flit::Header::None, 12, 3, 5);

        flit::message_t data(flit::CONFIG_MESSAGE_LENGTH, 1);
        auto body = std::make_unique<flit::BodyFlit>(3, std::move(data));

        data = flit::message_t(flit::CONFIG_MESSAGE_LENGTH, 1);
        auto tail = std::make_unique<flit::TailFlit>(3, std::move(data));

        std::unique_ptr<flit::Flit> head2 = std::move(head);

        std::vector<std::unique_ptr<flit::Flit>> flits;
        flits.push_back(std::move(head2));
        flits.push_back(std::move(body));
        flits.push_back(std::move(tail));

        EXPECT_EQ(flits.at(0)->get_type(), flit::FlitType::Head);
        EXPECT_EQ(flits.at(1)->get_type(), flit::FlitType::Body);
        EXPECT_EQ(flits.at(2)->get_type(), flit::FlitType::Tail);
    }
}

TEST(Packet, header) {
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}