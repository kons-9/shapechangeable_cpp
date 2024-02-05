#include "packet.hpp"

#include "flit.hpp"
#include "types.hpp"

#include <_log.hpp>

#include <variant>
#include <expected>

namespace network {

headchecksum_t Packet::caluculate_checksum() const {
    headchecksum_t checksum = 0;
    checksum += version;
    checksum += head_length;
    checksum += priority;
    checksum += packetid;
    checksum += global_src;
    checksum += global_dst;
    checksum += flag;
    checksum += fragment;
    checksum += protocol;
    return checksum;
}

flitid_t Packet::calculate_flit_length(const message_t &data) const noexcept {
    return (data.size() + packet::CONFIG_NORMAL_HEADER_SIZE + CONFIG_MESSAGE_LENGTH - 1) / CONFIG_MESSAGE_LENGTH;
}

std::vector<message_element_t> Packet::make_header() const {
    std::vector<message_element_t> header;
    header.push_back(version);
    header.push_back(head_length);
    header.push_back(priority);
    header.push_back(packetid >> 8);
    header.push_back(packetid & 0xff);
    header.push_back(global_src >> 8);
    header.push_back(global_src & 0xff);
    header.push_back(global_dst >> 8);
    header.push_back(global_dst & 0xff);
    header.push_back(flag);
    header.push_back(fragment);
    header.push_back(headchecksum >> 8);
    header.push_back(headchecksum & 0xff);
    header.push_back(protocol);
    return header;
}
NetworkError Packet::validate(void) const {
    if (flag & HASFRAGMENT || flag & LASTFRAGMENT || fragment != 0) {
        // currently, unsupported
        return NetworkError::UNSUPPORTED;
    }
    auto length = data.size();
    if (length > packet::CONFIG_MTU - 1) {
        return NetworkError::OVER_MTU;
    }
    if (headchecksum != caluculate_checksum()) {
        return NetworkError::CHECKSUM_NOT_MATCH;
    }
    if (version > packet::CONFIG_CURRENT_VERSION) {
        return NetworkError::VERSION_UNSUPPORTED;
    }
    return NetworkError::OK;
}

bool Packet::send_ready(void) {
    if (only_header(header)) {
        return true;
    }
    if (data.size() > packet::CONFIG_MTU - 1) {
        return false;
    }
    // EOF
    data.push_back(flit::FLIT_EOF);
    auto rem = (data.size() + head_raw_data.size()) % flit::CONFIG_MESSAGE_LENGTH;
    if (rem != 0) {
        // push 0 to last
        data.insert(data.end(), flit::CONFIG_MESSAGE_LENGTH - rem, 0);
    }
    return true;
}

std::expected<Flit, NetworkError> Packet::to_flit(const ip_address_t this_id,
                                                  const network::Routing &routing) noexcept {
    if (current_flit_index == 0) {
        current_flit_index++;
        if (header == Header::None) {
            return NopeFlit();
        }

        if (!send_ready()) {
            return std::unexpected(NetworkError::NOT_READY);
        }
        auto dst = routing.next(this_id, global_dst);

        auto flit = HeadFlit(flit_length, header, packetid, this_id, dst);
        auto err = flit.validate();
        if (err != NetworkError::OK) {
            switch (err) {
            case INVALID_LENGTH: return std::unexpected(NetworkError::INVALID_FLIT_LENGTH);
            default: return std::unexpected(INVALID_FLIT_UNKNOWN);
            }
        }
        LOGI("Packet::to_flit", "success to make head flit");
        return flit;
    } else if (current_flit_index < flit_length) {
        // make body flit
        auto flit_data = std::vector<message_element_t>(CONFIG_MESSAGE_LENGTH);
        auto flit_data_index = 0;
        auto id = current_flit_index;
        current_flit_index++;
        // copy data TODO
        if (header_index < head_raw_data.size()) {
            // header_raw_data to flit
            if (head_raw_data.size() - header_index < CONFIG_MESSAGE_LENGTH) {
                std::copy(head_raw_data.begin() + header_index, head_raw_data.end(), flit_data.begin());
                flit_data_index = head_raw_data.size() - header_index;
                header_index = head_raw_data.size();
            } else {
                std::copy(head_raw_data.begin() + header_index,
                          head_raw_data.begin() + header_index + CONFIG_MESSAGE_LENGTH,
                          flit_data.begin());
                header_index += CONFIG_MESSAGE_LENGTH;
                return BodyFlit(id, std::move(flit_data));
            }
        }

        std::copy(data.begin() + data_index,
                  data.begin() + data_index + CONFIG_MESSAGE_LENGTH - flit_data_index,
                  flit_data.begin() + flit_data_index);
        data_index += CONFIG_MESSAGE_LENGTH - flit_data_index;

        auto flit = BodyFlit(id, std::move(flit_data));
        if (flit.validate() != NetworkError::OK) {
            return std::unexpected(NetworkError::INVALID_FLIT_UNKNOWN);
        }

        LOGI("Packet::to_flit", "success to make body flit");
        return flit;
    } else if (current_flit_index == flit_length) {
        if (only_header(header)) {
            return std::unexpected(NetworkError::ALREADY_FINISHED);
        }
        // make tail flit
        auto flit_data = std::vector<message_element_t>(CONFIG_MESSAGE_LENGTH);
        auto flit_data_index = 0;
        auto id = current_flit_index;
        current_flit_index++;
        // copy data TODO
        if (header_index < head_raw_data.size()) {
            // header_raw_data to flit
            if (head_raw_data.size() - header_index < CONFIG_MESSAGE_LENGTH) {
                std::copy(head_raw_data.begin() + header_index, head_raw_data.end(), flit_data.begin());
                flit_data_index = head_raw_data.size() - header_index;
                header_index = head_raw_data.size();
            } else {
                std::copy(head_raw_data.begin() + header_index,
                          head_raw_data.begin() + header_index + CONFIG_MESSAGE_LENGTH,
                          flit_data.begin());
                header_index += CONFIG_MESSAGE_LENGTH;
                return TailFlit(id, std::move(flit_data));
            }
        }

        std::copy(data.begin() + data_index,
                  data.begin() + data_index + CONFIG_MESSAGE_LENGTH - flit_data_index,
                  flit_data.begin() + flit_data_index);
        data_index += CONFIG_MESSAGE_LENGTH - flit_data_index;

        auto flit = TailFlit(id, std::move(flit_data));
        if (flit.validate() != NetworkError::OK) {
            return std::unexpected(NetworkError::INVALID_FLIT_UNKNOWN);
        }
        LOGI("Packet::to_flit", "success to make tail flit");
        return flit;
    } else {
        return std::unexpected(NetworkError::ALREADY_FINISHED);
    }
}

NetworkError Packet::parse_header(const message_t &data, size_t &last_index) {
    if (header_index == CONFIG_MESSAGE_LENGTH) {
        // 一回処理している
        fragment = data[0];
        headchecksum = data[1] << 8 | data[2];
        protocol = data[3];

        header_index = 14;
        header_finished = true;
        last_index = 4;
    } else {
        if (data.size() < CONFIG_MESSAGE_LENGTH) {
            return NetworkError::INVALID_LENGTH;
        }
        version = data[0];
        head_length = data[1];
        priority = data[2];
        packetid = data[3] << 8 | data[4];
        global_src = data[5] << 8 | data[6];
        global_dst = data[7] << 8 | data[8];
        flag = data[9];
        header_index = 10;
        last_index = 10;
    }

    return NetworkError::OK;
}

NetworkError Packet::load_flit(const network::ip_address_t this_id, Flit &&flit) noexcept {
    // todo
    switch (std::visit([](auto &&flit) { return flit.get_type(); }, flit)) {
    case FlitType::Head: {
        auto head = std::get<HeadFlit>(std::move(flit));
        if (head.validate() != NetworkError::OK) {
            LOGW("Packet::load_flit", "head flit validate error");
            return NetworkError::INVALID_FLIT_UNKNOWN;
        }
        header = head.get_header();
        packetid = head.get_packet_id();
        // src = head.get_src();
        auto dst = head.get_dst();
        if (dst != this_id && dst != BROADCAST_ADDRESS) {
            LOGW("Packet::load_flit", "head dst error");
            return NetworkError::INVALID_DESTINATION;
        }
        // option = head.get_option();
        flit_length = head.get_length();
        header_finished = only_header(header);
        if (header_finished) {
            current_flit_index++;
            LOGI("Packet::load_flit", "header finished");
            break;
        }
        data.resize(flit_length * CONFIG_FLIT_LENGTH);
        LOGI("Packet::load_flit", "head flit_length: %d", flit_length);
        break;
    }
    case FlitType::Body: {
        auto body = std::get<BodyFlit>(std::move(flit));
        auto id = body.get_id();
        current_flit_index++;
        if (current_flit_index != id) {
            LOGW("Packet::load_flit", "body id error, expected: %d, actual: %d", current_flit_index, id);
            return NetworkError::INVALID_ID;
        }
        auto err = body.validate();
        if (err != NetworkError::OK) {
            LOGW("Packet::load_flit", "body flit validate error %d", err);
            return err;
        }
        auto flit_data = body.get_data();
        if (header_finished) {
            auto base = (id - (head_length + CONFIG_MESSAGE_LENGTH - 1) / CONFIG_MESSAGE_LENGTH) * CONFIG_MESSAGE_LENGTH
                        - head_length % CONFIG_MESSAGE_LENGTH;
            if (base + CONFIG_MESSAGE_LENGTH >= data.size()) {
                LOGW("Packet::load_flit", "body base error, base: %d, flit_length: %d", base, flit_length);
                return NetworkError::INVALID_ID;
            }
            for (auto i = base; i < base + CONFIG_MESSAGE_LENGTH; i++) {
                data[i] = flit_data[i - base];
            }
            return NetworkError::OK;
        }
        std::size_t last_index = 0;
        err = parse_header(flit_data, last_index);
        if (err != NetworkError::OK) {
            LOGW("Packet::load_flit", "body parse header error, %d", err);
            return err;
        }
        if (last_index == flit_data.size()) {
            // header only body
            return NetworkError::OK;
        }
        for (auto i = last_index; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i - last_index] = flit_data[i];
        }
        break;
    }
    case FlitType::Tail: {
        // TODO if tail, remove eof
        auto tail = std::get<TailFlit>(std::move(flit));
        auto id = tail.get_id();
        current_flit_index++;
        if (current_flit_index != flit_length || id != current_flit_index) {
            LOGE("tail", "id: %d, flit_length: %d, current_flit_index: %d\n", id, flit_length, current_flit_index);
            return NetworkError::INVALID_TAIL;
        }
        auto err = tail.validate();
        if (err != NetworkError::OK) {
            LOGW("Packet::load_flit", "tail flit validate error, %d", err);
            return err;
        }
        auto flit_data = tail.get_data();
        if (header_finished) {
            auto base = (id - (head_length + CONFIG_MESSAGE_LENGTH - 1) / CONFIG_MESSAGE_LENGTH) * CONFIG_MESSAGE_LENGTH
                        - head_length % CONFIG_MESSAGE_LENGTH;
            if (base + CONFIG_MESSAGE_LENGTH >= data.size()) {
                LOGW("Packet::load_flit", "tail base error, base: %d, flit_length: %d", base, flit_length);
                return NetworkError::INVALID_ID;
            }
            for (auto i = base; i < base + CONFIG_MESSAGE_LENGTH; i++) {
                data[i] = flit_data[i - base];
            }
            return NetworkError::OK;
        }
        std::size_t last_index = 0;
        err = parse_header(flit_data, last_index);
        if (err != NetworkError::OK) {
            LOGW("Packet::load_flit", "tail parse header error, %d", err);
            return err;
        }
        if (last_index == flit_data.size()) {
            LOGI("Packet::load_flit", "last index tail, expected: %d, actual: %d", CONFIG_MESSAGE_LENGTH, last_index);
            return NetworkError::OK;
        }
        for (auto i = last_index; i < CONFIG_MESSAGE_LENGTH; i++) {
            data[i - last_index] = flit_data[i];
        }
        break;
    }
    case FlitType::Nope: LOGW("Packet::load_flit", "nope flit"); return NetworkError::INVALID_NOPE;
    default: break;
    }
    return NetworkError::OK;
};
}  // namespace network