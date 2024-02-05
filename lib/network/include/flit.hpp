#pragma once
#include "types.hpp"

#include <cstdint>
#include <optional>
#include <memory>
#include <expected>

#include <concepts.hpp>

namespace network {
using namespace flit;

enum class FlitType : flittype_t {
    Nope = 0,
    Head,
    Body,
    Tail,
};


class BaseFlit {
#if CFG_TEST_PUBLIC == true
  public:
#endif

  public:
    virtual checksum_t culculate_checksum() const = 0;
    virtual FlitType get_type(void) const = 0;
    virtual NetworkError validate(void) const = 0;
    virtual void to_rawdata(raw_data_t &raw_data) const = 0;
    template <traits::serial T>
    traits::SerialError send(T &sender) const {
        raw_data_t raw_data;
        to_rawdata(raw_data);
        // todo: automatically ack
        return sender.send(raw_data);
    }
    virtual ~BaseFlit() = default;
};

class NopeFlit : public BaseFlit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    checksum_t checksum;

  public:
    NopeFlit()
        : version(flit::CONFIG_CURRENT_VERSION)
        , checksum(0){};

    NopeFlit(const version_t &version, const checksum_t &checksum)
        : version(version)
        , checksum(checksum){};

    checksum_t culculate_checksum() const override {
        return 0;
    };
    FlitType get_type(void) const override {
        return FlitType::Nope;
    };
    NetworkError validate(void) const override {
        return NetworkError::OK;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const NopeFlit &rhs) const {
        return version == rhs.version;
    };
};

class HeadFlit : public BaseFlit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    flitid_t length;
    Header header;
    ip_address_t src;
    ip_address_t dst;
    packetid_t packetid;
    option_t option;
    checksum_t checksum;

  public:
    HeadFlit(flitid_t length, Header header, packetid_t packetid, ip_address_t src, ip_address_t dst)
        : version(CONFIG_CURRENT_VERSION)
        , length(length)
        , header(header)
        , src(src)
        , dst(dst)
        , packetid(packetid)
        , option(0)
        , checksum(culculate_checksum()){};

    HeadFlit(version_t version,
             flitid_t length,
             Header header,
             packetid_t packetid,
             ip_address_t src,
             ip_address_t dst,
             option_t option,
             checksum_t checksum)
        : version(version)
        , length(length)
        , header(header)
        , src(src)
        , dst(dst)
        , packetid(packetid)
        , option(option)
        , checksum(checksum){};
    checksum_t culculate_checksum() const override;
    FlitType get_type(void) const override {
        return FlitType::Head;
    };
    NetworkError validate(void) const override;

    packetid_t get_packet_id() const {
        return packetid;
    }
    flitid_t get_length() const {
        return length;
    }
    ip_address_t get_src() const {
        return src;
    }
    ip_address_t get_dst() const {
        return dst;
    }
    Header get_header() const {
        return header;
    }
    option_t get_option() const {
        return option;
    }

    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const HeadFlit &rhs) const {
        return version == rhs.version && length == rhs.length && header == rhs.header && src == rhs.src
               && dst == rhs.dst && packetid == rhs.packetid && option == rhs.option;
    };
};

class BodyFlit : public BaseFlit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    flitid_t id;
    message_t data;
    checksum_t checksum;

  public:
    BodyFlit(const flitid_t &id, message_t &&data)
        : version(CONFIG_CURRENT_VERSION)
        , id(id)
        , data(std::move(data))
        , checksum(culculate_checksum()){};

    BodyFlit(const version_t &version, const flitid_t &id, message_t &&data, const checksum_t &checksum)
        : version(version)
        , id(id)
        , data(std::move(data))
        , checksum(checksum){};
    checksum_t culculate_checksum() const override;
    FlitType get_type(void) const override {
        return FlitType::Body;
    };
    NetworkError validate(void) const override;
    message_t get_data(void) {
        return data;
    };
    flitid_t get_id(void) const {
        return id;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const BodyFlit &rhs) const {
        return version == rhs.version && id == rhs.id && data == rhs.data;
    };
};

class TailFlit : public BaseFlit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    flitid_t id;
    message_t data;
    checksum_t checksum;

  public:
    TailFlit(const flitid_t &id, message_t &&data)
        : version(CONFIG_CURRENT_VERSION)
        , id(id)
        , data(std::move(data))
        , checksum(culculate_checksum()){};
    TailFlit(const version_t &version, const flitid_t &id, message_t &&data, const checksum_t &checksum)
        : version(version)
        , id(id)
        , data(std::move(data))
        , checksum(checksum){};
    checksum_t culculate_checksum() const override;
    FlitType get_type(void) const override {
        return FlitType::Tail;
    };
    NetworkError validate(void) const override;
    message_t get_data(void) {
        return data;
    };
    flitid_t get_id(void) const {
        return id;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const TailFlit &rhs) const {
        return version == rhs.version && id == rhs.id && data == rhs.data;
    };
};

std::expected<Flit, NetworkError> decoder(raw_data_t &raw_data);

template <traits::serial T>
traits::SerialError receive(T &receiver, Flit &flit, const network::ip_address_t this_id) {
    raw_data_t raw_data;
    auto err = receiver.receive(raw_data);
    if (!err) {
        return err;
    }
    flit = decoder(raw_data).value();
    if (std::holds_alternative<HeadFlit>(flit)) {
        auto headflit = std::get<HeadFlit>(std::move(flit));
        if (headflit.get_dst() != this_id && headflit.get_dst() != network::BROADCAST_ADDRESS) {
            return traits::SerialError::GenericError;
        }
    }
    return traits::SerialError::Ok;
}

template <traits::serial T>
traits::SerialError send(T &sender, const Flit &flit) {
    return std::visit([&sender](auto &&arg) { return arg.send(sender); }, flit);
}


}  // namespace network