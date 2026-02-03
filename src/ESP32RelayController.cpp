/**
 * @file ESP32RelayController.cpp
 * @brief Implementation of ESP32 GPIO-based relay controller
 */

#include "ESP32RelayController.h"

ESP32RelayController::ESP32RelayController(
    uint8_t masthead_pin,
    uint8_t port_side_pin,
    uint8_t starboard_side_pin,
    uint8_t stern_pin,
    uint8_t allround_white_pin,
    uint8_t allround_red_upper_pin,
    uint8_t allround_red_lower_pin,
    uint8_t horn_pin
) {
    pin_map_[static_cast<uint8_t>(RelayChannel::MASTHEAD_LIGHT)] = masthead_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::PORT_SIDELIGHT)] = port_side_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::STARBOARD_SIDELIGHT)] = starboard_side_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::STERNLIGHT)] = stern_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::ALLROUND_WHITE)] = allround_white_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::ALLROUND_RED_UPPER)] = allround_red_upper_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::ALLROUND_RED_LOWER)] = allround_red_lower_pin;
    pin_map_[static_cast<uint8_t>(RelayChannel::HORN)] = horn_pin;
    
    // Initialize all relay states to OFF
    for (int i = 0; i < 8; i++) {
        relay_states_[i] = false;
    }
}

bool ESP32RelayController::begin() {
    // SAFETY CRITICAL: Configure all GPIO pins as OUTPUT with HIGH state
    // BEFORE relay module is powered up (active-low = HIGH is OFF)
    for (int i = 0; i < 8; i++) {
        pinMode(pin_map_[i], OUTPUT);
        digitalWrite(pin_map_[i], HIGH); // Ensure all relays OFF
    }
    
    return true;
}

void ESP32RelayController::activate(RelayChannel channel) {
    uint8_t pin = getPin(channel);
    uint8_t idx = getIndex(channel);
    
    digitalWrite(pin, LOW); // Active-low: LOW = relay ON
    relay_states_[idx] = true;
}

void ESP32RelayController::deactivate(RelayChannel channel) {
    uint8_t pin = getPin(channel);
    uint8_t idx = getIndex(channel);
    
    digitalWrite(pin, HIGH); // Active-low: HIGH = relay OFF
    relay_states_[idx] = false;
}

bool ESP32RelayController::isActive(RelayChannel channel) const {
    return relay_states_[getIndex(channel)];
}

void ESP32RelayController::deactivateAll() {
    // Emergency/safety function: turn off all relays immediately
    for (int i = 0; i < 8; i++) {
        digitalWrite(pin_map_[i], HIGH); // Active-low: HIGH = OFF
        relay_states_[i] = false;
    }
}

uint8_t ESP32RelayController::getPin(RelayChannel channel) const {
    return pin_map_[static_cast<uint8_t>(channel)];
}

uint8_t ESP32RelayController::getIndex(RelayChannel channel) const {
    return static_cast<uint8_t>(channel);
}
