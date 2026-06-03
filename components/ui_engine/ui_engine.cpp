#include "ui_engine.hpp"

static portMUX_TYPE lvgl_mux = portMUX_INITIALIZER_UNLOCKED;

UIEngine& UIEngine::get_instance() {
    static UIEngine instance;
    return instance;
}

void UIEngine::lock_ui() { taskENTER_CRITICAL(&lvgl_mux); }
void UIEngine::unlock_ui() { taskEXIT_CRITICAL(&lvgl_mux); }

void UIEngine::init() {
    lv_init();
    xTaskCreatePinnedToCore(UIEngine::lvgl_task, "lvgl_loop", 4096, nullptr, 4, &lv_task_handle, 1);
}

void UIEngine::lvgl_task(void* pvParameters) {
    while (true) {
        UIEngine::get_instance().lock_ui();
        uint32_t task_delay = lv_timer_handler();
        UIEngine::get_instance().unlock_ui();
        
        vTaskDelay(pdMS_TO_TICKS(task_delay < 5 ? 5 : task_delay));
    }
}
