/**
 * @file LightController.cpp
 * @brief Implementation of navigation light controller
 */

#include "LightController.h"

LightController::LightController(IRelayController& relay_controller)
    : relay_controller_(relay_controller), current_config_() {
    // Start with all lights off
}

void LightController::applyConfiguration(const LightConfiguration& config) {
    // Update each light relay only if state changed (efficiency)
    updateRelay(RelayChannel::MASTHEAD_LIGHT, config.masthead_light, current_config_.masthead_light);
    updateRelay(RelayChannel::PORT_SIDELIGHT, config.port_sidelight, current_config_.port_sidelight);
    updateRelay(RelayChannel::STARBOARD_SIDELIGHT, config.starboard_sidelight, current_config_.starboard_sidelight);
    updateRelay(RelayChannel::STERNLIGHT, config.sternlight, current_config_.sternlight);
    updateRelay(RelayChannel::ALLROUND_WHITE, config.allround_white, current_config_.allround_white);
    updateRelay(RelayChannel::ALLROUND_RED_UPPER, config.allround_red_upper, current_config_.allround_red_upper);
    updateRelay(RelayChannel::ALLROUND_RED_LOWER, config.allround_red_lower, current_config_.allround_red_lower);
    updateRelay(RelayChannel::YELLOW_TOWING_LIGHT, config.yellow_towing_light, current_config_.yellow_towing_light);
    
    // Store new configuration
    current_config_ = config;
}

void LightController::allLightsOff() {
    LightConfiguration off_config; // All lights false by default
    applyConfiguration(off_config);
}

bool LightController::anyLightsActive() const {
    return current_config_.masthead_light ||
           current_config_.port_sidelight ||
           current_config_.starboard_sidelight ||
           current_config_.sternlight ||
           current_config_.allround_white ||
           current_config_.allround_red_upper ||
           current_config_.allround_red_lower ||
           current_config_.yellow_towing_light;
}

void LightController::updateRelay(RelayChannel channel, bool should_be_active, bool currently_active) {
    if (should_be_active == currently_active) {
        return; // No change needed
    }
    
    if (should_be_active) {
        relay_controller_.activate(channel);
    } else {
        relay_controller_.deactivate(channel);
    }
}
