/**
 * @file state_machine.cpp
 * @brief Implementation of COLREGs state machine
 * 
 * Implements COLREGs Rules 20, 23, 25, 27, 30, 35 for vessels <15m
 */

#include "state_machine.h"

// =============================================================================
// CONSTRUCTOR
// =============================================================================

StateMachine::StateMachine()
    : current_condition_(Condition::DAY),
      current_boat_state_(BoatState::MOORED) {
    // Default to safest state: Day, Moored (no lights/signals required)
}

// =============================================================================
// STATE SETTERS
// =============================================================================

void StateMachine::setCondition(Condition condition) {
    current_condition_ = condition;
}

void StateMachine::setBoatState(BoatState state) {
    current_boat_state_ = state;
}

// =============================================================================
// LIGHT CONFIGURATION (COLREGs Rules 20, 23, 25, 27, 30)
// =============================================================================

LightConfiguration StateMachine::getRequiredLights() const {
    switch (current_condition_) {
        case Condition::DAY:
            return computeLightsForDay();
        case Condition::HOURS_OF_DARKNESS:
            return computeLightsForDarkness();
        case Condition::RESTRICTED_VISIBILITY:
            return computeLightsForRestrictedVisibility();
        default:
            return LightConfiguration(); // All lights off
    }
}

LightConfiguration StateMachine::computeLightsForDay() const {
    // Rule 20: Lights not required during daylight
    // (Day shapes like black balls are visual signals, not lights)
    return LightConfiguration(); // All lights off
}

LightConfiguration StateMachine::computeLightsForDarkness() const {
    LightConfiguration config;
    
    switch (current_boat_state_) {
        case BoatState::MOORED:
            // Moored vessel: no lights required
            break;
            
        case BoatState::UNDERWAY_NO_WAY:
        case BoatState::UNDERWAY_MAKING_WAY:
            // Rule 23: Power-driven vessel <50m underway
            config.masthead_light = true;
            config.port_sidelight = true;
            config.starboard_sidelight = true;
            config.sternlight = true;
            break;
            
        case BoatState::ANCHORAGE:
            // Rule 30: Anchored vessel <50m
            config.allround_white = true;
            break;
            
        case BoatState::NUC_NO_WAY:
            // Rule 27(a): NUC vessel not making way
            config.allround_red_upper = true;
            config.allround_red_lower = true;
            break;
            
        case BoatState::NUC_MAKING_WAY:
            // Rule 27(b): NUC vessel making way
            config.allround_red_upper = true;
            config.allround_red_lower = true;
            config.port_sidelight = true;
            config.starboard_sidelight = true;
            config.sternlight = true;
            // Note: No masthead light when NUC
            break;
            
        case BoatState::TOWING:
            // Rule 24: Vessel towing shows normal underway lights + yellow towing light
            config.masthead_light = true;
            config.port_sidelight = true;
            config.starboard_sidelight = true;
            config.sternlight = true;
            config.yellow_towing_light = true;
            break;
    }
    
    return config;
}

LightConfiguration StateMachine::computeLightsForRestrictedVisibility() const {
    LightConfiguration config = computeLightsForDarkness();
    
    // In restricted visibility, NUC vessels show sidelights/sternlight regardless of making way
    // (Per COLREGs table in proposal - enhanced visibility needed in fog)
    if (current_boat_state_ == BoatState::NUC_NO_WAY) {
        config.port_sidelight = true;
        config.starboard_sidelight = true;
        config.sternlight = true;
    }
    
    return config;
}

// =============================================================================
// SOUND SIGNALS (COLREGs Rule 35)
// =============================================================================

SoundSignalPattern StateMachine::getPeriodicSoundSignal() const {
    if (current_condition_ != Condition::RESTRICTED_VISIBILITY) {
        return SoundSignalPattern::NONE;
    }
    
    return computeSignalForRestrictedVisibility();
}

SoundSignalPattern StateMachine::computeSignalForRestrictedVisibility() const {
    switch (current_boat_state_) {
        case BoatState::MOORED:
            // Moored vessel: no sound signal required
            return SoundSignalPattern::NONE;
            
        case BoatState::UNDERWAY_NO_WAY:
            // Rule 35(b): Power-driven vessel underway but stopped (making no way) - 2 prolonged blasts every 2min
            return SoundSignalPattern::PROLONGED_PROLONGED_2MIN;
            
        case BoatState::UNDERWAY_MAKING_WAY:
            // Rule 35(a): Power-driven vessel making way through the water - 1 prolonged blast every 2min
            return SoundSignalPattern::PROLONGED_2MIN;
            
        case BoatState::ANCHORAGE:
            // Rule 35(g): Anchored vessel - rapid bell ringing
            // Simplified to short-prolonged-short pattern (warning signal)
            return SoundSignalPattern::SHORT_PROLONGED_SHORT;
            
        case BoatState::NUC_NO_WAY:
        case BoatState::NUC_MAKING_WAY:
            // Rule 35(c): NUC vessel - 1 prolonged + 2 short blasts every 2min
            return SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN;
            
        case BoatState::TOWING:
            // Rule 35(c): Vessel towing - 1 prolonged + 2 short blasts every 2min (same as NUC)
            return SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN;
    }
    
    return SoundSignalPattern::NONE;
}

uint16_t StateMachine::getPeriodicSignalIntervalSeconds() const {
    SoundSignalPattern pattern = getPeriodicSoundSignal();
    
    switch (pattern) {
        case SoundSignalPattern::PROLONGED_2MIN:
        case SoundSignalPattern::PROLONGED_PROLONGED_2MIN:
        case SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN:
            return 120; // 2 minutes
            
        case SoundSignalPattern::SHORT_PROLONGED_SHORT:
            // Anchorage warning signal interval (implementation specific)
            return 60; // 1 minute for warning signal
            
        case SoundSignalPattern::NONE:
        default:
            return 0;
    }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

std::string StateMachine::getStateDescription() const {
    std::string desc = "Condition: ";
    desc += conditionToString(current_condition_);
    desc += " | Boat State: ";
    desc += boatStateToString(current_boat_state_);
    return desc;
}

const char* conditionToString(Condition condition) {
    switch (condition) {
        case Condition::DAY: return "Day";
        case Condition::HOURS_OF_DARKNESS: return "Hours of Darkness";
        case Condition::RESTRICTED_VISIBILITY: return "Restricted Visibility";
        default: return "Unknown";
    }
}

const char* boatStateToString(BoatState state) {
    switch (state) {
        case BoatState::MOORED: return "Moored";
        case BoatState::UNDERWAY_NO_WAY: return "Underway (No Way)";
        case BoatState::UNDERWAY_MAKING_WAY: return "Underway (Making Way)";
        case BoatState::ANCHORAGE: return "Anchorage";
        case BoatState::NUC_NO_WAY: return "NUC (No Way)";
        case BoatState::NUC_MAKING_WAY: return "NUC (Making Way)";
        case BoatState::TOWING: return "Towing";
        default: return "Unknown";
    }
}

const char* soundSignalPatternToString(SoundSignalPattern pattern) {
    switch (pattern) {
        case SoundSignalPattern::NONE: return "None";
        case SoundSignalPattern::PROLONGED_2MIN: return "▬▬ / 2min";
        case SoundSignalPattern::PROLONGED_PROLONGED_2MIN: return "▬▬ ▬▬ / 2min";
        case SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN: return "▬▬ ● ● / 2min";
        case SoundSignalPattern::SHORT_PROLONGED_SHORT: return "● ▬▬ ●";
        default: return "Unknown";
    }
}
