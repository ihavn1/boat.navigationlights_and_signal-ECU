/**
 * @file ESP32RelayController.h
 * @brief ESP32 GPIO-based implementation of relay controller
 * 
 * Controls opto-isolated relay module via GPIO pins (active-low operation).
 * UI-agnostic: Can be controlled from SignalK (main UI) or web UI (fallback).
 */

#ifndef ESP32_RELAY_CONTROLLER_H
#define ESP32_RELAY_CONTROLLER_H

#include "interfaces/IRelayController.h"
#include <Arduino.h>

/**
 * @class ESP32RelayController
 * @brief Concrete implementation of IRelayController for ESP32
 * 
 * Active-low relay control: Writing LOW activates relay (turns ON)
 * Safety: All pins initialized HIGH (relays OFF) before module power-up
 */
class ESP32RelayController : public IRelayController {
public:
    /**
     * @brief Constructor with GPIO pin assignments
     * @param masthead_pin GPIO for masthead light relay
     * @param port_side_pin GPIO for port sidelight relay
     * @param starboard_side_pin GPIO for starboard sidelight relay
     * @param stern_pin GPIO for sternlight relay
     * @param allround_white_pin GPIO for all-round white light relay
     * @param allround_red_upper_pin GPIO for upper NUC red light relay
     * @param allround_red_lower_pin GPIO for lower NUC red light relay
     * @param horn_pin GPIO for horn relay
     */
    ESP32RelayController(
        uint8_t masthead_pin,
        uint8_t port_side_pin,
        uint8_t starboard_side_pin,
        uint8_t stern_pin,
        uint8_t allround_white_pin,
        uint8_t allround_red_upper_pin,
        uint8_t allround_red_lower_pin,
        uint8_t horn_pin
    );

    ~ESP32RelayController() override = default;

    bool begin() override;
    void activate(RelayChannel channel) override;
    void deactivate(RelayChannel channel) override;
    bool isActive(RelayChannel channel) const override;
    void deactivateAll() override;

private:
    // GPIO pin assignments
    uint8_t pin_map_[8];
    
    // Track relay states (true = active/ON, false = inactive/OFF)
    bool relay_states_[8];

    // Helper to get pin for a channel
    uint8_t getPin(RelayChannel channel) const;
    
    // Helper to get array index for a channel
    uint8_t getIndex(RelayChannel channel) const;
};

#endif // ESP32_RELAY_CONTROLLER_H
