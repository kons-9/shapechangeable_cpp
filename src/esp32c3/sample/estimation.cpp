#include <esp32c3/log.hpp>
#include <esp32c3/esp32c3.hpp>
// #include <stub/stub.hpp>
#include <esp32c3/display.hpp>
#include <types.hpp>
#include <estimation.hpp>

const static char *TAG = "estimation_sample";

void responce_estimation_task(void *args) {
    TaskArgs<serial::Uart, fs::SpiFFS> *task_args = (TaskArgs<serial::Uart, fs::SpiFFS> *)args;
    auto &uart = task_args->uart;
    auto &spiffs = task_args->spiffs;

    auto macaddress = spiffs.read_macaddress().value();
    auto ipaddress = estimation::macaddress_to_ip_address(macaddress);
    auto packet = estimation::make_request(ipaddress);
    while (true) {
        packet.send(uart, ipaddress, network::DefaultRouting());
        vTaskDelay(500 / portTICK_PERIOD_MS);
        auto err = packet.receive(uart, ipaddress);
        if (!err) {
            LOGE(TAG, "receive packet error");
            continue;
        }
        LOGI(TAG, "receive packet");
        LOGI(TAG, "packetid: %d", packet.get_packet_id());
        LOGI(TAG, "header  : %d", (network::header_t)packet.get_header());
    }
}
