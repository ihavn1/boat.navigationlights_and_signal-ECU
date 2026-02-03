/**
 * @file MockNavigationLightsECU.h
 * @brief Mock NavigationLightsECU for SignalK integration testing
 */

#ifndef MOCK_NAVIGATION_LIGHTS_ECU_H
#define MOCK_NAVIGATION_LIGHTS_ECU_H

#include "../../src/state_machine.h"
#include "../../src/NavigationLightsECU.h"
#include <functional>

/**
 * @class MockNavigationLightsECU
 * @brief Mock implementation of NavigationLightsECU for testing SignalK integration
 */
class MockNavigationLightsECU {
public:
    // Recorded method calls for verification
    Condition last_condition_set = Condition::DAY;
    BoatState last_state_set = BoatState::MOORED;
    AdHocSignal last_adhoc_signal = AdHocSignal::TURN_STARBOARD;
    bool mute_called = false;
    bool unmute_called = false;
    bool emergency_stop_called = false;
    
    // Current state for getters
    Condition current_condition = Condition::DAY;
    BoatState current_state = BoatState::MOORED;
    bool periodic_muted = false;
    unsigned long periodic_countdown = 120;
    LightConfiguration current_lights;
    bool horn_active = false;
    
    // State change callback
    std::function<void()> state_change_callback;
    
    MockNavigationLightsECU() {
        current_lights.masthead_light = false;
        current_lights.port_sidelight = false;
        current_lights.starboard_sidelight = false;
        current_lights.sternlight = false;
        current_lights.allround_white = false;
        current_lights.allround_red_upper = false;
        current_lights.allround_red_lower = false;
    }
    
    // Mock methods
    void setCondition(Condition condition) {
        last_condition_set = condition;
        current_condition = condition;
    }
    
    void setBoatState(BoatState state) {
        last_state_set = state;
        current_state = state;
    }
    
    void mutePeriodicSignals() {
        mute_called = true;
        periodic_muted = true;
    }
    
    void unmutePeriodicSignals() {
        unmute_called = true;
        periodic_muted = false;
    }
    
    void triggerAdHocSignal(AdHocSignal signal) {
        last_adhoc_signal = signal;
    }
    
    void emergencyStop() {
        emergency_stop_called = true;
    }
    
    void onStateChange(std::function<void()> callback) {
        state_change_callback = callback;
    }
    
    // Getters
    Condition getCondition() const { return current_condition; }
    BoatState getBoatState() const { return current_state; }
    bool isPeriodicMuted() const { return periodic_muted; }
    unsigned long getPeriodicCountdownSeconds() const { return periodic_countdown; }
    LightConfiguration getCurrentLights() const { return current_lights; }
    bool isHornActive() const { return horn_active; }
    
    // Test helper to trigger state change callback
    void triggerStateChange() {
        if (state_change_callback) {
            state_change_callback();
        }
    }
    
    // Reset mock state
    void reset() {
        last_condition_set = Condition::DAY;
        last_state_set = BoatState::MOORED;
        last_adhoc_signal = AdHocSignal::TURN_STARBOARD;
        mute_called = false;
        unmute_called = false;
        emergency_stop_called = false;
        state_change_callback = nullptr;
    }
};

#endif // MOCK_NAVIGATION_LIGHTS_ECU_H
