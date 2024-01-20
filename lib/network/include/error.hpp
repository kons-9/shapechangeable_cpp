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