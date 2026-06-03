#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

class UIEngine {
public:
    static UIEngine& get_instance();
    void init();
    void lock_ui();
    void unlock_ui();

private:
    UIEngine() = default;
    static void lvgl_task(void* pvParameters);
    TaskHandle_t lv_task_handle = nullptr;
};
