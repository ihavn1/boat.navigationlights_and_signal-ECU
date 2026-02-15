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
      state_change_callback_(nullptr),
      base_lights_(),
      sos_active_(false),
      horn_active_(false) {

    sound_controller_.setHornStateCallback(
        [this](bool horn_active) { this->onHornStateChanged(horn_active); });
    sound_controller_.setAdHocSignalCallback(
        [this](AdHocSignal signal, bool active) { this->onAdHocSignalStateChanged(signal, active); });
    
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
    sos_active_ = false;
    horn_active_ = false;
    
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

bool NavigationLightsECU::isSosActive() const {
    return sos_active_;
}

void NavigationLightsECU::applyState() {
    // Get required configuration from state machine
    base_lights_ = state_machine_.getRequiredLights();
    SoundSignalPattern sound = state_machine_.getPeriodicSoundSignal();
    uint16_t interval = state_machine_.getPeriodicSignalIntervalSeconds();
    
    // Apply to controllers
    applyLightsWithSos();
    sound_controller_.setPeriodicSignal(sound, interval);
}

void NavigationLightsECU::applyLightsWithSos() {
    LightConfiguration effective = base_lights_;
    if (shouldFlashSosLights()) {
        effective.masthead_light = true;
        effective.allround_white = true;
    }
    light_controller_.applyConfiguration(effective);
}

void NavigationLightsECU::onHornStateChanged(bool horn_active) {
    horn_active_ = horn_active;
    if (sos_active_) {
        applyLightsWithSos();
    }
}

void NavigationLightsECU::onAdHocSignalStateChanged(AdHocSignal signal, bool active) {
    if (signal != AdHocSignal::SOS) {
        return;
    }

    sos_active_ = active;
    applyLightsWithSos();
}

bool NavigationLightsECU::shouldFlashSosLights() const {
    if (!sos_active_ || !horn_active_) {
        return false;
    }

    Condition condition = state_machine_.getCondition();
    return condition == Condition::HOURS_OF_DARKNESS ||
           condition == Condition::RESTRICTED_VISIBILITY;
}
