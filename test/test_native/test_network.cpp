// make google test
#include <gtest/gtest.h>

#define CFG_TEST_PUBLIC true

#include <expected>
#include <memory>
#include <variant>

#include "types.hpp"
#include "flit.hpp"
#include "packet.hpp"

namespace network {
TEST(BaseFlit, HeadFlit) {
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
#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(raw_data[0], flit.version);
#endif
        EXPECT_EQ(raw_data[1], static_cast<u_int8_t>(FlitType::Head));

        auto decoded = decoder(raw_data);
        EXPECT_TRUE(decoded.has_value());
        auto flit2 = std::move(decoded.value());
        EXPECT_TRUE(std::holds_alternative<HeadFlit>(flit2));
        auto headflit = std::get<HeadFlit>(std::move(flit2));
        EXPECT_EQ(headflit.validate(), NetworkError::OK);
        raw_data_t raw_data2;
        headflit.to_rawdata(raw_data2);

        EXPECT_EQ(raw_data, raw_data2);

#if CFG_TEST_PUBLIC == true
        EXPECT_EQ(headflit.version, CONFIG_CURRENT_VERSION);
        EXPECT_EQ(headflit.length, 1);
        EXPECT_EQ(headflit.header, Header::None);
        EXPECT_EQ(headflit.src, 3);
        EXPECT_EQ(headflit.dst, 5);
        EXPECT_EQ(headflit.packetid, 12);
#endif

        EXPECT_EQ(flit, headflit);
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

TEST(BaseFlit, BodyFlit) {
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
        auto bodyflit = std::get<BodyFlit>(std::move(flit2));

        EXPECT_EQ(bodyflit.validate(), NetworkError::OK);

        raw_data_t raw_data2;
        bodyflit.to_rawdata(raw_data2);
        EXPECT_EQ(raw_data, raw_data2);

        EXPECT_EQ(flit, bodyflit);
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

TEST(BaseFlit, TailFlit) {
    message_t data(CONFIG_MESSAGE_LENGTH, 1);

    TailFlit flit(3, std::move(data));
    EXPECT_EQ(flit.get_type(), FlitType::Tail);
    EXPECT_EQ(flit.validate(), NetworkError::OK);

    raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = decoder(raw_data);
    EXPECT_TRUE(decoded.has_value());

    auto flit2 = std::move(decoded.value());
    EXPECT_TRUE(std::holds_alternative<TailFlit>(flit2));
    auto tailflit = std::get<TailFlit>(std::move(flit2));
    EXPECT_EQ(tailflit.validate(), NetworkError::OK);

    raw_data_t raw_data2;
    tailflit.to_rawdata(raw_data2);
    EXPECT_EQ(raw_data, raw_data2);


    EXPECT_EQ(flit, tailflit);
}

TEST(BaseFlit, NopeFlit) {
    NopeFlit flit = NopeFlit();

    EXPECT_EQ(flit.get_type(), FlitType::Nope);
    EXPECT_EQ(flit.validate(), NetworkError::OK);

    raw_data_t raw_data;
    flit.to_rawdata(raw_data);
    auto decoded = decoder(raw_data);
    EXPECT_TRUE(decoded.has_value());
    auto flit2 = std::move(decoded.value());
    auto nopeflit = std::get<NopeFlit>(std::move(flit2));
    EXPECT_EQ(nopeflit.validate(), NetworkError::OK);

    EXPECT_EQ(flit, nopeflit);
}

TEST(BaseFlit, UniquePtr) {
    // 今の実装だと使わない
    {
        auto head = std::make_unique<HeadFlit>(1, Header::None, 12, 3, 5);

        message_t data(CONFIG_MESSAGE_LENGTH, 1);
        auto body = std::make_unique<BodyFlit>(3, std::move(data));

        data = message_t(CONFIG_MESSAGE_LENGTH, 1);
        auto tail = std::make_unique<TailFlit>(3, std::move(data));

        std::unique_ptr<BaseFlit> head2 = std::move(head);

        std::vector<std::unique_ptr<BaseFlit>> flits;
        flits.push_back(std::move(head2));
        flits.push_back(std::move(body));
        flits.push_back(std::move(tail));

        EXPECT_EQ(flits.at(0)->get_type(), FlitType::Head);
        EXPECT_EQ(flits.at(1)->get_type(), FlitType::Body);
        EXPECT_EQ(flits.at(2)->get_type(), FlitType::Tail);
    }
}

TEST(Packet, load_flit) {
    auto this_ip = 1;
    {
        // only header
        auto src = 3;
        auto dst = this_ip;
        auto packet_id = 12;
        auto length = 1;
        HeadFlit head(length, Header::None, packet_id, src, dst);
        Packet packet;
        auto err = packet.load_flit(this_ip, std::move(head));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet.get_header(), Header::None);
        ASSERT_EQ(packet.get_packet_id(), packet_id);
        ASSERT_EQ(packet.get_flit_length(), length);
        ASSERT_TRUE(packet.is_receive_finished());
    }
    {
        // invalid dst
        auto src = 3;
        auto dst = this_ip + 1;
        auto packet_id = 12;
        auto length = 1;
        HeadFlit head(length, Header::None, packet_id, src, dst);
        Packet packet;
        auto err = packet.load_flit(this_ip, std::move(head));
        ASSERT_EQ(err, NetworkError::INVALID_DESTINATION);
    }
    {
        // coordinate estimation request(only header)
        auto src = 3;
        auto dst = this_ip;
        auto packet_id = 12;
        auto length = 1;
        HeadFlit head(length, Header::COORDINATE_ESTIMATION, packet_id, src, dst);
        Packet packet;
        auto err = packet.load_flit(this_ip, std::move(head));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet.get_header(), Header::COORDINATE_ESTIMATION);
        ASSERT_EQ(packet.get_packet_id(), packet_id);
        ASSERT_EQ(packet.get_flit_length(), length);
        ASSERT_TRUE(packet.is_receive_finished());
    }
    {
        // coordinate estimation response
        uint16_t src = 3;
        auto dst = this_ip;
        auto packet_id = 12;
        auto data = Packet(Header::COORDINATE_ESTIMATION_RSP, packet_id, src, BROADCAST_ADDRESS).get_header_raw_data();
        // confirmed (1,1)
        data.push_back(0xFF);
        data.push_back(src >> 8u);
        data.push_back(src & 0xFFu);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0xFF);
        data.insert(data.end(), CONFIG_MESSAGE_LENGTH - data.size() % CONFIG_MESSAGE_LENGTH, 0);
        ASSERT_EQ(data.size() % CONFIG_MESSAGE_LENGTH, 0);  // 3 flits
                                                            //
        auto length = data.size() / CONFIG_MESSAGE_LENGTH;

        HeadFlit head(length, Header::COORDINATE_ESTIMATION_RSP, packet_id, src, dst);
        Packet packet;
        auto err = packet.load_flit(this_ip, std::move(head));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet.get_header(), Header::COORDINATE_ESTIMATION_RSP);
        ASSERT_EQ(packet.get_packet_id(), packet_id);
        ASSERT_EQ(packet.get_flit_length(), length);
        ASSERT_FALSE(packet.is_receive_finished());
        for (size_t i = 0; i < data.size() / CONFIG_MESSAGE_LENGTH - 1; i++) {
            message_t message(CONFIG_MESSAGE_LENGTH);
            for (size_t j = 0; j < CONFIG_MESSAGE_LENGTH; j++) {
                message[j] = data[i * CONFIG_MESSAGE_LENGTH + j];
            }
            auto err = packet.load_flit(this_ip, BodyFlit(i + 1, std::move(message)));
            ASSERT_EQ(err, NetworkError::OK);
#ifdef CFG_TEST_PUBLIC
            ASSERT_EQ(packet.current_flit_index, i + 1);
#endif
            ASSERT_FALSE(packet.is_receive_finished());
        }
        message_t message(CONFIG_MESSAGE_LENGTH);
        std::copy(data.end() - CONFIG_MESSAGE_LENGTH, data.end(), message.begin());
        err = packet.load_flit(this_ip, TailFlit(data.size() / CONFIG_MESSAGE_LENGTH, std::move(message)));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_TRUE(packet.is_receive_finished());
        auto data2 = packet.get_data();
        ASSERT_EQ(data2.size(), 7);
        ASSERT_EQ(data2[0], 0xFF);
        ASSERT_EQ(data2[1], src >> 8u);
        ASSERT_EQ(data2[2], src & 0xFFu);
        ASSERT_EQ(data2[3], 0);
        ASSERT_EQ(data2[4], 1);
        ASSERT_EQ(data2[5], 0);
        ASSERT_EQ(data2[6], 1);
    }
    {
        // coordinate estimation response
        uint16_t src = 3;
        auto dst = this_ip;
        auto packet_id = 12;
        auto data = Packet(Header::COORDINATE_ESTIMATION_RSP, packet_id, src, BROADCAST_ADDRESS).get_header_raw_data();
        // confirmed (1,1)
        data.push_back(0xFF);
        data.push_back(src >> 8u);
        data.push_back(src & 0xFFu);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(1);
        data.push_back(src >> 8u);
        data.push_back(src & 0xFFu);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(1);
        data.push_back(src >> 8u);
        data.push_back(src & 0xFFu);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(1);
        data.push_back(src >> 8u);
        data.push_back(src & 0xFFu);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0xFF);
        data.insert(data.end(), CONFIG_MESSAGE_LENGTH - data.size() % CONFIG_MESSAGE_LENGTH, 0);
        ASSERT_EQ(data.size() % CONFIG_MESSAGE_LENGTH, 0);  // 3 flits
                                                            //
        auto length = data.size() / CONFIG_MESSAGE_LENGTH;

        HeadFlit head(length, Header::COORDINATE_ESTIMATION_RSP, packet_id, src, dst);
        Packet packet;
        auto err = packet.load_flit(this_ip, std::move(head));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet.get_header(), Header::COORDINATE_ESTIMATION_RSP);
        ASSERT_EQ(packet.get_packet_id(), packet_id);
        ASSERT_EQ(packet.get_flit_length(), length);
        ASSERT_FALSE(packet.is_receive_finished());
        for (size_t i = 0; i < data.size() / CONFIG_MESSAGE_LENGTH - 1; i++) {
            message_t message(CONFIG_MESSAGE_LENGTH);
            for (size_t j = 0; j < CONFIG_MESSAGE_LENGTH; j++) {
                message[j] = data[i * CONFIG_MESSAGE_LENGTH + j];
            }
            auto err = packet.load_flit(this_ip, BodyFlit(i + 1, std::move(message)));
            ASSERT_EQ(err, NetworkError::OK);
            ASSERT_FALSE(packet.is_receive_finished());
        }
        message_t message(CONFIG_MESSAGE_LENGTH);
        std::copy(data.end() - CONFIG_MESSAGE_LENGTH, data.end(), message.begin());
        err = packet.load_flit(this_ip, TailFlit(data.size() / CONFIG_MESSAGE_LENGTH, std::move(message)));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_TRUE(packet.is_receive_finished());
        auto data2 = packet.get_data();
        ASSERT_EQ(data2.size(), 25);
        ASSERT_EQ(data2[0], 0xFF);
        ASSERT_EQ(data2[1], src >> 8u);
        ASSERT_EQ(data2[2], src & 0xFFu);
        ASSERT_EQ(data2[3], 0);
        ASSERT_EQ(data2[4], 1);
        ASSERT_EQ(data2[5], 0);
        ASSERT_EQ(data2[6], 1);
        ASSERT_EQ(data2[7], src >> 8u);
        ASSERT_EQ(data2[8], src & 0xFFu);
        ASSERT_EQ(data2[9], 0);
        ASSERT_EQ(data2[10], 1);
        ASSERT_EQ(data2[11], 0);
        ASSERT_EQ(data2[12], 1);
        ASSERT_EQ(data2[13], src >> 8u);
        ASSERT_EQ(data2[14], src & 0xFFu);
        ASSERT_EQ(data2[15], 0);
        ASSERT_EQ(data2[16], 1);
        ASSERT_EQ(data2[17], 0);
        ASSERT_EQ(data2[18], 1);
        ASSERT_EQ(data2[19], src >> 8u);
        ASSERT_EQ(data2[20], src & 0xFFu);
        ASSERT_EQ(data2[21], 0);
        ASSERT_EQ(data2[22], 1);
        ASSERT_EQ(data2[23], 0);
        ASSERT_EQ(data2[24], 1);
    }
}

