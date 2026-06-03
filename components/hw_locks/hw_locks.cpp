#include "hw_locks.hpp"

HWLocks& HWLocks::get_instance() {
    static HWLocks instance;
    return instance;
}

HWLocks::HWLocks() {
    i2c0_mutex = xSemaphoreCreateMutex();
}

void HWLocks::lock_i2c0_bus() {
    xSemaphoreTake(i2c0_mutex, portMAX_DELAY);
}

void HWLocks::unlock_i2c0_bus() {
    xSemaphoreGive(i2c0_mutex);
}
