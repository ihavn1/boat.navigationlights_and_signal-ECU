/**
 * @file NavigationLightsECU.cpp
 * @brief Implementation of main ECU controller
 */

#include "NavigationLightsECU.h"

NavigationLightsECU::NavigationLightsECU(
    LightController& light_controller,
    SoundController& sound_controller
)
    : state_machine_(),
      light_controller_(light_controller),
      sound_controller_(sound_controller),
      state_change_callback_(nullptr) {
    
    // Apply initial state
    applyState();
}

void NavigationLightsECU::update() {
    sound_controller_.update();
}

void NavigationLightsECU::setCondition(Condition condition) {
    state_machine_.setCondition(condition);
    applyState();
    
    if (state_change_callback_) {
        state_change_callback_();
    }
}

void NavigationLightsECU::setBoatState(BoatState state) {
    state_machine_.setBoatState(state);
    applyState();
    
    if (state_change_callback_) {
        state_change_callback_();
    }
}

void NavigationLightsECU::mutePeriodicSignals() {
    sound_controller_.mutePeriodicSignals();
    
    if (state_change_callback_) {
        state_change_callback_();
    }
}

void NavigationLightsECU::unmutePeriodicSignals() {
    sound_controller_.unmutePeriodicSignals();
    
    if (state_change_callback_) {
        state_change_callback_();
    }
}

bool NavigationLightsECU::isPeriodicMuted() const {
    return sound_controller_.isPeriodicMuted();
}

uint16_t NavigationLightsECU::getPeriodicCountdownSeconds() const {
    return sound_controller_.getPeriodicCountdownSeconds();
}

void NavigationLightsECU::triggerAdHocSignal(AdHocSignal signal) {
    sound_controller_.triggerAdHocSignal(signal);
}

void NavigationLightsECU::emergencyStop() {
    light_controller_.allLightsOff();
    sound_controller_.stopAllSound();
    
    if (state_change_callback_) {
        state_change_callback_();
    }
}

LightConfiguration NavigationLightsECU::getCurrentLights() const {
    return light_controller_.getCurrentConfiguration();
}

SoundSignalPattern NavigationLightsECU::getCurrentPeriodicPattern() const {
    return state_machine_.getPeriodicSoundSignal();
}

bool NavigationLightsECU::anyLightsActive() const {
    return light_controller_.anyLightsActive();
}

bool NavigationLightsECU::isHornActive() const {
    return sound_controller_.isHornActive();
}

void NavigationLightsECU::applyState() {
    // Get required configuration from state machine
    LightConfiguration lights = state_machine_.getRequiredLights();
    SoundSignalPattern sound = state_machine_.getPeriodicSoundSignal();
    uint16_t interval = state_machine_.getPeriodicSignalIntervalSeconds();
    
    // Apply to controllers
    light_controller_.applyConfiguration(lights);
    sound_controller_.setPeriodicSignal(sound, interval);
}
