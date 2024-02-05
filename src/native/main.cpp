
#ifndef __linux__
#error "This file is for Linux only"
#endif

#include <stdio.h>
#include <types.hpp>
#include <stub/stub.hpp>
#include <estimation.hpp>
#include <packet.hpp>
using namespace network;

static LGFX display(1080, 1920, 2);

int main_func(bool *running) {
    display.init();
    while (*running) {
        display.printf("Hello, world!\n");
        sleep(10);
    }
    return 0;
}

int main(void) {
    uint16_t this_ip = 1;
    uint16_t src = 3;
    auto dst = this_ip;
    auto packet_id = 12;
    auto priority = 1;
    auto data = Packet(priority, packet_id, src, BROADCAST_ADDRESS, 0).make_header();
    // confirmed (1,1)
    data.push_back(0xFF);
    data.push_back(src >> 8u);
    data.push_back(src & 0xFFu);
    data.push_back(0);
    data.push_back(1);
    data.push_back(0);
    data.push_back(1);
    data.push_back(0xFF);
    data.insert(data.end(), CONFIG_MESSAGE_LENGTH - data.size() % CONFIG_MESSAGE_LENGTH, 0);
    //
    auto length = data.size() / CONFIG_MESSAGE_LENGTH;

    HeadFlit head(length, Header::COORDINATE_ESTIMATION_RSP, packet_id, src, dst);
    Packet packet;
    auto err = packet.load_flit(this_ip, std::move(head));
    for (size_t i = 0; i < data.size() / CONFIG_MESSAGE_LENGTH - 1; i++) {
        message_t message(CONFIG_MESSAGE_LENGTH);
        for (size_t j = 0; j < CONFIG_MESSAGE_LENGTH; j++) {
            message[j] = data[i * CONFIG_MESSAGE_LENGTH + j];
        }
        auto err = packet.load_flit(this_ip, BodyFlit(i + 1, std::move(message)));
    }
    message_t message(CONFIG_MESSAGE_LENGTH);
    std::copy(data.end() - CONFIG_MESSAGE_LENGTH, data.end(), message.begin());
    err = packet.load_flit(this_ip, TailFlit(data.size() / CONFIG_MESSAGE_LENGTH + 1, std::move(message)));
    auto data2 = packet.get_data();

    printf("Hello, world!\n");
    return lgfx::Panel_sdl::main(main_func);
}