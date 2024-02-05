#include "esp32c3/fs.hpp"

const char *TAG = "spiffs";

namespace fs {

int SpiFFS::File::read(char *data, int len) {
    if (len <= 0) {
        return 0;
    }
    fgets(data, len, f);
    char *pos = strchr(data, '\n');
    if (pos) {
        *pos = '\0';
    }
    return strlen(data);
};

size_t SpiFFS::File::raw_read(uint16_t *data, int len) {
    if (len <= 0) {
        return 0;
    }
    return fread(data, sizeof(uint16_t), len, f);
}

auto SpiFFS::read_macaddress(void) -> std::expected<network::macaddress_t, traits::FsError> {
    File file = File("/spiffs/macaddress.txt");
    char data[32];
    auto err = file.read(data, sizeof(data));
    if (err <= 0) {
        LOGE(TAG, "Failed to read macaddress");
        return std::unexpected{traits::FsError::GenericError};
    }
    uint32_t macaddress = 0;
    sscanf(data, "%lx", &macaddress);
    LOGI(TAG, "readed macaddress: %s", data);
    LOGI(TAG, "uint32_t macaddress: %08lx", macaddress);
    return macaddress;
}
std::size_t SpiFFS::read_image(const char *path, uint16_t *data, int len) {
    File file = File(path);
    return file.raw_read(data, len);
}
}  // namespace fs