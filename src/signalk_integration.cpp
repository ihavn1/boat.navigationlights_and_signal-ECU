/**
 * @file signalk_integration.cpp
 * @brief Implementation of SignalK integration
 */

#include "signalk_integration.h"

// =======================================================================
// ENUM <-> STRING CONVERSIONS
// =======================================================================

const char* conditionToString(Condition condition) {
    switch (condition) {
        case Condition::DAY: return "day";
        case Condition::HOURS_OF_DARKNESS: return "hours_of_darkness";
        case Condition::RESTRICTED_VISIBILITY: return "restricted_visibility";
        default: return "day";
    }
}

Condition stringToCondition(const char* str) {
    if (strcmp(str, "hours_of_darkness") == 0) return Condition::HOURS_OF_DARKNESS;
    if (strcmp(str, "restricted_visibility") == 0) return Condition::RESTRICTED_VISIBILITY;
    return Condition::DAY; // default
}

const char* boatStateToString(BoatState state) {
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

BoatState stringToBoatState(const char* str) {
    if (strcmp(str, "underway_making_way") == 0) return BoatState::UNDERWAY_MAKING_WAY;
    if (strcmp(str, "underway_no_way") == 0) return BoatState::UNDERWAY_NO_WAY;
    if (strcmp(str, "anchorage") == 0) return BoatState::ANCHORAGE;
    if (strcmp(str, "nuc_making_way") == 0) return BoatState::NUC_MAKING_WAY;
    if (strcmp(str, "nuc_no_way") == 0) return BoatState::NUC_NO_WAY;
    return BoatState::MOORED; // default
}

const char* adHocSignalToString(AdHocSignal signal) {
    switch (signal) {
        case AdHocSignal::ONE_SHORT_BLAST: return "one_short_blast";
        case AdHocSignal::TWO_SHORT_BLASTS: return "two_short_blasts";
        case AdHocSignal::THREE_SHORT_BLASTS: return "three_short_blasts";
        case AdHocSignal::FIVE_SHORT_BLASTS: return "five_short_blasts";
        case AdHocSignal::ONE_PROLONGED_BLAST: return "one_prolonged_blast";
        default: return "one_short_blast";
    }
}

AdHocSignal stringToAdHocSignal(const char* str) {
    if (strcmp(str, "two_short_blasts") == 0) return AdHocSignal::TWO_SHORT_BLASTS;
    if (strcmp(str, "three_short_blasts") == 0) return AdHocSignal::THREE_SHORT_BLASTS;
    if (strcmp(str, "five_short_blasts") == 0) return AdHocSignal::FIVE_SHORT_BLASTS;
    if (strcmp(str, "one_prolonged_blast") == 0) return AdHocSignal::ONE_PROLONGED_BLAST;
    return AdHocSignal::ONE_SHORT_BLAST;
}

// =======================================================================
// SIGNALK PUT REQUEST HANDLERS (INPUTS FROM UI)
// =======================================================================

/**
 * @brief Handle condition change from SignalK
 * Path: electrical.switches.navigationLights.condition
 */
class ConditionPutRequestListener : public SKPutRequestListener {
public:
    ConditionPutRequestListener(NavigationLightsECU& ecu, String sk_path)
        : SKPutRequestListener(sk_path), ecu_(ecu) {}

    void set(const String& new_value) override {
        Condition new_condition = stringToCondition(new_value.c_str());
        ecu_.setCondition(new_condition);
    }

private:
    NavigationLightsECU& ecu_;
};

/**
 * @brief Handle boat state change from SignalK
 * Path: electrical.switches.navigationLights.boatState
 */
class BoatStatePutRequestListener : public SKPutRequestListener {
public:
    BoatStatePutRequestListener(NavigationLightsECU& ecu, String sk_path)
        : SKPutRequestListener(sk_path), ecu_(ecu) {}

    void set(const String& new_value) override {
        BoatState new_state = stringToBoatState(new_value.c_str());
        ecu_.setBoatState(new_state);
    }

private:
    NavigationLightsECU& ecu_;
};

/**
 * @brief Handle periodic mute toggle from SignalK
 * Path: electrical.switches.navigationLights.periodicMuted
 */
class PeriodicMutePutRequestListener : public SKPutRequestListener {
public:
    PeriodicMutePutRequestListener(NavigationLightsECU& ecu, String sk_path)
        : SKPutRequestListener(sk_path), ecu_(ecu) {}

    void set(const bool& new_value) override {
        if (new_value) {
            ecu_.mutePeriodicSignals();
        } else {
            ecu_.unmutePeriodicSignals();
        }
    }

private:
    NavigationLightsECU& ecu_;
};

/**
 * @brief Handle ad-hoc signal trigger from SignalK
 * Path: electrical.switches.navigationLights.adHocSignal
 */
class AdHocSignalPutRequestListener : public SKPutRequestListener {
public:
    AdHocSignalPutRequestListener(NavigationLightsECU& ecu, String sk_path)
        : SKPutRequestListener(sk_path), ecu_(ecu) {}

    void set(const String& new_value) override {
        AdHocSignal signal = stringToAdHocSignal(new_value.c_str());
        ecu_.triggerAdHocSignal(signal);
    }

private:
    NavigationLightsECU& ecu_;
};

/**
 * @brief Handle emergency stop from SignalK
 * Path: electrical.switches.navigationLights.emergencyStop
 */
class EmergencyStopPutRequestListener : public SKPutRequestListener {
public:
    EmergencyStopPutRequestListener(NavigationLightsECU& ecu, String sk_path)
        : SKPutRequestListener(sk_path), ecu_(ecu) {}

    void set(const bool& new_value) override {
        if (new_value) {
            ecu_.emergencyStop();
        }
    }

private:
    NavigationLightsECU& ecu_;
};

// =======================================================================
// SIGNALK OUTPUT PUBLISHERS (STATUS TO UI)
// =======================================================================

/**
 * @brief Publish all ECU status to SignalK
 * Called on boot, reconnect, and state changes
 */
void publishECUStatus(NavigationLightsECU& ecu) {
    // Publish condition
    String condition_path = String(SK_PATH_PREFIX) + ".condition";
    SKOutput<String>(condition_path, "", new SKMetadata("", "Lighting condition"))
        .set_input(String(conditionToString(ecu.getCondition())));

    // Publish boat state
    String boat_state_path = String(SK_PATH_PREFIX) + ".boatState";
    SKOutput<String>(boat_state_path, "", new SKMetadata("", "Boat operational state"))
        .set_input(String(boatStateToString(ecu.getBoatState())));

    // Publish periodic mute status
    String mute_path = String(SK_PATH_PREFIX) + ".periodicMuted";
    SKOutput<bool>(mute_path, "", new SKMetadata("", "Periodic signals muted"))
        .set_input(ecu.isPeriodicMuted());

    // Publish countdown
    String countdown_path = String(SK_PATH_PREFIX) + ".periodicCountdown";
    SKOutput<uint16_t>(countdown_path, "", new SKMetadata("s", "Seconds until next periodic signal"))
        .set_input(ecu.getPeriodicCountdownSeconds());

    // Publish individual light status
    LightConfiguration lights = ecu.getCurrentLights();
    String lights_prefix = String(SK_PATH_PREFIX) + ".lights.";
    
    SKOutput<bool>(lights_prefix + "mastheadLight", "", new SKMetadata("", "Masthead light"))
        .set_input(lights.masthead_light);
    SKOutput<bool>(lights_prefix + "portSidelight", "", new SKMetadata("", "Port sidelight"))
        .set_input(lights.port_sidelight);
    SKOutput<bool>(lights_prefix + "starboardSidelight", "", new SKMetadata("", "Starboard sidelight"))
        .set_input(lights.starboard_sidelight);
    SKOutput<bool>(lights_prefix + "sternlight", "", new SKMetadata("", "Sternlight"))
        .set_input(lights.sternlight);
    SKOutput<bool>(lights_prefix + "allroundWhite", "", new SKMetadata("", "All-round white light"))
        .set_input(lights.allround_white);
    SKOutput<bool>(lights_prefix + "allroundRedUpper", "", new SKMetadata("", "All-round red upper"))
        .set_input(lights.allround_red_upper);
    SKOutput<bool>(lights_prefix + "allroundRedLower", "", new SKMetadata("", "All-round red lower"))
        .set_input(lights.allround_red_lower);

    // Publish horn status
    String horn_path = String(SK_PATH_PREFIX) + ".horn.active";
    SKOutput<bool>(horn_path, "", new SKMetadata("", "Horn active"))
        .set_input(ecu.isHornActive());
}

// =======================================================================
// MAIN SETUP FUNCTION
// =======================================================================

void setupSignalK(NavigationLightsECU& ecu) {
    // Register PUT request listeners (inputs from SignalK UI)
    String condition_path = String(SK_PATH_PREFIX) + ".condition";
    new ConditionPutRequestListener(ecu, condition_path);

    String boat_state_path = String(SK_PATH_PREFIX) + ".boatState";
    new BoatStatePutRequestListener(ecu, boat_state_path);

    String mute_path = String(SK_PATH_PREFIX) + ".periodicMuted";
    new PeriodicMutePutRequestListener(ecu, mute_path);

    String ad_hoc_path = String(SK_PATH_PREFIX) + ".adHocSignal";
    new AdHocSignalPutRequestListener(ecu, ad_hoc_path);

    String emergency_path = String(SK_PATH_PREFIX) + ".emergencyStop";
    new EmergencyStopPutRequestListener(ecu, emergency_path);

    // Setup callback to publish status on state changes
    ecu.onStateChange([&ecu]() {
        publishECUStatus(ecu);
    });

    // Publish initial status
    publishECUStatus(ecu);
}
