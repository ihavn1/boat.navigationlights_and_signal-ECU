/**
 * @file signalk_integration.cpp
 * @brief SignalK integration for Navigation Lights ECU (SensESP v3 API)
 */

#include "signalk_integration.h"
#include "sensesp/system/valueconsumer.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp_app.h"

using namespace sensesp;

// =======================================================================
// ENUM <-> STRING CONVERSIONS (SignalK format - snake_case)
// Note: Different from state_machine.cpp versions which use human-readable format
// =======================================================================

static const char* sk_conditionToString(Condition condition) {
    switch (condition) {
        case Condition::DAY: return "day";
        case Condition::HOURS_OF_DARKNESS: return "hours_of_darkness";
        case Condition::RESTRICTED_VISIBILITY: return "restricted_visibility";
        default: return "day";
    }
}

static Condition sk_stringToCondition(const String& str) {
    if (str == "hours_of_darkness") return Condition::HOURS_OF_DARKNESS;
    if (str == "restricted_visibility") return Condition::RESTRICTED_VISIBILITY;
    return Condition::DAY; // default
}

static const char* sk_boatStateToString(BoatState state) {
    switch (state) {
        case BoatState::MOORED: return "moored";
        case BoatState::UNDERWAY_MAKING_WAY: return "underway_making_way";
        case BoatState::UNDERWAY_NO_WAY: return "underway_no_way";
        case BoatState::ANCHORAGE: return "anchorage";
        case BoatState::NUC_MAKING_WAY: return "nuc_making_way";
        case BoatState::NUC_NO_WAY: return "nuc_no_way";
        default: return "moored";
    }
}

static BoatState sk_stringToBoatState(const String& str) {
    if (str == "underway_making_way") return BoatState::UNDERWAY_MAKING_WAY;
    if (str == "underway_no_way") return BoatState::UNDERWAY_NO_WAY;
    if (str == "anchorage") return BoatState::ANCHORAGE;
    if (str == "nuc_making_way") return BoatState::NUC_MAKING_WAY;
    if (str == "nuc_no_way") return BoatState::NUC_NO_WAY;
    return BoatState::MOORED; // default
}

static const char* sk_adHocSignalToString(AdHocSignal signal) {
    switch (signal) {
        case AdHocSignal::TURN_STARBOARD: return "turn_starboard";
        case AdHocSignal::TURN_PORT: return "turn_port";
        case AdHocSignal::ASTERN_PROPULSION: return "astern_propulsion";
        case AdHocSignal::DANGER_CONFUSION: return "danger_confusion";
        case AdHocSignal::PAY_ATTENTION: return "pay_attention";
        case AdHocSignal::OVERTAKE_STARBOARD: return "overtake_starboard";
        case AdHocSignal::OVERTAKE_PORT: return "overtake_port";
        case AdHocSignal::AGREEMENT_OVERTAKEN: return "agreement_overtaken";
        default: return "turn_starboard";
    }
}

static AdHocSignal sk_stringToAdHocSignal(const String& str) {
    if (str == "turn_port") return AdHocSignal::TURN_PORT;
    if (str == "astern_propulsion") return AdHocSignal::ASTERN_PROPULSION;
    if (str == "danger_confusion") return AdHocSignal::DANGER_CONFUSION;
    if (str == "pay_attention") return AdHocSignal::PAY_ATTENTION;
    if (str == "overtake_starboard") return AdHocSignal::OVERTAKE_STARBOARD;
    if (str == "overtake_port") return AdHocSignal::OVERTAKE_PORT;
    if (str == "agreement_overtaken") return AdHocSignal::AGREEMENT_OVERTAKEN;
    return AdHocSignal::TURN_STARBOARD;
}

// =======================================================================
// SIGNALK INTEGRATION SETUP
// =======================================================================

// Global ObservableValue objects (source of truth that triggers SignalK updates)
static ObservableValue<String>* condition_value = nullptr;
static ObservableValue<String>* boat_state_value = nullptr;
static ObservableValue<bool>* periodic_muted_value = nullptr;
static ObservableValue<int>* countdown_value = nullptr;
static ObservableValue<bool>* masthead_value = nullptr;
static ObservableValue<bool>* port_value = nullptr;
static ObservableValue<bool>* starboard_value = nullptr;
static ObservableValue<bool>* stern_value = nullptr;
static ObservableValue<bool>* allround_white_value = nullptr;
static ObservableValue<bool>* allround_red_upper_value = nullptr;
static ObservableValue<bool>* allround_red_lower_value = nullptr;
static ObservableValue<bool>* horn_value = nullptr;
static ObservableValue<int>* heartbeat_value = nullptr;

/**
 * @brief Update all ObservableValues from ECU state
 * @param ecu Navigation lights ECU instance
 */
