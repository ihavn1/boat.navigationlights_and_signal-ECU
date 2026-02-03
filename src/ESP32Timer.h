/**
 * @file ESP32Timer.h
 * @brief ESP32 implementation of ITimer using FreeRTOS
 * 
 * Uses Arduino/FreeRTOS timer functions for sound signal timing.
 */

#ifndef ESP32_TIMER_H
#define ESP32_TIMER_H

#include "interfaces/ITimer.h"
#include <Arduino.h>
#include <vector>

struct ESP32TimerCallback {
    uint32_t id;
    uint32_t trigger_time;
    uint32_t interval; // 0 for one-shot
    std::function<void()> callback;
    bool active;
};

/**
 * @class ESP32Timer
 * @brief Concrete implementation of ITimer for ESP32
 * 
 * Manages callbacks using millis() for timing.
 * Call update() from main loop to process expired timers.
 */
class ESP32Timer : public ITimer {
public:
    ESP32Timer();
    ~ESP32Timer() override = default;

    uint32_t millis() const override;
    uint32_t scheduleOnce(uint32_t delayMs, std::function<void()> callback) override;
    uint32_t scheduleRepeating(uint32_t intervalMs, std::function<void()> callback) override;
    bool cancel(uint32_t timerId) override;
    void update() override;

private:
    uint32_t next_id_;
    std::vector<ESP32TimerCallback> callbacks_;
};

#endif // ESP32_TIMER_H
