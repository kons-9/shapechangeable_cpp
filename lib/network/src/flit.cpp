#include "flit.hpp"

namespace flit {
FlitError Flit::validate() {
    if (version != CONFIG_CURRENT_VERSION) {
        return FlitError::INVALID_VERSION;
    }
    switch (type) {
    case FlitType::Head:
        if (head.length > CONFIG_MESSAGE_LENGTH) {
            // unreachable
            return FlitError::INVALID_LENGTH;
        }
        break;
    case FlitType::Body: break;
    case FlitType::Tail: break;
    case FlitType::Nope: break;
    default: return FlitError::INVALID_TYPE;
    }

    checksum_t cur_checksum = culculate_checksum();
    if (checksum != cur_checksum) {
        return FlitError::INVALID_CHECKSUM;
    }

    return FlitError::OK;
}
checksum_t Flit::culculate_checksum() {
    checksum_t checksum = 0;
    switch (type) {
    case FlitType::Head:
        checksum += head.length;
        checksum += static_cast<checksum_t>(head.header);
        checksum += head.src;
        checksum += head.dst;
        checksum += head.packetid;
        checksum += head.option;
        break;
    case FlitType::Body:
        checksum += body.id;
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            checksum += body.data[i];
        }
        break;
    case FlitType::Tail:
        checksum += tail.id;
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            checksum += tail.data[i];
        }
        break;
    default: break;
    }
    return checksum;
}
void Flit::to_rawdata(raw_data_t &raw_data) {
    raw_data[0] = (static_cast<uint8_t>(type) & 0x0f) | (version << 4);
    switch (type) {
    case FlitType::Head:
        //     version:4:flittype:4:flitid:32:header:8:nodeid:16:nodeid:16:packetid:16:option:16:checksum:16
        raw_data[1] = head.length >> 24;
        raw_data[2] = head.length >> 16;
        raw_data[3] = head.length >> 8;
        raw_data[4] = head.length;
        raw_data[5] = static_cast<uint8_t>(head.header);
        raw_data[6] = head.src >> 8;
        raw_data[7] = head.src;
        raw_data[8] = head.dst >> 8;
        raw_data[9] = head.dst;
        raw_data[10] = head.packetid >> 8;
        raw_data[11] = head.packetid;
        raw_data[12] = head.option >> 8;
        raw_data[13] = head.option;
        break;
    case FlitType::Body:
        //     version:4:flittype:4:flitid:32:message:72:checksum:16
        raw_data[1] = body.id >> 24;
        raw_data[2] = body.id >> 16;
        raw_data[3] = body.id >> 8;
        raw_data[4] = body.id;
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            raw_data[i + 5] = body.data[i];
        }
        break;
    case FlitType::Tail:
        //     version:4:flittype:4:flitid:32:message:72:checksum:16
        raw_data[1] = tail.id >> 24;
        raw_data[2] = tail.id >> 16;
        raw_data[3] = tail.id >> 8;
        raw_data[4] = tail.id;
        for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
            raw_data[i + 5] = tail.data[i];
        }
        break;
    default: break;
    }
    raw_data[14] = checksum >> 8;
    raw_data[15] = checksum;
};
}  // namespace flit