/**
 * @file signalk_integration.cpp
 * @brief SignalK integration for Navigation Lights ECU (SensESP v3 API)
 */

#include "signalk_integration.h"
#include "sensesp/system/valueconsumer.h"
#include "sensesp/signalk/signalk_ws_client.h"
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

// Global pointers to SKOutput objects (must persist)
static SKOutput<String>* condition_output = nullptr;
static SKOutput<String>* boat_state_output = nullptr;
static SKOutput<bool>* periodic_muted_output = nullptr;
static SKOutput<int>* countdown_output = nullptr;
static SKOutput<bool>* masthead_output = nullptr;
static SKOutput<bool>* port_output = nullptr;
static SKOutput<bool>* starboard_output = nullptr;
static SKOutput<bool>* stern_output = nullptr;
static SKOutput<bool>* allround_white_output = nullptr;
static SKOutput<bool>* allround_red_upper_output = nullptr;
static SKOutput<bool>* allround_red_lower_output = nullptr;
static SKOutput<bool>* horn_output = nullptr;

void setupSignalK(NavigationLightsECU& ecu) {
    Serial.println("\n[SignalK] ===== Starting SignalK Integration Setup =====");
    Serial.println("[SignalK] Base path: " SK_PATH_PREFIX);
    
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
    // OUTPUT PUBLISHERS (Status to SignalK)
    // =======================================================================

    // Create persistent SK output objects
    condition_output = new SKOutput<String>(
        String(SK_PATH_PREFIX) + ".condition",
        "/nav/condition",
        new SKMetadata("", "Lighting condition")
    );

    boat_state_output = new SKOutput<String>(
        String(SK_PATH_PREFIX) + ".boatState",
        "/nav/boatState",
        new SKMetadata("", "Boat operational state")
    );

    periodic_muted_output = new SKOutput<bool>(
        String(SK_PATH_PREFIX) + ".periodicMuted",
        "/nav/periodicMuted",
        new SKMetadata("", "Periodic signals muted")
    );

    countdown_output = new SKOutput<int>(
        String(SK_PATH_PREFIX) + ".periodicCountdown",
        "/nav/countdown",
        new SKMetadata("s", "Seconds until next periodic signal")
    );

    // Light outputs
    String lights_prefix = String(SK_PATH_PREFIX) + ".lights.";
    
    masthead_output = new SKOutput<bool>(
        lights_prefix + "mastheadLight",
        "/nav/lights/masthead",
        new SKMetadata("", "Masthead light")
    );

    port_output = new SKOutput<bool>(
        lights_prefix + "portSidelight",
        "/nav/lights/port",
        new SKMetadata("", "Port sidelight")
    );

    starboard_output = new SKOutput<bool>(
        lights_prefix + "starboardSidelight",
        "/nav/lights/starboard",
        new SKMetadata("", "Starboard sidelight")
    );

    stern_output = new SKOutput<bool>(
        lights_prefix + "sternlight",
        "/nav/lights/stern",
        new SKMetadata("", "Sternlight")
    );

    allround_white_output = new SKOutput<bool>(
        lights_prefix + "allroundWhite",
        "/nav/lights/white",
        new SKMetadata("", "All-round white light")
    );

    allround_red_upper_output = new SKOutput<bool>(
        lights_prefix + "allroundRedUpper",
        "/nav/lights/red_upper",
        new SKMetadata("", "All-round red upper")
    );

    allround_red_lower_output = new SKOutput<bool>(
        lights_prefix + "allroundRedLower",
        "/nav/lights/red_lower",
        new SKMetadata("", "All-round red lower")
    );

    horn_output = new SKOutput<bool>(
        String(SK_PATH_PREFIX) + ".horn.active",
        "/nav/horn",
        new SKMetadata("", "Horn active")
    );

    // =======================================================================
    // STATE CHANGE CALLBACK (Publish updates)
    // =======================================================================

    Serial.println("[SignalK] Setting up state change callback...");

    ecu.onStateChange([&ecu]() {
        Serial.println("\n[SignalK] State changed! Publishing updates...");
        
        // Publish current state
        String condition_str = String(sk_conditionToString(ecu.getCondition()));
        String state_str = String(sk_boatStateToString(ecu.getBoatState()));
        
        Serial.print("  condition: "); Serial.println(condition_str);
        condition_output->set(condition_str);
        
        Serial.print("  boatState: "); Serial.println(state_str);
        boat_state_output->set(state_str);
        
        Serial.print("  periodicMuted: "); Serial.println(ecu.isPeriodicMuted() ? "true" : "false");
        periodic_muted_output->set(ecu.isPeriodicMuted());
        
        Serial.print("  countdown: "); Serial.println(ecu.getPeriodicCountdownSeconds());
        countdown_output->set((int)ecu.getPeriodicCountdownSeconds());

        // Publish light status
        LightConfiguration lights = ecu.getCurrentLights();
        Serial.println("  Lights:");
        Serial.print("    masthead: "); Serial.println(lights.masthead_light);
        masthead_output->set(lights.masthead_light);
        Serial.print("    port: "); Serial.println(lights.port_sidelight);
        port_output->set(lights.port_sidelight);
        Serial.print("    starboard: "); Serial.println(lights.starboard_sidelight);
        starboard_output->set(lights.starboard_sidelight);
        Serial.print("    stern: "); Serial.println(lights.sternlight);
        stern_output->set(lights.sternlight);
        Serial.print("    allround_white: "); Serial.println(lights.allround_white);
        allround_white_output->set(lights.allround_white);
        Serial.print("    allround_red_upper: "); Serial.println(lights.allround_red_upper);
        allround_red_upper_output->set(lights.allround_red_upper);
        Serial.print("    allround_red_lower: "); Serial.println(lights.allround_red_lower);
        allround_red_lower_output->set(lights.allround_red_lower);

        // Publish horn status
        Serial.print("  horn.active: "); Serial.println(ecu.isHornActive());
        horn_output->set(ecu.isHornActive());
        
        Serial.println("[SignalK] All updates published");
    });

    // Publish initial state immediately
    Serial.println("\n[SignalK] Publishing initial state...");
    
    String initial_condition = String(sk_conditionToString(ecu.getCondition()));
    String initial_state = String(sk_boatStateToString(ecu.getBoatState()));
    
    Serial.print("  Initial condition: "); Serial.println(initial_condition);
    condition_output->set(initial_condition);
    
    Serial.print("  Initial boatState: "); Serial.println(initial_state);
    boat_state_output->set(initial_state);
    
    Serial.print("  Initial periodicMuted: "); Serial.println(ecu.isPeriodicMuted() ? "true" : "false");
    periodic_muted_output->set(ecu.isPeriodicMuted());
    
    Serial.print("  Initial countdown: "); Serial.println(ecu.getPeriodicCountdownSeconds());
    countdown_output->set((int)ecu.getPeriodicCountdownSeconds());

    LightConfiguration lights = ecu.getCurrentLights();
    Serial.println("  Initial lights:");
    Serial.print("    masthead: "); Serial.println(lights.masthead_light);
    masthead_output->set(lights.masthead_light);
    Serial.print("    port: "); Serial.println(lights.port_sidelight);
    port_output->set(lights.port_sidelight);
    Serial.print("    starboard: "); Serial.println(lights.starboard_sidelight);
    starboard_output->set(lights.starboard_sidelight);
    Serial.print("    stern: "); Serial.println(lights.sternlight);
    stern_output->set(lights.sternlight);
    Serial.print("    allround_white: "); Serial.println(lights.allround_white);
    allround_white_output->set(lights.allround_white);
    Serial.print("    allround_red_upper: "); Serial.println(lights.allround_red_upper);
    allround_red_upper_output->set(lights.allround_red_upper);
    Serial.print("    allround_red_lower: "); Serial.println(lights.allround_red_lower);
    allround_red_lower_output->set(lights.allround_red_lower);
    
    Serial.print("  Initial horn.active: "); Serial.println(ecu.isHornActive());
    horn_output->set(ecu.isHornActive());
    
    Serial.println("[SignalK] Initial state published!\n");
}
