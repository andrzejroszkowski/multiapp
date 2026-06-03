#include "error_hub.hpp"
#include "esp_log.h"
#include <cstdio>

ErrorHub& ErrorHub::get_instance() {
    static ErrorHub instance;
    return instance;
}

ErrorHub::ErrorHub() {
    error_queue = xQueueCreate(10, sizeof(ErrorMessage));
}

void ErrorHub::report(ErrorSeverity severity, const std::string& module, const std::string& desc, uint32_t code) {
    ErrorMessage msg;
    msg.severity = severity;
    msg.error_code = code;
    snprintf(msg.module_name, sizeof(msg.module_name), "%s", module.c_str());
    snprintf(msg.description, sizeof(msg.description), "%s", desc.c_str());
    xQueueSend(error_queue, &msg, 0);
}

void ErrorHub::start_supervisor() {
    xTaskCreatePinnedToCore(ErrorHub::supervisor_task, "err_sup", 3072, this, 6, nullptr, 0);
}

void ErrorHub::supervisor_task(void* pvParameters) {
    ErrorHub* hub = static_cast<ErrorHub*>(pvParameters);
    ErrorMessage msg;
    while (true) {
        if (xQueueReceive(hub->error_queue, &msg, portMAX_DELAY)) {
            ESP_LOGW("ErrorHub", "[%s] Code %lu: %s", msg.module_name, msg.error_code, msg.description);
        }
    }
}
