/**
 * @file ITimer.h
 * @brief Hardware abstraction interface for timing operations
 * 
 * Defines interface for managing sound signal timing (short/prolonged blasts,
 * periodic intervals, countdown tracking).
 */

#ifndef I_TIMER_H
#define I_TIMER_H

#include <cstdint>
#include <functional>

/**
 * @interface ITimer
 * @brief Abstract interface for timer/timing operations
 * 
 * Enables testing of time-dependent logic without real delays.
 * Used for sound signal durations, periodic signal intervals, and countdown.
 */
class ITimer {
public:
    virtual ~ITimer() = default;

    /**
     * @brief Get current time in milliseconds
     * @return Milliseconds since system start (or epoch for tests)
     */
    virtual uint32_t millis() const = 0;

    /**
     * @brief Schedule a one-shot callback after specified delay
     * @param delayMs Delay in milliseconds before callback fires
     * @param callback Function to call when timer expires
     * @return Timer ID (for cancellation) or 0 if scheduling failed
     */
    virtual uint32_t scheduleOnce(uint32_t delayMs, std::function<void()> callback) = 0;

    /**
     * @brief Schedule a repeating callback with specified interval
     * @param intervalMs Interval in milliseconds between callbacks
     * @param callback Function to call each interval
     * @return Timer ID (for cancellation) or 0 if scheduling failed
     */
    virtual uint32_t scheduleRepeating(uint32_t intervalMs, std::function<void()> callback) = 0;

    /**
     * @brief Cancel a scheduled timer
     * @param timerId ID returned from scheduleOnce or scheduleRepeating
     * @return true if timer was found and cancelled, false otherwise
     */
    virtual bool cancel(uint32_t timerId) = 0;

    /**
     * @brief Process/update timers (called from main loop)
     * 
     * Implementations should check for expired timers and invoke callbacks.
     * Mock implementations can advance time programmatically.
     */
    virtual void update() = 0;
};

#endif // I_TIMER_H
