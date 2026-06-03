#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class HWLocks {
public:
    static HWLocks& get_instance();
    void lock_i2c0_bus();
    void unlock_i2c0_bus();

    class I2C0Guard {
    public:
        I2C0Guard() { HWLocks::get_instance().lock_i2c0_bus(); }
        ~I2C0Guard() { HWLocks::get_instance().unlock_i2c0_bus(); }
        I2C0Guard(const I2C0Guard&) = delete;
        I2C0Guard& operator=(const I2C0Guard&) = delete;
    };

private:
    HWLocks();
    SemaphoreHandle_t i2c0_mutex = nullptr;
};
