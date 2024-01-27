#pragma once
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include <expected>
#include <cstring>

namespace fs {


class SpiFFS {
    class File {

      public:
        File(const char *path) {
            const char *mode = "r";
            f = fopen(path, mode);
            if (f == NULL) {
                ESP_LOGE("spiffs", "Failed to open file for reading");
            }
        }
        ~File() {
            if (f != NULL) {
                fclose(f);
            }
        }
        int read(char *data, int len) {
            if (len <= 0) {
                return 0;
            }
            fgets(data, len, f);
            char *pos = strchr(data, '\n');
            if (pos) {
                *pos = '\0';
            }
            return strlen(data);
        }

        size_t raw_read(uint16_t *data, int len) {
            if (len <= 0) {
                return 0;
            }
            return fread(data, sizeof(uint16_t), len, f);
        }

      private:
        FILE *f;
    };

    const char *TAG = "spiffs";

  public:
    SpiFFS() {
        esp_vfs_spiffs_conf_t conf
            = {.base_path = "/spiffs", .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true};
        esp_err_t ret = esp_vfs_spiffs_register(&conf);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to mount or format filesystem");
            } else if (ret == ESP_ERR_NOT_FOUND) {
                ESP_LOGE(TAG, "Failed to find SPIFFS partition");
            } else {
                ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            }
        }
        ESP_LOGE(TAG, "mount success");
    }

    std::expected<uint32_t, int> read_macaddress(void) {
        File file = File("/spiffs/macaddress.txt");
        char data[32];
        auto err = file.read(data, sizeof(data));
        if (err <= 0) {
            ESP_LOGE(TAG, "Failed to read macaddress");
            return std::unexpected{err};
        }
        uint32_t macaddress = 0;
        sscanf(data, "%lx", &macaddress);
        ESP_LOGI(TAG, "readed macaddress: %s", data);
        ESP_LOGI(TAG, "uint32_t macaddress: %08lx", macaddress);
        return macaddress;
    }

    size_t read_image(const char *path, uint16_t *data, int len) {
        File file = File(path);
        return file.raw_read(data, len);
    }

    ~SpiFFS() {
        esp_vfs_spiffs_unregister(NULL);
    }
};
}  // namespace fs