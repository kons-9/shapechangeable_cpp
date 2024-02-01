// make google test
#include <gtest/gtest.h>

#define CFG_TEST_PUBLIC true

#include <expected>
#include <memory>
#include <variant>

#include "types.hpp"
#include "flit.hpp"

namespace network {
TEST(Flit, HeadFlit) {
    {
        HeadFlit flit(1, Header::None, 12, 3, 5);

        EXPECT_EQ(flit.get_type(), FlitType::Head);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.length, 1);
        EXPECT_EQ(flit.header, Header::None);
        EXPECT_EQ(flit.src, 3);
        EXPECT_EQ(flit.dst, 5);
        EXPECT_EQ(flit.packetid, 12);
#endif

        EXPECT_EQ(flit.validate(), NetworkError::OK);

        raw_data_t raw_data;
        flit.to_rawdata(raw_data);
        EXPECT_EQ(raw_data[0], flit.version);
        EXPECT_EQ(raw_data[1], static_cast<u_int8_t>(FlitType::Head));

        auto decoded = decoder(raw_data);
        EXPECT_TRUE(decoded.has_value());
        auto flit2 = std::move(decoded.value());
        EXPECT_EQ(flit2->get_type(), FlitType::Head);
        EXPECT_EQ(flit2->validate(), NetworkError::OK);
        raw_data_t raw_data2;
        flit2->to_rawdata(raw_data2);

        EXPECT_EQ(raw_data, raw_data2);

        auto headflit = static_pointer_cast<HeadFlit>(std::move(flit2));
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(headflit->version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(headflit->length, 1);
        EXPECT_EQ(headflit->header, Header::None);
        EXPECT_EQ(headflit->src, 3);
        EXPECT_EQ(headflit->dst, 5);
        EXPECT_EQ(headflit->packetid, 12);
#endif

        EXPECT_EQ(flit, *headflit);
    }
    {
        HeadFlit flit(0, Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), FlitType::Head);
        HeadFlit flit2(0, Header::None, 0, 0, 0);
        EXPECT_EQ(flit, flit2);
    }
    {
        HeadFlit flit(1, Header::None, 0, 0, 0);
        EXPECT_EQ(flit.get_type(), FlitType::Head);
        HeadFlit flit2(0, Header::None, 0, 0, 0);
        EXPECT_EQ(flit2.get_type(), FlitType::Head);
        EXPECT_NE(flit, flit2);
    }
}

TEST(Flit, BodyFlit) {
    {
        message_t data(CONFIG_MESSAGE_LENGTH, 1);

        BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), FlitType::Body);

        EXPECT_EQ(flit.validate(), NetworkError::OK);

        raw_data_t raw_data;
        flit.to_rawdata(raw_data);

        auto decoded = decoder(raw_data);
        EXPECT_TRUE(decoded.has_value());

        auto flit2 = std::move(decoded.value());

        EXPECT_EQ(flit2->validate(), NetworkError::OK);
        EXPECT_EQ(flit2->get_type(), FlitType::Body);

        raw_data_t raw_data2;
        flit2->to_rawdata(raw_data2);
        EXPECT_EQ(raw_data, raw_data2);

        auto bodyflit = static_pointer_cast<BodyFlit>(std::move(flit2));

        EXPECT_EQ(flit, *bodyflit);
    }
    {
        message_t data(CONFIG_MESSAGE_LENGTH);
        data[0] = 1;
        BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), FlitType::Body);

        data = message_t(CONFIG_MESSAGE_LENGTH);
        data[0] = 0;
        BodyFlit flit2(3, std::move(data));
        EXPECT_NE(flit, flit2);
    }
    {
        message_t data = message_t(CONFIG_MESSAGE_LENGTH);
        for (size_t i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i] = 1;
        }
        BodyFlit flit(3, std::move(data));
        EXPECT_EQ(flit.get_type(), FlitType::Body);
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(flit.id, 3);
        EXPECT_EQ(flit.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(flit.data[0], 1);
        EXPECT_EQ(flit.data[CONFIG_MESSAGE_LENGTH - 1], 1);
        EXPECT_EQ(flit.checksum, flit.culculate_checksum());
#endif

        message_t data2 = message_t(CONFIG_MESSAGE_LENGTH);
        for (size_t i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data2[i] = 1;
        }
        BodyFlit flit2(3, std::move(data2));
        EXPECT_EQ(flit, flit2);
    }
}

TEST(Flit, TailFlit) {
    message_t data(CONFIG_MESSAGE_LENGTH, 1);

    TailFlit flit(3, std::move(data));
    EXPECT_EQ(flit.get_type(), FlitType::Tail);
    EXPECT_EQ(flit.validate(), NetworkError::OK);

    raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = decoder(raw_data);
    EXPECT_TRUE(decoded.has_value());

    auto flit2 = std::move(decoded.value());

    EXPECT_EQ(flit2->validate(), NetworkError::OK);
    EXPECT_EQ(flit2->get_type(), FlitType::Tail);

    raw_data_t raw_data2;
    flit2->to_rawdata(raw_data2);
    EXPECT_EQ(raw_data, raw_data2);

    auto tailflit = static_pointer_cast<TailFlit>(std::move(flit2));

    EXPECT_EQ(flit, *tailflit);
}

TEST(Flit, NopeFlit) {
    NopeFlit flit = NopeFlit();

    EXPECT_EQ(flit.get_type(), FlitType::Nope);
    EXPECT_EQ(flit.validate(), NetworkError::OK);

    raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = decoder(raw_data);
    EXPECT_TRUE(decoded.has_value());
    auto flit2 = std::move(decoded.value());

    EXPECT_EQ(flit2->validate(), NetworkError::OK);
    EXPECT_EQ(flit2->get_type(), FlitType::Nope);

    auto nopeflit = static_pointer_cast<NopeFlit>(std::move(flit2));

    EXPECT_EQ(flit, *nopeflit);
}

TEST(Flit, UniquePtr) {
    {
        auto head = std::make_unique<HeadFlit>(1, Header::None, 12, 3, 5);

        message_t data(CONFIG_MESSAGE_LENGTH, 1);
        auto body = std::make_unique<BodyFlit>(3, std::move(data));

        data = message_t(CONFIG_MESSAGE_LENGTH, 1);
        auto tail = std::make_unique<TailFlit>(3, std::move(data));

        std::unique_ptr<Flit> head2 = std::move(head);

        std::vector<std::unique_ptr<Flit>> flits;
        flits.push_back(std::move(head2));
        flits.push_back(std::move(body));
        flits.push_back(std::move(tail));

        EXPECT_EQ(flits.at(0)->get_type(), FlitType::Head);
        EXPECT_EQ(flits.at(1)->get_type(), FlitType::Body);
        EXPECT_EQ(flits.at(2)->get_type(), FlitType::Tail);
    }
}

TEST(Packet, header) {
}

}  // namespace network
