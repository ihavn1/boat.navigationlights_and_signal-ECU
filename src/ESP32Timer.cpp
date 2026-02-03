/**
 * @file ESP32Timer.cpp
 * @brief Implementation of ESP32 timer
 */

#include "ESP32Timer.h"

ESP32Timer::ESP32Timer() : next_id_(1) {}

uint32_t ESP32Timer::millis() const {
    return ::millis(); // Arduino millis()
}

uint32_t ESP32Timer::scheduleOnce(uint32_t delayMs, std::function<void()> callback) {
    uint32_t id = next_id_++;
    uint32_t trigger_time = ::millis() + delayMs;
    callbacks_.push_back({id, trigger_time, 0, callback, true});
    return id;
}

uint32_t ESP32Timer::scheduleRepeating(uint32_t intervalMs, std::function<void()> callback) {
    uint32_t id = next_id_++;
    uint32_t trigger_time = ::millis() + intervalMs;
    callbacks_.push_back({id, trigger_time, intervalMs, callback, true});
    return id;
}

bool ESP32Timer::cancel(uint32_t timerId) {
    for (auto& cb : callbacks_) {
        if (cb.id == timerId && cb.active) {
            cb.active = false;
            return true;
        }
    }
    return false;
}

void ESP32Timer::update() {
    uint32_t now = ::millis();
    
    // Process expired timers
    for (auto& cb : callbacks_) {
        if (cb.active && cb.trigger_time <= now) {
            cb.callback();
            
            if (cb.interval > 0) {
                // Repeating timer - reschedule
                cb.trigger_time = now + cb.interval;
            } else {
                // One-shot timer - deactivate
                cb.active = false;
            }
        }
    }
    
    // Clean up inactive timers periodically to prevent memory growth
    static uint32_t last_cleanup = 0;
    if (now - last_cleanup > 60000) { // Every 60 seconds
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [](const ESP32TimerCallback& cb) { return !cb.active; }),
            callbacks_.end()
        );
        last_cleanup = now;
    }
}
