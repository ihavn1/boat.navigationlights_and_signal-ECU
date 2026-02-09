/**
 * @file LightController.h
 * @brief Controller for navigation lights based on state machine
 * 
 * UI-agnostic design: Accepts light configuration from any source
 * (SignalK main UI or web fallback UI) and applies it to relay hardware.
 */

#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include "interfaces/IRelayController.h"
#include "state_machine.h"

/**
 * @class LightController
 * @brief Manages navigation light relay states based on COLREGs configuration
 * 
 * Single Responsibility: Translates LightConfiguration to relay commands
 * Dependency Inversion: Depends on IRelayController abstraction
 * 
 * Can be controlled from:
 * - SignalK (main UI): Updates state machine, controller applies changes
 * - Web UI (fallback): Same interface, different source
 */
class LightController {
public:
    /**
     * @brief Constructor with dependency injection
     * @param relay_controller Hardware abstraction for relay control
     */
    explicit LightController(IRelayController& relay_controller);
    
    ~LightController() = default;

    /**
     * @brief Apply light configuration to hardware
     * @param config Desired light configuration from state machine
     * 
     * Updates only changed relays for efficiency.
     * UI-agnostic: config can come from SignalK or web UI.
     */
    void applyConfiguration(const LightConfiguration& config);

    /**
     * @brief Get current applied configuration
     * @return Currently active light configuration
     */
    LightConfiguration getCurrentConfiguration() const { return current_config_; }

    /**
     * @brief Turn off all lights immediately (emergency/safety function)
     */
    void allLightsOff();

    /**
     * @brief Check if any lights are currently active
     * @return true if any navigation light is on
     */
    bool anyLightsActive() const;

private:
    IRelayController& relay_controller_;
    LightConfiguration current_config_;

    // Helper to update a single relay only if state changed
    void updateRelay(RelayChannel channel, bool should_be_active, bool currently_active);
};

#endif // LIGHT_CONTROLLER_H
