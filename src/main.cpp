/**
 * @file main.cpp
 * @brief Main entry point for Navigation Lights and Signal ECU
 * 
 * Implements COLREGs-compliant navigation lights and sound signals for pleasure boats <15m.
 * Built on SensESP platform with SignalK protocol for communication.
 * 
 * GPIO Pin Mapping (AZ-Delivery ESP32 Dev Kit C V4):
 * - GPIO 25: Masthead Light (Relay 1)
 * - GPIO 26: Port Sidelight (Relay 2)
 * - GPIO 27: Starboard Sidelight (Relay 3)
 * - GPIO 14: Sternlight (Relay 4)
 * - GPIO 12: All-round White (Relay 5)
 * - GPIO 13: All-round Red Upper (Relay 6)
 * - GPIO 15: All-round Red Lower (Relay 7)
 * - GPIO 4: Horn (Relay 8)
 */

#include <Arduino.h>
#include "sensesp_app_builder.h"
#include "sensesp/system/led_blinker.h"
#include "signalk_integration.h"

// Project includes
#include "ESP32RelayController.h"
#include "ESP32Timer.h"
#include "LightController.h"
#include "SoundController.h"
#include "NavigationLightsECU.h"

using namespace sensesp;

// GPIO pin definitions (active-low opto-isolated relay module)
const uint8_t PIN_MASTHEAD_LIGHT = 25;
const uint8_t PIN_PORT_SIDELIGHT = 26;
const uint8_t PIN_STARBOARD_SIDELIGHT = 27;
const uint8_t PIN_STERNLIGHT = 14;
const uint8_t PIN_ALLROUND_WHITE = 12;
const uint8_t PIN_ALLROUND_RED_UPPER = 13;
const uint8_t PIN_ALLROUND_RED_LOWER = 15;
const uint8_t PIN_HORN = 4;

// Global objects (need to persist across loop iterations)
ESP32RelayController* relay_controller = nullptr;
ESP32Timer* timer = nullptr;
LightController* light_controller = nullptr;
SoundController* sound_controller = nullptr;
NavigationLightsECU* ecu = nullptr;

void setup() {
    // Initialize logging subsystem
    SetupLogging();
    
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("Navigation Lights & Signal ECU");
    Serial.println("Version 1.0.0 - Production");
    Serial.println("========================================");
    
    // Initialize SensESP application
    SensESPAppBuilder builder;
    sensesp_app = (&builder)
        ->set_hostname("nav-lights-ecu")
        ->enable_ota("boat-ecu")
        ->get_app();
    
    // Initialize hardware layer
    relay_controller = new ESP32RelayController(
        PIN_MASTHEAD_LIGHT,
        PIN_PORT_SIDELIGHT,
        PIN_STARBOARD_SIDELIGHT,
        PIN_STERNLIGHT,
        PIN_ALLROUND_WHITE,
        PIN_ALLROUND_RED_UPPER,
        PIN_ALLROUND_RED_LOWER,
        PIN_HORN
    );
    relay_controller->begin();
    
    // Create timer
    timer = new ESP32Timer();
    
    // Create controllers
    light_controller = new LightController(*relay_controller);
    sound_controller = new SoundController(*relay_controller, *timer);
    
    // Create ECU facade
    ecu = new NavigationLightsECU(*light_controller, *sound_controller);
    
    // Setup SignalK integration
    setupSignalK(*ecu);
    
    Serial.println("\nSystem initialized successfully");
    Serial.println("Connect to http://nav-lights-ecu.local for configuration");
}

void loop() {
    // CRITICAL: Process SensESP event loop (handles WiFi, HTTP server, WebSocket)
    // This MUST be called - it runs the ReactESP event loop which drives RepeatSensors
    static auto event_loop = sensesp_app->get_event_loop();
    event_loop->tick();
    
    // Update ECU (processes sound controller timers)
    if (ecu != nullptr) {
        ecu->update();
    }
}
