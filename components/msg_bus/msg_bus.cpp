#include "msg_bus.hpp"

MsgBus& MsgBus::get_instance() {
    static MsgBus instance;
    return instance;
}

MsgBus::MsgBus() {
    msg_queue = xQueueCreate(10, sizeof(SystemMessage));
}

bool MsgBus::send(const SystemMessage& msg) {
    return xQueueSend(msg_queue, &msg, 0) == pdTRUE;
}

bool MsgBus::receive(SystemMessage& msg, TickType_t wait_ticks) {
    return xQueueReceive(msg_queue, &msg, wait_ticks) == pdTRUE;
}
