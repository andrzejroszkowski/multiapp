#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui_engine.hpp"
#include "msg_bus.hpp"
#include "hw_locks.hpp"
#include "error_hub.hpp"
#include "telemetry_app.hpp"

void background_i2c_sensor_task(void* pvParameters) {
    while (true) {
        {
            HWLocks::I2C0Guard lock;
            bool sensor_acknowledged = false; 
            if (!sensor_acknowledged) {
                ErrorHub::get_instance().report(
                    ErrorSeverity::WARNING, 
                    "SHT31_Sensor", 
                    "I2C read acknowledgement failed timed out", 
                    0xEA01
                );
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

extern "C" void app_main(void) {
    ErrorHub::get_instance().start_supervisor();
    ErrorHub::get_instance().report(ErrorSeverity::NOTICE, "System", "Framework Engine Initialized", 0x0000);
    
    UIEngine::get_instance().init();
    xTaskCreatePinnedToCore(background_i2c_sensor_task, "i2c_worker", 3072, nullptr, 3, nullptr, 0);

    std::unique_ptr<AppInterface> active_app = std::make_unique<TelemetryApp>();
    active_app->start();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
