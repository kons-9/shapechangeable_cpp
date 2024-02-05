#pragma once
#include "_log.hpp"

#include <concepts.hpp>

#include <expected>
#include <cstring>
#include "esp_err.h"
#include "esp_spiffs.h"

namespace fs {

// pull data from spiffs
class SpiFFS {
    // read only file class
    class File {

      public:
        File(const char *path) {
            const char *mode = "r";
            const char *TAG = "fils";
            f = fopen(path, mode);
            if (f == NULL) {
                LOGE(TAG, "Failed to open file for reading");
            }
        }
        ~File() {
            if (f != NULL) {
                fclose(f);
            }
        }
        int read(char *data, int len);
        size_t raw_read(uint16_t *data, int len);

      private:
        FILE *f;
    };

  public:
    SpiFFS() {
        const char *TAG = "spiffs";
        esp_vfs_spiffs_conf_t conf
            = {.base_path = "/spiffs", .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true};
        esp_err_t ret = esp_vfs_spiffs_register(&conf);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                LOGE(TAG, "Failed to mount or format filesystem");
            } else if (ret == ESP_ERR_NOT_FOUND) {
                LOGE(TAG, "Failed to find SPIFFS partition");
            } else {
                LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            }
        }
        LOGE(TAG, "mount success");
    }
    ~SpiFFS() {
        esp_vfs_spiffs_unregister(NULL);
    }

    // for file_system trait
    auto read_macaddress(void) -> std::expected<network::macaddress_t, traits::FsError>;
    std::size_t read_image(const char *path, uint16_t *data, int len);
};

static_assert(traits::file_system<SpiFFS>);

}  // namespace fs