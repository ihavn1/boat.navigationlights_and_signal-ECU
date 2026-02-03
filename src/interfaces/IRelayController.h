/**
 * @file IRelayController.h
 * @brief Hardware abstraction interface for relay control
 * 
 * Defines interface for controlling navigation lights and horn via relays.
 * Active-low control: LOW = relay ON, HIGH = relay OFF (safety requirement)
 */

#ifndef I_RELAY_CONTROLLER_H
#define I_RELAY_CONTROLLER_H

#include <cstdint>

/**
 * @enum RelayChannel
 * @brief Identifies each relay output for navigation lights and horn
 */
enum class RelayChannel : uint8_t {
    MASTHEAD_LIGHT = 0,      // White light forward 225° arc (Rule 21)
    PORT_SIDELIGHT = 1,      // Red light 112.5° arc port side (Rule 21)
    STARBOARD_SIDELIGHT = 2, // Green light 112.5° arc starboard side (Rule 21)
    STERNLIGHT = 3,          // White light aft 135° arc (Rule 21)
    ALLROUND_WHITE = 4,      // 360° white light (anchorage, Rule 30)
    ALLROUND_RED_UPPER = 5,  // Upper red light for NUC (Rule 27)
    ALLROUND_RED_LOWER = 6,  // Lower red light for NUC (Rule 27)
    HORN = 7                 // Sound signaling device
};

/**
 * @interface IRelayController
 * @brief Abstract interface for relay hardware control
 * 
 * Enables testing without physical hardware via dependency injection.
 * Implementations must ensure active-low operation (LOW = ON).
 */
class IRelayController {
public:
    virtual ~IRelayController() = default;

    /**
     * @brief Initialize relay controller hardware
     * @return true if initialization successful, false otherwise
     * 
     * Must configure all GPIO pins as OUTPUT with initial state HIGH (relays OFF)
     * before relay module power-up to ensure safety on boot.
     */
    virtual bool begin() = 0;

    /**
     * @brief Activate a relay (turn ON - pull LOW for active-low modules)
     * @param channel The relay to activate
     */
    virtual void activate(RelayChannel channel) = 0;

    /**
     * @brief Deactivate a relay (turn OFF - set HIGH for active-low modules)
     * @param channel The relay to deactivate
     */
    virtual void deactivate(RelayChannel channel) = 0;

    /**
     * @brief Get current state of a relay
     * @param channel The relay to query
     * @return true if relay is active (ON), false if inactive (OFF)
     */
    virtual bool isActive(RelayChannel channel) const = 0;

    /**
     * @brief Deactivate all relays immediately (emergency/safety function)
     */
    virtual void deactivateAll() = 0;
};

#endif // I_RELAY_CONTROLLER_H
