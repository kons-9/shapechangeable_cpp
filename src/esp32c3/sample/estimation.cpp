#include <esp32c3/log.hpp>
#include <esp32c3/esp32c3.hpp>
// #include <stub/stub.hpp>
#include <esp32c3/display.hpp>
#include <types.hpp>
#include <estimation.hpp>

const static char *TAG = "estimation_sample";

void sample_estimation_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &lov_display = task_args->lov_display;

    // auto &spiffs = task_args->spiffs;
    // auto macaddress = spiffs.read_macaddress().value();
    // auto ipaddress = estimation::macaddress_to_ip_address(macaddress);

    auto ipaddress = 0b11110000;
    uart.set_ip_address(ipaddress);
    auto cnt = 0;
    while (true) {
        // vTaskDelay(100 / portTICK_PERIOD_MS);
        auto packet = estimation::make_request(ipaddress);
        packet.send(uart, ipaddress, network::DefaultRouting());
        // packet = network::Packet();
        auto err = packet.receive(uart, ipaddress);
        if (err == false) {
            LOGE(TAG, "receive packet error");
            continue;
        }
        if (packet.get_header() != network::Header::COORDINATE_ESTIMATION_RSP) {
            LOGW(TAG, "receive packet: header error");
            continue;
        }
        LOGI(TAG, "receive responce packet");

        std::vector<std::pair<network::macaddress_t, estimation::coordinate_t>> coordinates;
        err = estimation::process_data(packet.get_src(), ipaddress, packet.get_data(), coordinates);
        if (err != network::NetworkError::OK) {
            LOGE(TAG, "process data error");
            continue;
        }

        for (auto &coordinate : coordinates) {
            LOGI(TAG, "macaddress: x: %d, y: %d", coordinate.second.first, coordinate.second.second);
            lov_display.fillScreen(TFT_BLACK);
            lov_display.setCursor(0, 0);
            lov_display.printf("macaddress: x: %d, y: %d, cnt: %d",
                               coordinate.second.first,
                               coordinate.second.second,
                               cnt++);
        }
        // vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sample_other_confirmed_estimaiton_task(void *args) {
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &lov_display = task_args->lov_display;
    // auto &spiffs = task_args->spiffs;
    // auto macaddress = spiffs.read_macaddress().value();
    // auto ipaddress = estimation::macaddress_to_ip_address(macaddress);
    auto ipaddress = 0b11100000;
    uart.set_ip_address(ipaddress);
    auto cnt = 0;
    while (true) {
        // vTaskDelay(2000 / portTICK_PERIOD_MS);
        auto packet = network::Packet();
        auto err = packet.receive(uart, ipaddress);
        if (err == false) {
            LOGE(TAG, "receive packet error");
            continue;
        }
        if (packet.get_header() != network::Header::COORDINATE_ESTIMATION) {
            LOGW(TAG, "receive packet: header error");
            continue;
        }
        LOGI(TAG, "receive packet");

        auto confirmed_packet = estimation::make_response_to_other_unit(ipaddress, std::make_pair(0, 0));
        // vTaskDelay(10 / portTICK_PERIOD_MS);
        confirmed_packet.send(uart, ipaddress, network::DefaultRouting());
        lov_display.fillScreen(TFT_BLACK);
        lov_display.setCursor(0, 0);
        lov_display.printf("send confirmed packet %d", cnt++);
    }
}
