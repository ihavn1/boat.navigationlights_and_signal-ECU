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
 * - GPIO 32: Yellow Towing Light (Relay 8)
 * - GPIO 4: Horn (Relay 9)
 * 
 * Active-LOW control (production): HIGH=OFF, LOW=ON
 * Active-HIGH testing mode: Set ACTIVE_HIGH_RELAYS in platformio.ini
 * 
 * See HARDWARE.md for complete pin mapping, reserved pins, and wiring diagrams.
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include <LittleFS.h>
#include <esp_partition.h>
#include "sensesp_app_builder.h"
#include "sensesp/system/led_blinker.h"
#include "sensesp/ui/ui_controls.h"
#include "sensesp/ui/config_item.h"
#include "signalk_integration.h"
#include "web_api.h"

// Project includes
#include "ESP32RelayController.h"
#include "ESP32Timer.h"
#include "LightController.h"
#include "SoundController.h"
#include "NavigationLightsECU.h"

using namespace sensesp;

// Helper to access protected http_server_ member
namespace {
    class SensESPAppAccessor : public SensESPApp {
    public:
        static HTTPServer* getHttpServer(SensESPApp* app) {
            return static_cast<SensESPAppAccessor*>(app)->http_server_.get();
        }
    };
}

// GPIO pin definitions
// Production: Active-LOW opto-isolated relay module (LOW=ON, HIGH=OFF)
// Testing: Set ACTIVE_HIGH_RELAYS in platformio.ini for active-HIGH relays (HIGH=ON, LOW=OFF)
const uint8_t PIN_MASTHEAD_LIGHT = 25;
const uint8_t PIN_PORT_SIDELIGHT = 26;
const uint8_t PIN_STARBOARD_SIDELIGHT = 27;
const uint8_t PIN_STERNLIGHT = 14;
const uint8_t PIN_ALLROUND_WHITE = 12;
const uint8_t PIN_ALLROUND_RED_UPPER = 13;
const uint8_t PIN_ALLROUND_RED_LOWER = 15;
const uint8_t PIN_YELLOW_TOWING_LIGHT = 32;
const uint8_t PIN_HORN = 4;

// Global objects (need to persist across loop iterations)
ESP32RelayController* relay_controller = nullptr;
ESP32Timer* timer = nullptr;
LightController* light_controller = nullptr;
SoundController* sound_controller = nullptr;
NavigationLightsECU* ecu = nullptr;

// Hardware capability configuration (runtime-configurable via SensESP web UI)
bool g_has_nuc_lights = true;     // Configurable: Does boat have NUC (Not Under Command) lights?
bool g_has_towing_lights = true;  // Configurable: Does boat have towing lights?
std::shared_ptr<CheckboxConfig> g_config_has_nuc = nullptr;     // Config objects for reading current values
std::shared_ptr<CheckboxConfig> g_config_has_towing = nullptr;

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
#ifdef ACTIVE_HIGH_RELAYS
    Serial.println("Relay Mode: ACTIVE-HIGH (Testing)");
#else
    Serial.println("Relay Mode: ACTIVE-LOW (Production)");
#endif
    Serial.println("========================================");
    
    // Check partition table
    Serial.println("\nPartition Information:");
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (pi != NULL) {
        do {
            const esp_partition_t* p = esp_partition_get(pi);
            Serial.printf("  %s: size=%dKB, address=0x%x\n", p->label, p->size/1024, p->address);
        } while ((pi = esp_partition_next(pi)) != NULL);
        esp_partition_iterator_release(pi);
    }
    
    // Initialize SensESP application FIRST (it manages LittleFS for config storage)
    Serial.println("\nInitializing SensESP (will use 'littlefs' partition for config)...");
    SensESPAppBuilder builder;
    sensesp_app = (&builder)
        ->set_hostname("nav-lights-ecu")
        ->enable_ota("boat-ecu")
        ->get_app();
    
    // Add hardware capability configuration items (appear in SensESP web UI)
    // Use ConfigItem() to register checkboxes with SensESP's configuration system
    g_config_has_nuc = std::make_shared<CheckboxConfig>(
        g_has_nuc_lights,
        "NUC Lights Installed",
        "/Hardware/NUC_Lights"
    );
    ConfigItem(g_config_has_nuc)
        ->set_title("NUC Lights")
        ->set_description("Does this boat have NUC (Not Under Command) lights installed?")
        ->set_requires_restart(true);
    g_has_nuc_lights = g_config_has_nuc->get_value();
    
    g_config_has_towing = std::make_shared<CheckboxConfig>(
        g_has_towing_lights,
        "Towing Lights Installed",
        "/Hardware/Towing_Lights"
    );
    ConfigItem(g_config_has_towing)
        ->set_title("Towing Lights")
        ->set_description("Does this boat have towing lights installed?")
        ->set_requires_restart(true);
    g_has_towing_lights = g_config_has_towing->get_value();
    
    Serial.println("\nHardware configuration:");
    Serial.printf("  NUC Lights: %s\n", g_has_nuc_lights ? "Installed" : "Not Installed");
    Serial.printf("  Towing Lights: %s\n", g_has_towing_lights ? "Installed" : "Not Installed");
    
    // NOW initialize SPIFFS for web UI files (AFTER SensESP has initialized LittleFS)
    // This way: SensESP config in 'littlefs' partition, Web UI files in 'spiffs' partition - no conflicts!
    Serial.println("\nInitializing SPIFFS for web UI files (using 'spiffs' partition)...");
    // Mount to /www (VFS mount point) but access files directly via SPIFFS.open()
    if (!SPIFFS.begin(false, "/www", 10, "spiffs")) {  // false = don't auto-format
        Serial.println("WARNING: SPIFFS mount failed - web UI not available");
        Serial.println("Run 'pio run --target uploadfs' to upload web UI files");
    } else {
        Serial.println("SPIFFS mounted successfully");
        Serial.printf("  Total: %d bytes, Used: %d bytes\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
    }
    
    // Initialize hardware layer
    relay_controller = new ESP32RelayController(
        PIN_MASTHEAD_LIGHT,
        PIN_PORT_SIDELIGHT,
        PIN_STARBOARD_SIDELIGHT,
        PIN_STERNLIGHT,
        PIN_ALLROUND_WHITE,
        PIN_ALLROUND_RED_UPPER,
        PIN_ALLROUND_RED_LOWER,
        PIN_YELLOW_TOWING_LIGHT,
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
    
    // Setup Web API for fallback UI
    HTTPServer* http_server = SensESPAppAccessor::getHttpServer(sensesp_app.get());
    setupWebAPI(http_server, ecu);
    setupStaticFiles(http_server);
    
    Serial.println("\nSystem initialized successfully");
    Serial.println("Connect to http://nav-lights-ecu.local for configuration");
    Serial.println("Web API available at http://nav-lights-ecu.local/api/status");
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
