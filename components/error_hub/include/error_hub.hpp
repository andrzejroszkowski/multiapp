#pragma once
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

enum class ErrorSeverity { NOTICE, WARNING, CRITICAL };

struct ErrorMessage {
    ErrorSeverity severity;
    char module_name[16];
    char description[64];
    uint32_t error_code;
};

class ErrorHub {
public:
    static ErrorHub& get_instance();
    void report(ErrorSeverity severity, const std::string& module, const std::string& desc, uint32_t code);
    void start_supervisor();

private:
    ErrorHub();
    static void supervisor_task(void* pvParameters);
    QueueHandle_t error_queue = nullptr;
};
