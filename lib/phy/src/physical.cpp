#include "physical.hpp"

namespace physical {

void wifi_init() {
    // do something
}

void Uart::uart_event_task(void *args) {
    uart_event_t event;
    while (true) {
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                // normal event
                received_data_size = uart_read_bytes(UART_NUM, received_data, event.size, 100);
                break;
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                // overflow, reset queue
                ESP_LOGI("uart", "overflow");
                reset();
                break;
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                // collision detect
                ESP_LOGI("uart", "collision detect");
                reset();
                break;
            case UART_BREAK:
            case UART_PATTERN_DET:
            default:
                // unhandeled event
                ESP_LOGI("uart", "uart event type: %d", event.type);
                break;
            }
        }
    }
}
}  // namespace physical