/**
 * @file NavigationLightsECU.h
 * @brief Main ECU controller integrating state machine, lights, sounds, and SignalK
 * 
 * UI-Agnostic Architecture:
 * - Main UI: SignalK (WiFi) for normal operation
 * - Fallback UI: Web-based control (future) when SignalK unavailable
 * Both UIs control the same underlying controllers via this facade.
 */

#ifndef NAVIGATION_LIGHTS_ECU_H
#define NAVIGATION_LIGHTS_ECU_H

#include "state_machine.h"
#include "LightController.h"
#include "SoundController.h"
#include <functional>

/**
 * @class NavigationLightsECU
 * @brief Facade coordinating state machine, light controller, and sound controller
 * 
 * Single Responsibility: Coordinate components and expose unified interface
 * UI-agnostic: Methods can be called from SignalK handlers or web UI handlers
 */
class NavigationLightsECU {
public:
    /**
     * @brief Constructor with dependency injection
     */
    NavigationLightsECU(
        LightController& light_controller,
        SoundController& sound_controller
    );

    ~NavigationLightsECU() = default;

    /**
     * @brief Update ECU (call from main loop)
     * Processes timers, updates controllers
     */
    void update();

    // =======================================================================
    // CONDITION & STATE CONTROL (from UI: SignalK or web UI)
    // =======================================================================

    /**
     * @brief Set environmental condition
     * UI command: operator selects from interface
     */
    void setCondition(Condition condition);

    /**
     * @brief Set boat operational state
     * UI command: operator selects from interface
     */
    void setBoatState(BoatState state);

    /**
     * @brief Get current condition
     */
    Condition getCondition() const { return state_machine_.getCondition(); }

    /**
     * @brief Get current boat state
     */
    BoatState getBoatState() const { return state_machine_.getBoatState(); }

    /**
     * @brief Get human-readable state description
     */
    std::string getStateDescription() const { return state_machine_.getStateDescription(); }

    // =======================================================================
    // SOUND SIGNAL CONTROL (from UI: SignalK or web UI)
    // =======================================================================

    /**
     * @brief Mute periodic sound signals
     * UI command: mute button pressed
     */
    void mutePeriodicSignals();

    /**
     * @brief Unmute periodic sound signals
     * UI command: unmute button pressed
     */
    void unmutePeriodicSignals();

    /**
     * @brief Check if periodic signals are muted
     */
    bool isPeriodicMuted() const;

    /**
     * @brief Get countdown to next periodic signal (seconds)
     * For UI display: shows time remaining
     */
    uint16_t getPeriodicCountdownSeconds() const;

    /**
     * @brief Trigger ad-hoc semi-automatic signal
     * UI command: operator selects signal type from interface
     */
    void triggerAdHocSignal(AdHocSignal signal);

    /**
     * @brief Emergency stop all lights and sounds
     */
    void emergencyStop();

    // =======================================================================
    // STATUS QUERY (for UI updates: SignalK or web UI)
    // =======================================================================

    /**
     * @brief Get current light configuration
     */
    LightConfiguration getCurrentLights() const;

    /**
     * @brief Get current periodic sound pattern
     */
    SoundSignalPattern getCurrentPeriodicPattern() const;

    /**
     * @brief Check if any lights are active
     */
    bool anyLightsActive() const;

    /**
     * @brief Check if horn is currently active
     */
    bool isHornActive() const;

    // =======================================================================
    // CALLBACKS (for SignalK publishing)
    // =======================================================================

    /**
     * @brief Set callback for when state changes
     * SignalK integration: publish updates when state changes
     */
    void onStateChange(std::function<void()> callback) {
        state_change_callback_ = callback;
    }

private:
    StateMachine state_machine_;
    LightController& light_controller_;
    SoundController& sound_controller_;

    std::function<void()> state_change_callback_;
    LightConfiguration base_lights_;
    bool sos_active_;
    bool horn_active_;

    // Apply current state to controllers
    void applyState();

    void applyLightsWithSos();
    void onHornStateChanged(bool horn_active);
    void onAdHocSignalStateChanged(AdHocSignal signal, bool active);
    bool shouldFlashSosLights() const;
};

#endif // NAVIGATION_LIGHTS_ECU_H