TEST(Packet, to_flit) {
    auto this_ip = 1;
    {
        // only header
        auto src = this_ip;
        auto dst = 3;
        auto packet_id = 12;
        auto length = 1;
        Packet packet(Header::Data, packet_id, src, dst);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<HeadFlit>(exp_flit.value()));
        auto flit = std::get<HeadFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);
        ASSERT_EQ(flit.get_header(), Header::Data);
        ASSERT_EQ(flit.get_packet_id(), packet_id);
        ASSERT_EQ(flit.get_src(), src);
        ASSERT_EQ(flit.get_dst(), BROADCAST_ADDRESS);
        ASSERT_EQ(flit.get_length(), length);
    }
    {
        Packet packet(Header::None, 12, this_ip, BROADCAST_ADDRESS);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<NopeFlit>(exp_flit.value()));
        auto flit = std::get<NopeFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);
    }
    {
        Packet packet(Header::COORDINATE_ESTIMATION, 12, this_ip, BROADCAST_ADDRESS);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<HeadFlit>(exp_flit.value()));
        auto flit = std::get<HeadFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);
        ASSERT_EQ(flit.get_header(), Header::COORDINATE_ESTIMATION);
        ASSERT_EQ(flit.get_packet_id(), 12);
        ASSERT_EQ(flit.get_src(), this_ip);
        ASSERT_EQ(flit.get_dst(), BROADCAST_ADDRESS);
        ASSERT_EQ(flit.get_length(), 1);
    }
    {
        auto data = message_t{
            0xFF,  // confirmed
            0x00,
            0x01,  // src
            0x00,
            0x01,  // x
            0x00,
            0x01,  // y
        };
        auto t_data = data;
        Packet packet(Header::COORDINATE_ESTIMATION_RSP, 12, this_ip, BROADCAST_ADDRESS, std::move(t_data));
        auto header_raw_data = packet.get_header_raw_data();
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<HeadFlit>(exp_flit.value()));
        auto headflit = std::get<HeadFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(headflit.validate(), NetworkError::OK);
        ASSERT_EQ(headflit.get_header(), Header::COORDINATE_ESTIMATION_RSP);
        ASSERT_EQ(headflit.get_packet_id(), 12);
        ASSERT_EQ(headflit.get_src(), this_ip);
        ASSERT_EQ(headflit.get_dst(), BROADCAST_ADDRESS);
        ASSERT_EQ(headflit.get_length(), 3);

        exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<BodyFlit>(exp_flit.value()));
        auto bodyflit = std::get<BodyFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(bodyflit.validate(), NetworkError::OK);
        ASSERT_EQ(bodyflit.get_id(), 1);
        auto data2 = bodyflit.get_data();

        exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<BodyFlit>(exp_flit.value()));
        bodyflit = std::get<BodyFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(bodyflit.validate(), NetworkError::OK);
        ASSERT_EQ(bodyflit.get_id(), 2);
        auto data3 = bodyflit.get_data();

        exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<TailFlit>(exp_flit.value()));
        auto tailflit = std::get<TailFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(tailflit.validate(), NetworkError::OK);
        ASSERT_EQ(tailflit.get_id(), 3);
        auto data4 = tailflit.get_data();
        // header_raw_data + data + eof + padding= data2 + data3 + data4
        header_raw_data.insert(header_raw_data.end(), data.begin(), data.end());
        header_raw_data.insert(header_raw_data.end(), 0xFF);
        header_raw_data.insert(header_raw_data.end(),
                               CONFIG_MESSAGE_LENGTH - header_raw_data.size() % CONFIG_MESSAGE_LENGTH,
                               0);
        data2.insert(data2.end(), data3.begin(), data3.end());
        data2.insert(data2.end(), data4.begin(), data4.end());

        ASSERT_EQ(header_raw_data, data2);
    }
}
TEST(Packet, from_to_flit) {
    auto this_ip = 1;
    {
        // only header
        auto src = this_ip;
        auto dst = 3;
        auto packet_id = 12;
        auto length = 1;
        Packet packet(Header::Data, packet_id, src, dst);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<HeadFlit>(exp_flit.value()));
        auto flit = std::get<HeadFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);
        ASSERT_EQ(flit.get_header(), Header::Data);
        ASSERT_EQ(flit.get_packet_id(), packet_id);
        ASSERT_EQ(flit.get_src(), src);
        ASSERT_EQ(flit.get_dst(), BROADCAST_ADDRESS);
        ASSERT_EQ(flit.get_length(), length);

        Packet packet2;
        auto err = packet2.load_flit(this_ip, std::move(flit));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet2.get_header(), Header::Data);
        ASSERT_EQ(packet2.get_packet_id(), packet_id);
        ASSERT_EQ(packet2.get_flit_length(), length);
        ASSERT_FALSE(packet2.is_receive_finished());
    }
    {
        Packet packet(Header::None, 12, this_ip, BROADCAST_ADDRESS);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<NopeFlit>(exp_flit.value()));
        auto flit = std::get<NopeFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);

        Packet packet2;
        auto err = packet2.load_flit(this_ip, std::move(flit));
        ASSERT_EQ(err, NetworkError::INVALID_NOPE);
    }
    {
        Packet packet(Header::COORDINATE_ESTIMATION, 12, this_ip, BROADCAST_ADDRESS);
        auto exp_flit = packet.to_flit(this_ip, DefaultRouting());
        ASSERT_TRUE(exp_flit.has_value());
        ASSERT_TRUE(std::holds_alternative<HeadFlit>(exp_flit.value()));
        auto flit = std::get<HeadFlit>(std::move(exp_flit.value()));
        ASSERT_EQ(flit.validate(), NetworkError::OK);
        ASSERT_EQ(flit.get_header(), Header::COORDINATE_ESTIMATION);
        ASSERT_EQ(flit.get_packet_id(), 12);
        ASSERT_EQ(flit.get_src(), this_ip);
        ASSERT_EQ(flit.get_dst(), BROADCAST_ADDRESS);
        ASSERT_EQ(flit.get_length(), 1);

        Packet packet2;
        auto err = packet2.load_flit(this_ip, std::move(flit));
        ASSERT_EQ(err, NetworkError::OK);
        ASSERT_EQ(packet2.get_header(), Header::COORDINATE_ESTIMATION);
        ASSERT_EQ(packet2.get_packet_id(), 12);
        ASSERT_EQ(packet2.get_flit_length(), 1);
#ifdef CFG_TEST_PUBLIC
        ASSERT_EQ(packet2.current_flit_index, 1);
#endif
        ASSERT_TRUE(packet2.is_receive_finished());
    }
}

}  // namespace network