static void updateAllObservableValues(NavigationLightsECU& ecu) {
    Serial.println("[SignalK] updateAllObservableValues: Updating all values");
    
    // State values
    String cond = String(sk_conditionToString(ecu.getCondition()));
    Serial.print("  condition: "); Serial.println(cond);
    condition_value->set(cond);
    
    String state_str = String(sk_boatStateToString(ecu.getBoatState()));
    Serial.print("  boatState: "); Serial.println(state_str);
    boat_state_value->set(state_str);
    
    bool muted = ecu.isPeriodicMuted();
    Serial.print("  periodicMuted: "); Serial.println(muted ? "true" : "false");
    periodic_muted_value->set(muted);
    
    int countdown = (int)ecu.getPeriodicCountdownSeconds();
    Serial.print("  countdown: "); Serial.println(countdown);
    countdown_value->set(countdown);
    
    // Light status
    LightConfiguration lights = ecu.getCurrentLights();
    Serial.println("  Lights:");
    Serial.print("    masthead: "); Serial.println(lights.masthead_light);
    masthead_value->set(lights.masthead_light);
    
    Serial.print("    port: "); Serial.println(lights.port_sidelight);
    port_value->set(lights.port_sidelight);
    
    Serial.print("    starboard: "); Serial.println(lights.starboard_sidelight);
    starboard_value->set(lights.starboard_sidelight);
    
    Serial.print("    stern: "); Serial.println(lights.sternlight);
    stern_value->set(lights.sternlight);
    
    Serial.print("    allround_white: "); Serial.println(lights.allround_white);
    allround_white_value->set(lights.allround_white);
    
    Serial.print("    allround_red_upper: "); Serial.println(lights.allround_red_upper);
    allround_red_upper_value->set(lights.allround_red_upper);
    
    Serial.print("    allround_red_lower: "); Serial.println(lights.allround_red_lower);
    allround_red_lower_value->set(lights.allround_red_lower);
    
    // Horn status
    bool horn = ecu.isHornActive();
    Serial.print("  horn.active: "); Serial.println(horn ? "true" : "false");
    horn_value->set(horn);
    
    Serial.println("[SignalK] updateAllObservableValues: Complete");
}

