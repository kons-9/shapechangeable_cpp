#pragma once

namespace flit {
enum FlitError {
    OK = 0,

    INVALID_FLIT_CONFIG_HEAD = 1,
    INVALID_FLIT_CONFIG_BODY,
    INVALID_FLIT_CONFIG_TAIL,

    // parser error
    INVALID_TYPE = 100,
    INVALID_LENGTH,
    INVALID_CHECKSUM,
    INVALID_VERSION,
};
}  // namespace flit
namespace packet {
enum PacketError {
    OK = 0,

    INVALID_PACKET_CONFIG = 1,

    // parser error
    INVALID_VERSION = 100,
    INVALID_LENGTH,
    INVALID_CHECKSUM,
    INVALID_PROTOCOL,
    INVALID_NOPE,
    NOT_END,

    // validation
    OVER_MTU,
    EMPTY,
    NOT_SAME_PACKET_ID,
    INSUFFICIENT_FRAGMENT,
    VERSION_UNSUPPORTED,
    UNSUPPORTED,
    CHECKSUM_NOT_MATCH,

    // to flit
    NOT_READY,
    EXPECTED_HEAD,
    EXPECTED_BODY,
    EXPECTED_TAIL,
    ALREADY_FINISHED,
    INVALID_FLIT_LENGTH,
    INVALID_FLIT_UNKNOWN,  // treated as fatal error
};
}  // namespace packet