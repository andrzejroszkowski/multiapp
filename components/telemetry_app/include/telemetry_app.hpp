#pragma once
#include "app_interface.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

class TelemetryApp : public AppInterface {
public:
    void start() override;
    void stop() override;
    std::string get_name() const override { return "Telemetry"; }

private:
    static void consumer_task(void* p);
    TaskHandle_t task_h = nullptr;
    lv_obj_t* label = nullptr;
};