void setupSignalK(NavigationLightsECU& ecu) {
    Serial.println("\n[SignalK] ===== Starting SignalK Integration Setup =====");
    Serial.println("[SignalK] Base path: " SK_PATH_PREFIX);
    
    // =======================================================================
    // CREATE OBSERVABLE VALUES (Sources that trigger SignalK updates)
    // =======================================================================
    
    Serial.println("[SignalK] Creating ObservableValue objects...");
    
    // Initialize with current ECU state
    condition_value = new ObservableValue<String>(String(sk_conditionToString(ecu.getCondition())));
    boat_state_value = new ObservableValue<String>(String(sk_boatStateToString(ecu.getBoatState())));
    periodic_muted_value = new ObservableValue<bool>(ecu.isPeriodicMuted());
    countdown_value = new ObservableValue<int>((int)ecu.getPeriodicCountdownSeconds());
    
    LightConfiguration initial_lights = ecu.getCurrentLights();
    masthead_value = new ObservableValue<bool>(initial_lights.masthead_light);
    port_value = new ObservableValue<bool>(initial_lights.port_sidelight);
    starboard_value = new ObservableValue<bool>(initial_lights.starboard_sidelight);
    stern_value = new ObservableValue<bool>(initial_lights.sternlight);
    allround_white_value = new ObservableValue<bool>(initial_lights.allround_white);
    allround_red_upper_value = new ObservableValue<bool>(initial_lights.allround_red_upper);
    allround_red_lower_value = new ObservableValue<bool>(initial_lights.allround_red_lower);
    horn_value = new ObservableValue<bool>(ecu.isHornActive());
    heartbeat_value = new ObservableValue<int>(0);  // Start at 0, will toggle
    
    Serial.println("[SignalK] ObservableValue objects created");
    
    // =======================================================================
    // INPUT LISTENERS (PUT requests from SignalK)
    // =======================================================================

    Serial.println("\n[SignalK] Creating PUT request listeners...");

    // Condition listener
    class ConditionConsumer : public ValueConsumer<String> {
     public:
      ConditionConsumer(NavigationLightsECU* ecu_ptr) : ecu(ecu_ptr) {}
      void set(const String& value) override {
        Condition new_condition = sk_stringToCondition(value);
        ecu->setCondition(new_condition);
      }
     private:
      NavigationLightsECU* ecu;
    };
    auto* condition_listener = new StringSKPutRequestListener(
        String(SK_PATH_PREFIX) + ".condition"
    );
    condition_listener->connect_to(new ConditionConsumer(&ecu));

    // Boat state listener
    class BoatStateConsumer : public ValueConsumer<String> {
     public:
      BoatStateConsumer(NavigationLightsECU* ecu_ptr) : ecu(ecu_ptr) {}
      void set(const String& value) override {
        BoatState new_state = sk_stringToBoatState(value);
        ecu->setBoatState(new_state);
      }
     private:
      NavigationLightsECU* ecu;
    };
    auto* boat_state_listener = new StringSKPutRequestListener(
        String(SK_PATH_PREFIX) + ".boatState"
    );
    boat_state_listener->connect_to(new BoatStateConsumer(&ecu));

    // Periodic muted listener
    class MuteConsumer : public ValueConsumer<bool> {
     public:
      MuteConsumer(NavigationLightsECU* ecu_ptr) : ecu(ecu_ptr) {}
      void set(const bool& value) override {
        if (value) {
            ecu->mutePeriodicSignals();
        } else {
            ecu->unmutePeriodicSignals();
        }
      }
     private:
      NavigationLightsECU* ecu;
    };
    auto* mute_listener = new BoolSKPutRequestListener(
        String(SK_PATH_PREFIX) + ".periodicMuted"
    );
    mute_listener->connect_to(new MuteConsumer(&ecu));

    // Ad-hoc signal listener
    class AdHocSignalConsumer : public ValueConsumer<String> {
     public:
      AdHocSignalConsumer(NavigationLightsECU* ecu_ptr) : ecu(ecu_ptr) {}
      void set(const String& value) override {
        AdHocSignal signal = sk_stringToAdHocSignal(value);
        ecu->triggerAdHocSignal(signal);
      }
     private:
      NavigationLightsECU* ecu;
    };
    auto* adhoc_listener = new StringSKPutRequestListener(
        String(SK_PATH_PREFIX) + ".adHocSignal"
    );
    adhoc_listener->connect_to(new AdHocSignalConsumer(&ecu));

    // Emergency stop listener
    class EmergencyStopConsumer : public ValueConsumer<bool> {
     public:
      EmergencyStopConsumer(NavigationLightsECU* ecu_ptr) : ecu(ecu_ptr) {}
      void set(const bool& value) override {
        if (value) {
            ecu->emergencyStop();
        }
      }
     private:
      NavigationLightsECU* ecu;
    };
    auto* emergency_listener = new BoolSKPutRequestListener(
        String(SK_PATH_PREFIX) + ".emergencyStop"
    );
    emergency_listener->connect_to(new EmergencyStopConsumer(&ecu));

    // =======================================================================
    // OUTPUT PUBLISHERS (ObservableValue→SKOutput, boat.light-signal-ECU pattern)
    // =======================================================================
    
    Serial.println("[SignalK] Connecting ObservableValues to SKOutput...");
    
    // Connect state values to SignalK outputs
    condition_value->connect_to(new SKOutput<String>(
        String(SK_PATH_PREFIX) + ".condition",
        "/nav/condition",
        new SKMetadata("", "Lighting condition")
    ));
    
    boat_state_value->connect_to(new SKOutput<String>(
        String(SK_PATH_PREFIX) + ".boatState",
        "/nav/boatState",
        new SKMetadata("", "Boat operational state")
    ));
    
    periodic_muted_value->connect_to(new SKOutput<bool>(
        String(SK_PATH_PREFIX) + ".periodicMuted",
        "/nav/periodicMuted",
        new SKMetadata("", "Periodic signals muted")
    ));
    
    countdown_value->connect_to(new SKOutput<int>(
        String(SK_PATH_PREFIX) + ".periodicCountdown",
        "/nav/countdown",
        new SKMetadata("s", "Seconds until next periodic signal")
    ));
    
    // Connect light status to SignalK outputs
    String lights_prefix = String(SK_PATH_PREFIX) + ".lights.";
    
    masthead_value->connect_to(new SKOutput<bool>(
        lights_prefix + "mastheadLight",
        "/nav/lights/masthead",
        new SKMetadata("", "Masthead light")
    ));
    
    port_value->connect_to(new SKOutput<bool>(
        lights_prefix + "portSidelight",
        "/nav/lights/port",
        new SKMetadata("", "Port sidelight")
    ));
    
    starboard_value->connect_to(new SKOutput<bool>(
        lights_prefix + "starboardSidelight",
        "/nav/lights/starboard",
        new SKMetadata("", "Starboard sidelight")
    ));
    
    stern_value->connect_to(new SKOutput<bool>(
        lights_prefix + "sternlight",
        "/nav/lights/stern",
        new SKMetadata("", "Sternlight")
    ));
    
    allround_white_value->connect_to(new SKOutput<bool>(
        lights_prefix + "allroundWhite",
        "/nav/lights/white",
        new SKMetadata("", "All-round white light")
    ));
    
    allround_red_upper_value->connect_to(new SKOutput<bool>(
        lights_prefix + "allroundRedUpper",
        "/nav/lights/red_upper",
        new SKMetadata("", "All-round red upper")
    ));
    
    allround_red_lower_value->connect_to(new SKOutput<bool>(
        lights_prefix + "allroundRedLower",
        "/nav/lights/red_lower",
        new SKMetadata("", "All-round red lower")
    ));
    
    horn_value->connect_to(new SKOutput<bool>(
        String(SK_PATH_PREFIX) + ".horn.active",
        "/nav/horn",
        new SKMetadata("", "Horn active")
    ));
    
    heartbeat_value->connect_to(new SKOutput<int>(
        String(SK_PATH_PREFIX) + ".heartbeat",
        "/nav/heartbeat",
        new SKMetadata("", "ECU heartbeat - toggles every 60 seconds")
    ));
    
    Serial.println("[SignalK] ObservableValues connected to SKOutput");
    
    // =======================================================================
    // REPEATSENSORS (Call .set() + read back for forced emission)
    // =======================================================================
    
    Serial.println("[SignalK] Setting up RepeatSensors to update ObservableValues...");
    
    // CRITICAL: SKOutput filters unchanged values. 
    // Workaround: .set() updates internal value, then immediately read it back
    // This creates a fresh reading that SKOutput treats as new
    
    // Countdown: Update every 1s (always changes naturally)
    auto* countdown_sensor = new RepeatSensor<int>(1000, [&ecu]() {
        int val = (int)ecu.getPeriodicCountdownSeconds();
        countdown_value->set(val);
        countdown_value->notify();
        return val;  // Force fresh emission
    });
    
    // Horn: Update every 100ms (changes naturally)
    auto* horn_sensor = new RepeatSensor<bool>(100, [&ecu]() {
        bool val = ecu.isHornActive();
        horn_value->set(val);
        horn_value->notify();
        return val;  // Force fresh emission
    });
    
    // Heartbeat + Update all values: Every 60s
    auto* heartbeat_sensor = new RepeatSensor<int>(60000, [&ecu]() {
        Serial.println("\n[SignalK] === HEARTBEAT ===");
        
        // Update all values (boat.light-signal-ECU pattern: just call .set() on all values)
        updateAllObservableValues(ecu);
        
        // Toggle heartbeat (naturally changes each time)
        static int toggle = 0;
        toggle = 1 - toggle;
        heartbeat_value->set(toggle);
        
        Serial.println("[SignalK] Heartbeat toggle: " + String(toggle));
        return toggle;
    });
    
    Serial.println("[SignalK] RepeatSensors configured:");
    Serial.println("  - Countdown: 1s");
    Serial.println("  - Horn: 100ms");
    Serial.println("  - Heartbeat + refresh all values: 60s");

    // =======================================================================
    // STATE CHANGE CALLBACK (Update ObservableValues which trigger SignalK)
    // =======================================================================

    Serial.println("[SignalK] Setting up state change callback...");

    ecu.onStateChange([&ecu]() {
        Serial.println("\n[SignalK] State changed! Updating ObservableValues...");
        updateAllObservableValues(ecu);
        Serial.println("[SignalK] ObservableValues updated");
    });

    // =======================================================================
    // PUBLISH INITIAL STATE ON CONNECTION (using simpler approach)
    // =======================================================================
    
    Serial.println("\n[SignalK] Setting up connection-based state publishing...");
    
    // Track connection state to republish on reconnect
    static bool was_connected = false;
    static bool first_connection = true;
    static unsigned long connection_time = 0;
    
    // Check connection every second and publish state when reconnected
    sensesp_app->get_event_loop()->onRepeat(1000, [&ecu]() {
        auto ws_client = sensesp_app->get_ws_client();
        bool is_connected = ws_client->is_connected();
        
        if (is_connected && !was_connected) {
            // Just connected (first or reconnection) - record time to publish after 2s delay
            connection_time = millis();
            if (first_connection) {
                Serial.println("\n[SignalK] Initial connection, waiting for server to be ready...");
                first_connection = false;
            } else {
                Serial.println("\n[SignalK] Reconnected, waiting for server to be ready...");
            }
        }
        else if (is_connected && connection_time > 0 && (millis() - connection_time >= 2000)) {
            // Connected and stable for 2+ seconds - publish/republish state
            Serial.println("\n[SignalK] Publishing all values to SignalK...");
            
            updateAllObservableValues(ecu);
            
            Serial.println("[SignalK] All values published!\n");
            
            connection_time = 0;  // Reset so we don't publish again
        }
        
        was_connected = is_connected;
    });

    Serial.println("\n[SignalK] ===== SignalK Integration Setup Complete =====\n");
}
