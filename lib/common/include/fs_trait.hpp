#pragma once
#include <types.hpp>

#include <concepts>
#include <expected>

namespace traits {
enum class FsError {
    Ok,
    GenericError,
};

template <typename T>
concept file_system = requires(T fs, const char *path, uint16_t *buffer, std::size_t size) {
    { fs.read_macaddress() } -> std::same_as<std::expected<network::macaddress_t, FsError>>;
    { fs.read_image(path, buffer, size) } -> std::same_as<std::size_t>;
};
}  // namespace traits