#include <esp32c3/log.hpp>
#include <esp32c3/esp32c3.hpp>
#include <esp32c3/display.hpp>
#include <types.hpp>
#include <estimation.hpp>

const static char *TAG = "tei_demo_app";

void sample_tei_demo_app(void *arg) {
    LOGI(TAG, "sample_tei_demo_app");
    TaskArgs<serial::Link, fs::SpiFFS> *task_args = (TaskArgs<serial::Link, fs::SpiFFS> *)arg;
    auto &uart = task_args->uart;
    auto &lov_display = task_args->lov_display;
    auto &spiffs = task_args->spiffs;
    auto macaddress = spiffs.read_macaddress().value();
    auto ipaddress = estimation::macaddress_to_ip_address(macaddress);
    auto image = task_args->image_buffer;
    std::vector<std::pair<network::macaddress_t, estimation::coordinate_t>> coordinates;
    estimation::coordinate_t coordinate;

    // firstly, confirm the coordinate
    LOGI(TAG, "confirming the coordinate...");
    while (estimation::is_finished(ipaddress, coordinates)) {
        auto packet = estimation::make_request(ipaddress);
        packet.send(uart, ipaddress, network::DefaultRouting());
        auto err = packet.receive(uart, ipaddress);
        if (err == false) {
            LOGE(TAG, "receive packet error");
            continue;
        }
        if (packet.get_header() != network::Header::COORDINATE_ESTIMATION) {
            LOGI(TAG, "coordinate request");
            auto not_confirmed_packet = estimation::make_response_to_same_unit(false, ipaddress, coordinates);
            not_confirmed_packet.send(uart, ipaddress, network::DefaultRouting());
            continue;
        }
        if (packet.get_header() != network::Header::COORDINATE_ESTIMATION_RSP) {
            LOGW(TAG, "receive packet: header error");
            continue;
        }
        LOGI(TAG, "receive responce packet");

        std::vector<std::pair<network::macaddress_t, estimation::coordinate_t>> coordinates;
        err = estimation::process_data(packet.get_src(), ipaddress, packet.get_data(), coordinates);
    }
    coordinate = estimation::get_coordinate(ipaddress, coordinates);
    LOGI(TAG, "confirmed coordinate: x: %d, y: %d", coordinate.first, coordinate.second);

    // secondly, display test_x_y.raw
    LOGI(TAG, "displaying test_%d_%d.raw...", coordinate.first, coordinate.second);
    auto basepath = "/spiffs/test_%d_%d.raw";
    char path[100];
    sprintf(path, basepath, coordinate.first, coordinate.second);

    auto len = spiffs.read_image(path, image, 128 * 128);
    LOGI(TAG, "readed image: %d", len);
    lov_display.pushImage(0, 0, 128, 128, image);

    // finaly, response
    LOGI(TAG, "sending response packet...");
    while (true) {
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
        if (estimation::is_same_unit_node(packet.get_src(), ipaddress)) {
            LOGI(TAG, "same unit node");
            // TODO
            // めんどくさいので確定していないことにして返す。ちゃんと渡れば確定しできるはず
            auto not_confirmed_packet = estimation::make_response_to_same_unit(false, ipaddress, coordinates);
            not_confirmed_packet.send(uart, ipaddress, network::DefaultRouting());
        } else {
            LOGI(TAG, "different unit node");
            auto confirmed_packet = estimation::make_response_to_other_unit(ipaddress, coordinate);
            confirmed_packet.send(uart, ipaddress, network::DefaultRouting());
        }
    }
}