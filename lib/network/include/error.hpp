#pragma once

namespace network {
enum NetworkError {
    OK = 0,

    // parser error
    INVALID_TYPE,
    INVALID_LENGTH,
    INVALID_CHECKSUM,
    INVALID_VERSION,
    INVALID_PROTOCOL,
    INVALID_NOPE,
    ALREADY_FULL,
    NOT_END,
    DATA_SIZE_ERROR,
    INVALID_DATA,

    // for uart
    RECEIVE_ERROR = 100,
    DECODER_ERROR,
    HEAD_FLIT_ERROR,

    // for flit
    INVALID_FLIT_CONFIG_HEAD = 1,
    INVALID_FLIT_CONFIG_BODY,
    INVALID_FLIT_CONFIG_TAIL,

    // for packet
    INVALID_PACKET_CONFIG = 200,

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
}  // namespace network