#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct SystemMessage {
    int event_id;
    float float_val;
};

class MsgBus {
public:
    static MsgBus& get_instance();
    bool send(const SystemMessage& msg);
    bool receive(SystemMessage& msg, TickType_t wait_ticks);

private:
    MsgBus();
    QueueHandle_t msg_queue = nullptr;
};
