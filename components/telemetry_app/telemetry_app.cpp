#include "telemetry_app.hpp"
#include "ui_engine.hpp"
#include "msg_bus.hpp"

void TelemetryApp::start() {
    UIEngine::get_instance().lock_ui();
    label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Telemetry Ready");
    lv_obj_center(label);
    UIEngine::get_instance().unlock_ui();
    
    xTaskCreatePinnedToCore(TelemetryApp::consumer_task, "tel_worker", 4096, this, 5, &task_h, 1);
}

void TelemetryApp::stop() {
    if (task_h) { vTaskDelete(task_h); task_h = nullptr; }
    UIEngine::get_instance().lock_ui();
    if (label) { lv_obj_delete(label); label = nullptr; }
    UIEngine::get_instance().unlock_ui();
}

void TelemetryApp::consumer_task(void* p) {
    TelemetryApp* app = static_cast<TelemetryApp*>(p);
    SystemMessage msg;
    while (true) {
        if (MsgBus::get_instance().receive(msg, portMAX_DELAY)) {
            UIEngine::get_instance().lock_ui();
            if (app->label) {
                lv_label_set_text_fmt(app->label, "Val: %.2f", msg.float_val);
            }
            UIEngine::get_instance().unlock_ui();
        }
    }
}
