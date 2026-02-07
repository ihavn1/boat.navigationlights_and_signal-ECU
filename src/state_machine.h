/**
 * @file state_machine.h
 * @brief COLREGs state machine for navigation lights and sound signals
 * 
 * Manages two-dimensional state: Condition × Boat State
 * Determines required lights and periodic sound signals per COLREGs rules
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <cstdint>
#include <string>

/**
 * @enum Condition
 * @brief Environmental/visibility conditions (COLREGs Rule 19, 20, 30)
 */
enum class Condition : uint8_t {
    DAY = 0,                  // Daylight hours
    HOURS_OF_DARKNESS = 1,    // Night, sunset to sunrise
    RESTRICTED_VISIBILITY = 2 // Fog, mist, heavy rain, etc.
};

/**
 * @enum BoatState
 * @brief Vessel operational state (COLREGs Rules 23, 25, 27, 30)
 */
enum class BoatState : uint8_t {
    MOORED = 0,                   // Tied to dock/pier
    UNDERWAY_NO_WAY = 1,          // Underway but not making way through water
    UNDERWAY_MAKING_WAY = 2,      // Underway and making way through water
    ANCHORAGE = 3,                // At anchor
    NUC_NO_WAY = 4,              // Not Under Command, not making way (Rule 27)
    NUC_MAKING_WAY = 5           // Not Under Command, making way (Rule 27)
};

/**
 * @struct LightConfiguration
 * @brief Specifies which navigation lights should be active
 */
struct LightConfiguration {
    bool masthead_light = false;       // White 225° forward light (Rule 21)
    bool port_sidelight = false;       // Red 112.5° port light (Rule 21)
    bool starboard_sidelight = false;  // Green 112.5° starboard light (Rule 21)
    bool sternlight = false;           // White 135° aft light (Rule 21)
    bool allround_white = false;       // 360° white light (anchorage, Rule 30)
    bool allround_red_upper = false;   // Upper red NUC light (Rule 27)
    bool allround_red_lower = false;   // Lower red NUC light (Rule 27)

    bool operator==(const LightConfiguration& other) const {
        return masthead_light == other.masthead_light &&
               port_sidelight == other.port_sidelight &&
               starboard_sidelight == other.starboard_sidelight &&
               sternlight == other.sternlight &&
               allround_white == other.allround_white &&
               allround_red_upper == other.allround_red_upper &&
               allround_red_lower == other.allround_red_lower;
    }
};

/**
 * @enum SoundSignalPattern
 * @brief Periodic sound signal patterns (COLREGs Rule 35)
 */
enum class SoundSignalPattern : uint8_t {
    NONE = 0,                          // No periodic signal
    PROLONGED_2MIN = 1,                // ▬▬ every 2 minutes (underway making no way, restricted visibility)
    PROLONGED_PROLONGED_2MIN = 2,      // ▬▬ ▬▬ every 2 minutes (underway making way, restricted visibility)
    PROLONGED_SHORT_SHORT_2MIN = 3,    // ▬▬ ● ● every 2 minutes (NUC, restricted visibility)
    SHORT_PROLONGED_SHORT = 4          // ● ▬▬ ● (anchorage warning, restricted visibility)
};

/**
 * @class StateMachine
 * @brief COLREGs-compliant state machine for maritime navigation
 * 
 * Single Responsibility: Manages condition/state and computes required lights/signals
 * Open/Closed: Extendable for additional COLREGs rules without modification
 */
class StateMachine {
public:
    StateMachine();
    ~StateMachine() = default;

    /**
     * @brief Set current environmental condition
     * @param condition Daylight, darkness, or restricted visibility
     */
    void setCondition(Condition condition);

    /**
     * @brief Set current boat operational state
     * @param state Moored, underway, anchorage, or NUC
     */
    void setBoatState(BoatState state);

    /**
     * @brief Get current condition
     */
    Condition getCondition() const { return current_condition_; }

    /**
     * @brief Get current boat state
     */
    BoatState getBoatState() const { return current_boat_state_; }

    /**
     * @brief Compute required light configuration for current state
     * @return Light configuration per COLREGs rules
     * 
     * Implements COLREGs Rules 20, 23, 25, 27, 30 for vessels <15m
     */
    LightConfiguration getRequiredLights() const;

    /**
     * @brief Compute required periodic sound signal pattern
     * @return Sound signal pattern per COLREGs Rule 35
     */
    SoundSignalPattern getPeriodicSoundSignal() const;

    /**
     * @brief Get interval in seconds for periodic sound signal
     * @return Interval in seconds (e.g., 120 for 2min), or 0 if no periodic signal
     */
    uint16_t getPeriodicSignalIntervalSeconds() const;

    /**
     * @brief Get human-readable description of current state
     */
    std::string getStateDescription() const;

private:
    Condition current_condition_;
    BoatState current_boat_state_;

    // Helper methods for computing light/signal configurations
    LightConfiguration computeLightsForDay() const;
    LightConfiguration computeLightsForDarkness() const;
    LightConfiguration computeLightsForRestrictedVisibility() const;
    
    SoundSignalPattern computeSignalForRestrictedVisibility() const;
};

// Utility functions for string conversion (useful for debugging/logging)
const char* conditionToString(Condition condition);
const char* boatStateToString(BoatState state);
const char* soundSignalPatternToString(SoundSignalPattern pattern);

#endif // STATE_MACHINE_H
