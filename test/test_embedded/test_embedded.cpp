// make google test

#include <gtest/gtest.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "flit.hpp"

TEST(Sample, SampleTest) {
    ASSERT_EQ(1, 1);
}

extern "C" void app_main(void) {
    // default args
    int argc = 1;
    char *argv[] = {"test"};
    ::testing::InitGoogleTest(&argc, argv);
    xTaskCreate(
        [](void *) {
            auto err = RUN_ALL_TESTS();
            if (err != 0) {
                printf("error: %d\n", err);
            }
            while (1) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        },
        "test",
        4096,
        nullptr,
        1,
        nullptr);
}