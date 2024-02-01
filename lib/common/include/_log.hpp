#ifndef LOG
#ifdef ESP_PLATFORM
#include <esp_log.h>
#define LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#elif ARDUINO
#include <Arduino.h>
#define LOGI(tag, fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)
#else  // native
#include <cstdio>
#define LOGI(tag, fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#endif
#endif  // LOG

#ifdef LOG_DISABLE
#define LOGI_DISABLE
#define LOGD_DISABLE
#define LOGW_DISABLE
#define LOGE_DISABLE
#endif

#ifdef LOGI_DISABLE
#undef LOGI
#endif

#ifdef LOGD_DISABLE
#undef LOGD
#endif

#ifdef LOGW_DISABLE
#undef LOGW
#endif

#ifdef LOGE_DISABLE
#undef LOGE
#endif

// default values
#ifndef LOGI
#define LOGI(tag, fmt, ...)
#endif

#ifndef LOGD
#define LOGD(tag, fmt, ...)
#endif

#ifndef LOGW
#define LOGW(tag, fmt, ...)
#endif

#ifndef LOGE
#define LOGE(tag, fmt, ...)
#endif

#define LOG