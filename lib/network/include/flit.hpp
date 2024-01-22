#pragma once
#include "config.hpp"
#include "raw_data.hpp"
#include "error.hpp"
#include "header.hpp"
#include "packet_config.hpp"

#include <cstdint>
#include <optional>
#include <variant>
#include <memory>
#include <expected>

namespace flit {
using Header = header::Header;

enum class FlitType : flittype_t {
    Nope = 0,
    Head,
    Body,
    Tail,
};

class Flit {
#if CFG_TEST_PUBLIC == true
  public:
#endif

  public:
    virtual checksum_t culculate_checksum() const = 0;
    virtual FlitType get_type(void) const = 0;
    virtual FlitError validate(void) const = 0;
    virtual std::optional<message_t> get_data(void) = 0;
    virtual std::optional<flitid_t> get_id(void) const = 0;
    virtual void to_rawdata(raw_data_t &raw_data) const = 0;
    virtual ~Flit() = default;
};

class NopeFlit : public Flit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    checksum_t checksum;

  public:
    NopeFlit()
        : version(CONFIG_CURRENT_VERSION)
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
    FlitError validate(void) const override {
        return FlitError::OK;
    };
    std::optional<message_t> get_data(void) override {
        return std::nullopt;
    };
    std::optional<flitid_t> get_id(void) const override {
        return std::nullopt;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const NopeFlit &rhs) const {
        return version == rhs.version;
    };
};

class HeadFlit : public Flit {
#if CFG_TEST_PUBLIC == true
  public:
#endif
    version_t version;
    flitid_t length;
    Header header;
    node_id_t src;
    node_id_t dst;
    packetid_t packetid;
    option_t option;
    checksum_t checksum;

  public:
    HeadFlit(const flitid_t &length,
             const Header &header,
             const packetid_t &packetid,
             const node_id_t &src,
             const node_id_t &dst)
        : version(CONFIG_CURRENT_VERSION)
        , length(length)
        , header(header)
        , src(src)
        , dst(dst)
        , packetid(packetid)
        , option(0)
        , checksum(culculate_checksum()){};

    HeadFlit(const version_t &version,
             const flitid_t &length,
             const Header &header,
             const packetid_t &packetid,
             const node_id_t &src,
             const node_id_t &dst,
             const option_t &option,
             const checksum_t &checksum)
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
    FlitError validate(void) const override;
    std::optional<message_t> get_data(void) override {
        return std::nullopt;
    };
    std::optional<flitid_t> get_id(void) const override {
        return std::nullopt;
    };

    packetid_t get_packet_id() const {
        return packetid;
    }
    flitid_t get_length() const {
        return length;
    }
    node_id_t get_src() const {
        return src;
    }
    node_id_t get_dst() const {
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

class BodyFlit : public Flit {
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
    FlitError validate(void) const override;
    std::optional<message_t> get_data(void) override {
        return data;
    };
    std::optional<flitid_t> get_id(void) const override {
        return id;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const BodyFlit &rhs) const {
        return version == rhs.version && id == rhs.id && data == rhs.data;
    };
};

class TailFlit : public Flit {
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
    FlitError validate(void) const override;
    std::optional<message_t> get_data(void) override {
        return std::move(data);
    };
    std::optional<flitid_t> get_id(void) const override {
        return id;
    };
    void to_rawdata(raw_data_t &raw_data) const override;
    bool operator==(const TailFlit &rhs) const {
        return version == rhs.version && id == rhs.id && data == rhs.data;
    };
};

std::expected<std::unique_ptr<Flit>, FlitError> decoder(raw_data_t &raw_data);

}  // namespace flit