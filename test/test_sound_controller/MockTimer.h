/**
 * @file MockTimer.h
 * @brief Mock implementation of ITimer for unit testing
 */

#ifndef MOCK_TIMER_H
#define MOCK_TIMER_H

#include "../src/interfaces/ITimer.h"
#include <vector>
#include <algorithm>

struct ScheduledCallback {
    uint32_t id;
    uint32_t trigger_time;
    uint32_t interval; // 0 for one-shot
    std::function<void()> callback;
    bool active;
};

/**
 * @class MockTimer
 * @brief Test double for timer (controllable time advancement)
 */
class MockTimer : public ITimer {
public:
    MockTimer() : current_time_(0), next_id_(1) {}

    uint32_t millis() const override {
        return current_time_;
    }

    uint32_t scheduleOnce(uint32_t delayMs, std::function<void()> callback) override {
        uint32_t id = next_id_++;
        callbacks_.push_back({id, current_time_ + delayMs, 0, callback, true});
        return id;
    }

    uint32_t scheduleRepeating(uint32_t intervalMs, std::function<void()> callback) override {
        uint32_t id = next_id_++;
        callbacks_.push_back({id, current_time_ + intervalMs, intervalMs, callback, true});
        return id;
    }

    bool cancel(uint32_t timerId) override {
        for (auto& cb : callbacks_) {
            if (cb.id == timerId && cb.active) {
                cb.active = false;
                return true;
            }
        }
        return false;
    }

    void update() override {
        // Process expired timers
        for (auto& cb : callbacks_) {
            if (cb.active && cb.trigger_time <= current_time_) {
                cb.callback();
                
                if (cb.interval > 0) {
                    // Repeating timer - reschedule
                    cb.trigger_time = current_time_ + cb.interval;
                } else {
                    // One-shot timer - deactivate
                    cb.active = false;
                }
            }
        }
    }

    // Test helpers
    void advanceTime(uint32_t ms) {
        current_time_ += ms;
        update();
    }

    void setTime(uint32_t ms) {
        current_time_ = ms;
    }

private:
    uint32_t current_time_;
    uint32_t next_id_;
    std::vector<ScheduledCallback> callbacks_;
};

#endif // MOCK_TIMER_H
