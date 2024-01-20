#pragma once
#include <cstdint>
#include "config.hpp"
#include "raw_data.hpp"
#include "error.hpp"

namespace flit {

enum class FlitType : uint8_t {
    Nope = 0,
    Head,
    Body,
    Tail,
};

enum class Header : uint8_t {
    None = 0,
};

struct Flit {
#if CFG_TEST_PUBLIC == true
  public:
#else
  private:
#endif
    FlitType type : 4;
    version_t version : 4;
    checksum_t checksum;
    union {
        // nope flit
        struct {
        } nope;
        // head flit
        struct {
            flitid_t length;
            Header header;
            node_id_t src;
            node_id_t dst;
            packetid_t packetid;
            option_t option;
        } head;
        // body flit
        struct {
            flitid_t id;
            message_t data;
        } body;
        // tail flit
        struct {
            flitid_t id;
            message_t data;
        } tail;
    };
    checksum_t culculate_checksum();

  public:
    FlitType get_type(void) {
        return type;
    };
    FlitError validate(void);
    void to_rawdata(raw_data_t &raw_data);
    // head flit
    Flit(flitid_t &&length, Header &&header, packetid_t &&packetid, node_id_t &&src, node_id_t &&dst) {
        type = FlitType::Head;
        version = CONFIG_CURRENT_VERSION;
        head.length = std::move(length);
        head.header = std::move(header);
        head.src = std::move(src);
        head.dst = std::move(dst);
        head.packetid = std::move(packetid);
        head.option = 0;

        checksum = culculate_checksum();
    };

    // body flit or tail flit
    Flit(flitid_t &&id, message_t &&data, bool is_tail = false) {
        version = CONFIG_CURRENT_VERSION;
        if (is_tail) {
            type = FlitType::Tail;
            id = std::move(id);
            tail.data = std::move(data);
        } else {
            type = FlitType::Body;
            id = std::move(id);
            body.data = std::move(data);
        }

        checksum = culculate_checksum();
    };

    // nope flit
    Flit()
        : type(FlitType::Nope)
        , version(CONFIG_CURRENT_VERSION)
        , checksum(0){};


    // decoder
    Flit(raw_data_t &raw_data) {
        type = static_cast<FlitType>(raw_data[0] & 0x0f);
        version = raw_data[0] >> 4;

        switch (type) {
        case FlitType::Head:
            //     version:4:flittype:4:flitid:32:header:8:nodeid:16:nodeid:16:packetid:16:option:16:checksum:16
            head.length = raw_data[1] << 24 | raw_data[2] << 16 | raw_data[3] << 8 | raw_data[4];
            head.header = static_cast<Header>(raw_data[5]);
            head.src = raw_data[6] << 8 | raw_data[7];
            head.dst = raw_data[8] << 8 | raw_data[9];
            head.packetid = raw_data[10] << 8 | raw_data[11];
            head.option = raw_data[12] << 8 | raw_data[13];
            break;
        case FlitType::Body:
            //     version:4:flittype:4:flitid:32:message:72:checksum:16
            body.id = raw_data[1] << 24 | raw_data[2] << 16 | raw_data[3] << 8 | raw_data[4];

            for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
                body.data[i] = raw_data[i + 5];
            }
            break;
        case FlitType::Tail:
            //     version:4:flittype:4:flitid:32:message:72:checksum:16
            tail.id = raw_data[1] << 24 | raw_data[2] << 16 | raw_data[3] << 8 | raw_data[4];
            for (int i = 0; i < CONFIG_MESSAGE_LENGTH; i++) {
                tail.data[i] = raw_data[i + 5];
            }
            break;
        case FlitType::Nope: break;
        default: break;
        }
        checksum = raw_data[14] << 8 | raw_data[15];
    }
    bool operator==(const Flit &rhs) const {
        if (type != rhs.type || version != rhs.version) {
            return false;
        }
        switch (type) {
        case FlitType::Head:
            if (head.length != rhs.head.length || head.header != rhs.head.header || head.src != rhs.head.src
                || head.dst != rhs.head.dst || head.packetid != rhs.head.packetid || head.option != rhs.head.option) {
                return false;
            }
            break;
        case FlitType::Body:
            if (body.id != rhs.body.id || body.data != rhs.body.data) {
                return false;
            }
            break;
        case FlitType::Tail:
            if (tail.id != rhs.tail.id || tail.data != rhs.tail.data) {
                return false;
            }
            break;
        case FlitType::Nope: break;
        default: break;
        }
        if (checksum != rhs.checksum) {
            return false;
        }
        return true;
    }
};

}  // namespace flit