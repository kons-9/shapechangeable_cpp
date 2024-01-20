// make google test
#define CFG_TEST_PUBLIC false

#include <gtest/gtest.h>

#include "config.hpp"
#include "flit.hpp"

TEST(Flit, HeadFlit) {
    {
        flit::Flit flit(0, flit::Header::None, 0, 0, 0);

        EXPECT_EQ(flit.get_type(), flit::FlitType::Head);
        EXPECT_EQ(flit.validate(), flit::FlitError::OK);

        flit::raw_data_t raw_data;
        flit.to_rawdata(raw_data);
        flit::Flit flit2 = flit::Flit(raw_data);
        EXPECT_EQ(flit2.validate(), flit::FlitError::OK);

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
        data[0] = 1;
        flit::Flit flit(3, std::move(data), false);
        EXPECT_EQ(flit.get_type(), flit::FlitType::Body);
        data[0] = 1;
        flit::Flit flit2(3, std::move(data), false);
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}