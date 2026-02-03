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
// TODO: Complete SensESP v3 API integration (API has changed from v2)
// #include "sensesp_app_builder.h"
// #include "signalk_integration.h"

// Project includes
#include "ESP32RelayController.h"
#include "ESP32Timer.h"
#include "LightController.h"
#include "SoundController.h"
#include "NavigationLightsECU.h"

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
    Serial.begin(115200);
    delay(100); // Allow serial to stabilize
    
    Serial.println("\n=== Navigation Lights and Signal ECU ===");
    Serial.println("COLREGs compliant - Vessels <15m");
    
    // -----------------------------------------------------------------------
    // STEP 1: Initialize SensESP (TODO: Complete integration)
    // -----------------------------------------------------------------------
    Serial.println("[1/5] Initializing SensESP...");
    
    // TODO: SensESP v3 API has significant changes - need to:
    // 1. Study updated SensESPAppBuilder API
    // 2. Understand new SKPutRequestListener pattern
    // 3. Verify SKOutput template usage
    // 4. Test on actual hardware with SignalK server
    
    Serial.println("  ⚠ SensESP integration pending (Phase 4 TODO)");
    Serial.println("  → All core controllers functional");
    
    // -----------------------------------------------------------------------
    // STEP 2: Initialize Hardware
    // -----------------------------------------------------------------------
    Serial.println("[2/5] Initializing hardware...");
    
    // Create relay controller (active-low safety: all relays OFF on boot)
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
    Serial.println("  ✓ Relay controller initialized (all relays OFF)");
    
    // Create timer
    timer = new ESP32Timer();
    Serial.println("  ✓ Timer initialized");
    
    // -----------------------------------------------------------------------
    // STEP 3: Create Controllers
    // -----------------------------------------------------------------------
    Serial.println("[3/5] Creating controllers...");
    
    light_controller = new LightController(*relay_controller);
    Serial.println("  ✓ Light controller ready");
    
    sound_controller = new SoundController(*relay_controller, *timer);
    Serial.println("  ✓ Sound controller ready (periodic signals start muted)");
    
    // -----------------------------------------------------------------------
    // STEP 4: Create ECU Facade
    // -----------------------------------------------------------------------
    Serial.println("[4/5] Creating ECU controller...");
    
    ecu = new NavigationLightsECU(*light_controller, *sound_controller);
    Serial.println("  ✓ ECU initialized with default state");
    Serial.print("  State: ");
    Serial.println(ecu->getStateDescription().c_str());
    
    // --------------------------------- (TODO: Complete)
    // -----------------------------------------------------------------------
    Serial.println("[5/5] Setting up SignalK integration...");
    
    // TODO: Implement SignalK integration once SensESP v3 API is studied
    // setupSignalK(*ecu);
    Serial.println("  ⚠ SignalK integration pending");
    Serial.println("  → ECU ready for programmatic controltive");
    Serial.println("  ✓ Initial status published");
    
    // -----------------------------------------------------------------------
    // READY
    // -----------------------------------------------------------------------
    Serial.println("\n=== SETUP COMPLETE ===");
    Serial.println("ECU ready for SignalK control");
    Serial.println("All navigation lights OFF (COLREGs default: Day/Moored)");
    Serial.println("Periodic sound signals muted (safety default)");
    Serial.println("\nWaiting for SignalK commands...\n");
}

void loop() {
    // TODO: Add SensESP app.tick() when SignalK integration is complete
    // app.tick();
    
    // Update ECU (processes sound controller timers)
    if (ecu != nullptr) {
        ecu->update();
    }
    
    // Small delay to prevent CPU hogging
    delay(10);
}
