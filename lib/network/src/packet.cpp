#include "packet.hpp"

#include "flit.hpp"
#include "types.hpp"

#include <variant>
#include <cassert>
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

std::expected<std::unique_ptr<Flit>, NetworkError> Packet::to_flit(const ip_address_t this_id,
                                                                   const network::Routing &routing) noexcept {
    if (current_flit_index == 0) {
        current_flit_index++;

        if (!ready()) {
            return std::unexpected(NetworkError::NOT_READY);
        }
        Header header = Header::Data;
        auto dst = routing.next(this_id, global_dst);

        auto flit = std::make_unique<HeadFlit>(data.size(), header, packetid, this_id, dst);
        auto err = flit->validate();
        if (err != NetworkError::OK) {
            switch (err) {
            case INVALID_LENGTH: return std::unexpected(NetworkError::INVALID_FLIT_LENGTH);
            default: return std::unexpected(INVALID_FLIT_UNKNOWN);
            }
        }
        return flit;
    } else if (current_flit_index == data.size() + 1) {
        // make tail flit
        auto flit = std::make_unique<TailFlit>(current_flit_index, std::move(data[current_flit_index - 1]));
        if (flit->validate() != NetworkError::OK) {
            data[current_flit_index - 1] = std::move(flit->get_data().value());
            return std::unexpected(NetworkError::INVALID_FLIT_UNKNOWN);
        }

        current_flit_index++;
        return flit;
    } else if (current_flit_index < data.size() + 1) {
        // make body flit
        auto flit = std::make_unique<BodyFlit>(current_flit_index, std::move(data[current_flit_index - 1]));
        if (flit->validate() != NetworkError::OK) {
            data[current_flit_index - 1] = std::move(flit->get_data().value());
            return std::unexpected(NetworkError::INVALID_FLIT_UNKNOWN);
        }

        current_flit_index++;
        return flit;
    } else {
        return std::unexpected(NetworkError::ALREADY_FINISHED);
    }
}

NetworkError Packet::load_flit(std::unique_ptr<Flit> &&flit) noexcept {
    // todo
    switch (flit->get_type()) {
    case FlitType::Head: {
        if (flit->validate() != NetworkError::OK) {
            return NetworkError::INVALID_FLIT_UNKNOWN;
        }

        auto head = static_pointer_cast<HeadFlit>(std::move(flit));

        auto flit_length = head->get_length();
        data.resize(flit_length);

        break;
    }
    case FlitType::Body:
    case FlitType::Tail: {
        // TODO if tail, remove eof
        auto flitid = flit->get_id();
        assert(flitid.has_value());
        auto id = flitid.value();
        assert(id - 1 < data.size());
        if (data[id - 1].size() == CONFIG_MESSAGE_LENGTH) {
            // already full
            return NetworkError::ALREADY_FULL;
        }
        auto flit_data = flit->get_data();
        assert(flit_data.has_value());
        data[id - 1] = std::move(flit_data.value());
        break;
    }
    case FlitType::Nope: return NetworkError::INVALID_NOPE;
    default: break;
    }
    return NetworkError::OK;
};
}  // namespace network