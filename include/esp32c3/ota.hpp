#pragma once

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>

namespace ota {
void ota_init();
}