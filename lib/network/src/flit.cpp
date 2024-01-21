#include "flit.hpp"

namespace flit {
FlitError HeadFlit::validate() const {
    if (version != CONFIG_CURRENT_VERSION) {
        return FlitError::INVALID_VERSION;
    }
    if (length > CONFIG_MESSAGE_LENGTH) {
        return FlitError::INVALID_LENGTH;
    }
    checksum_t cur_checksum = culculate_checksum();
    if (checksum != cur_checksum) {
        return FlitError::INVALID_CHECKSUM;
    }
    return FlitError::OK;
}
checksum_t HeadFlit::culculate_checksum() const {
    checksum_t checksum = 0;
    checksum += length;
    checksum += static_cast<checksum_t>(header);
    checksum += src;
    checksum += dst;
    checksum += packetid;
    checksum += option;
    return checksum;
}
void HeadFlit::to_rawdata(raw_data_t &raw_data) const {
    raw_data[0] = static_cast<uint8_t>(version);
    raw_data[1] = static_cast<uint8_t>(FlitType::Head);
    // version:8:flittype:8:nodeid:16:nodeid:16:packetid:16:flitid:16:header:16:option:16:checksum:16
    raw_data[2] = src >> 8;
    raw_data[3] = src & 0xff;
    raw_data[4] = dst >> 8;
    raw_data[5] = dst & 0xff;
    raw_data[6] = packetid >> 8;
    raw_data[7] = packetid & 0xff;
    raw_data[8] = length >> 8;
    raw_data[9] = length & 0xff;
    raw_data[10] = static_cast<uint8_t>(header) >> 8;
    raw_data[11] = static_cast<uint8_t>(header) & 0xff;
    raw_data[12] = option >> 8;
    raw_data[13] = option & 0xff;
    raw_data[14] = checksum >> 8;
    raw_data[15] = checksum & 0xff;
};

FlitError BodyFlit::validate() const {
    if (version != CONFIG_CURRENT_VERSION) {
        return FlitError::INVALID_VERSION;
    }
    if (id > CONFIG_MESSAGE_LENGTH) {
        return FlitError::INVALID_LENGTH;
    }
    checksum_t cur_checksum = culculate_checksum();
    if (checksum != cur_checksum) {
        return FlitError::INVALID_CHECKSUM;
    }
    return FlitError::OK;
}
checksum_t BodyFlit::culculate_checksum() const {
    checksum_t checksum = 0;
    checksum += id;
    for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
        checksum += data[i];
    }
    return checksum;
}
void BodyFlit::to_rawdata(raw_data_t &raw_data) const {
    raw_data[0] = static_cast<uint8_t>(version);
    raw_data[1] = static_cast<uint8_t>(FlitType::Body);
    // version:8:flittype:8:flitid:16:message:80:checksum:16
    raw_data[2] = id >> 8;
    raw_data[3] = id & 0xff;
    for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
        raw_data[i + 4] = data[i];
    }
    raw_data[14] = checksum >> 8;
    raw_data[15] = checksum & 0xff;
};

FlitError TailFlit::validate() const {
    if (version != CONFIG_CURRENT_VERSION) {
        return FlitError::INVALID_VERSION;
    }
    if (id > CONFIG_MESSAGE_LENGTH) {
        return FlitError::INVALID_LENGTH;
    }
    checksum_t cur_checksum = culculate_checksum();
    if (checksum != cur_checksum) {
        return FlitError::INVALID_CHECKSUM;
    }
    return FlitError::OK;
}
checksum_t TailFlit::culculate_checksum() const {
    checksum_t checksum = 0;
    checksum += id;
    for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
        checksum += data[i];
    }
    return checksum;
}
void TailFlit::to_rawdata(raw_data_t &raw_data) const {
    raw_data[0] = static_cast<uint8_t>(version);
    raw_data[1] = static_cast<uint8_t>(FlitType::Tail);
    // version:8:flittype:8:flitid:16:message:80:checksum:16
    raw_data[2] = id >> 8;
    raw_data[3] = id & 0xff;
    for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
        raw_data[i + 4] = data[i];
    }
    raw_data[14] = checksum >> 8;
    raw_data[15] = checksum & 0xff;
};

std::variant<FlitError, HeadFlit, BodyFlit, TailFlit, NopeFlit> decoder(raw_data_t &raw_data) {
    auto version = raw_data[0];
    auto type = static_cast<FlitType>(raw_data[1]);
    auto checksum = raw_data[14] << 8 | raw_data[15];

    switch (type) {
    case FlitType::Head: {
        // version:8:flittype:8:nodeid:16:nodeid:16:packetid:16:flitid:16:header:16:option:16:checksum:16
        node_id_t src = raw_data[2] << 8 | raw_data[3];
        node_id_t dst = raw_data[4] << 8 | raw_data[5];
        packetid_t packetid = raw_data[6] << 8 | raw_data[7];
        flitid_t length = raw_data[8] << 8 | raw_data[9];
        Header header = static_cast<Header>(raw_data[10] << 8 | raw_data[11]);
        option_t option = raw_data[12] << 8 | raw_data[13];
        return HeadFlit(version, length, header, packetid, src, dst, option, checksum);
    }
    case FlitType::Body: {
        // version:8:flittype:8:flitid:16:message:80:checksum:16
        auto data = message_t(CONFIG_MESSAGE_LENGTH);
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i] = raw_data[i + 4];
        }
        flitid_t id = raw_data[2] << 8 | raw_data[3];
        return BodyFlit(version, id, std::move(data), checksum);
    }
    case FlitType::Tail: {
        auto data = message_t(CONFIG_MESSAGE_LENGTH);
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i] = raw_data[i + 4];
        }
        flitid_t id = raw_data[2] << 8 | raw_data[3];
        return TailFlit(version, id, std::move(data), checksum);
    }
    case FlitType::Nope: {
        return NopeFlit(version, checksum);
    }
    default: return FlitError::INVALID_TYPE;
    }
};

}  // namespace flit