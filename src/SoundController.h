/**
 * @file SoundController.h
 * @brief Controller for sound signals (horn) based on COLREGs
 * 
 * UI-agnostic design: Can be controlled from SignalK (main UI) or BLE (fallback UI).
 * Manages:
 * - Ad-hoc semi-automatic signals (one-shot blasts from UI command)
 * - Periodic signals (auto-repeating with countdown, mute/unmute from UI)
 */

#ifndef SOUND_CONTROLLER_H
#define SOUND_CONTROLLER_H

#include "interfaces/IRelayController.h"
#include "interfaces/ITimer.h"
#include "state_machine.h"
#include <functional>

// COLREGs blast durations (milliseconds)
constexpr uint32_t SHORT_BLAST_MS = 1000;      // ~1 second (Rule 32)
constexpr uint32_t PROLONGED_BLAST_MS = 5000;  // 4-6 seconds (Rule 32)
constexpr uint32_t PAUSE_MS = 1000;            // 1 second between blasts

/**
 * @enum AdHocSignal
 * @brief Semi-automatic ad-hoc signals triggered manually from UI
 */
enum class AdHocSignal : uint8_t {
    TURN_STARBOARD = 0,       // ● (Rule 34)
    TURN_PORT = 1,            // ●● (Rule 34)
    ASTERN_PROPULSION = 2,    // ●●● (Rule 34)
    DANGER_CONFUSION = 3,     // ●●●●● (Rule 34)
    PAY_ATTENTION = 4,        // ▬▬ (Rule 34)
    OVERTAKE_STARBOARD = 5,   // ▬▬ ▬▬ ● (Rule 34)
    OVERTAKE_PORT = 6,        // ▬▬ ▬▬ ●● (Rule 34)
    AGREEMENT_OVERTAKEN = 7   // ▬▬ ● ▬▬ ● (Rule 34)
};

/**
 * @class SoundController
 * @brief Manages horn relay for ad-hoc and periodic sound signals
 * 
 * Single Responsibility: Sound signal timing and horn control
 * Open/Closed: Easy to add new signal patterns
 * UI-agnostic: Commands from SignalK or BLE trigger same methods
 */
class SoundController {
public:
    /**
     * @brief Constructor with dependency injection
     * @param relay_controller Hardware abstraction for relay control
     * @param timer Timer abstraction for signal timing
     */
    SoundController(IRelayController& relay_controller, ITimer& timer);
    
    ~SoundController();

    /**
     * @brief Update controller (call from main loop)
     * Processes timer events, manages countdown
     */
    void update();

    /**
     * @brief Set periodic signal pattern from state machine
     * @param pattern Signal pattern required by current COLREGs state
     * @param interval_seconds Interval between signals (e.g., 120 for 2min)
     * 
     * Always starts muted (safety requirement).
     * UI must call unmute() to enable sound.
     */
    void setPeriodicSignal(SoundSignalPattern pattern, uint16_t interval_seconds);

    /**
     * @brief Mute periodic signals
     * Countdown continues, horn stays silent.
     * UI command: can come from SignalK or BLE.
     */
    void mutePeriodicSignals();

    /**
     * @brief Unmute periodic signals
     * Horn will sound on next countdown expiry.
     * UI command: can come from SignalK or BLE.
     */
    void unmutePeriodicSignals();

    /**
     * @brief Check if periodic signals are muted
     */
    bool isPeriodicMuted() const { return periodic_muted_; }

    /**
     * @brief Get seconds until next periodic signal
     * @return Countdown in seconds (for UI display)
     */
    uint16_t getPeriodicCountdownSeconds() const;

    /**
     * @brief Trigger ad-hoc semi-automatic signal
     * @param signal Type of signal to emit (one-shot)
     * 
     * UI command: operator selects from interface (SignalK or BLE).
     * Signal plays once immediately, regardless of mute state.
     */
    void triggerAdHocSignal(AdHocSignal signal);

    /**
     * @brief Stop all sound immediately (emergency/safety)
     */
    void stopAllSound();

    /**
     * @brief Check if horn is currently sounding
     */
    bool isHornActive() const;

private:
    IRelayController& relay_controller_;
    ITimer& timer_;

    // Periodic signal state
    SoundSignalPattern current_periodic_pattern_;
    uint16_t periodic_interval_seconds_;
    bool periodic_muted_;
    uint32_t periodic_timer_id_;
    uint32_t last_periodic_start_ms_;

    // Signal playback state
    bool signal_in_progress_;
    
    // Horn control helpers
    void startHorn();
    void stopHorn();
    
    // Signal pattern playback
    void playPeriodicSignal();
    void playAdHocPattern(AdHocSignal signal);
    void playShortBlast();
    void playProlongedBlast();
    
    // Timing helpers
    void scheduleNextPeriodicSignal();
    void onPeriodicTimerExpired();
};

#endif // SOUND_CONTROLLER_H
