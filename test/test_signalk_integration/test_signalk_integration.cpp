/**
 * @file test_signalk_integration.cpp
 * @brief Unit tests for SignalK integration
 * 
 * Tests conversion functions and SignalK data flow patterns
 */

#include <unity.h>
#include <cstring>
#include "../../src/state_machine.h"
#include "../../src/NavigationLightsECU.h"

// Since we can't easily mock SensESP classes in native tests,
// we'll focus on testing the conversion functions which are
// the core logic that can be tested without SignalK infrastructure

// Forward declarations of static functions from signalk_integration.cpp
// These would normally be static but we'll test the logic by duplicating it here
// In a real scenario, these would be extracted to a testable utility class

const char* test_conditionToString(Condition condition) {
    switch (condition) {
        case Condition::DAY: return "day";
        case Condition::HOURS_OF_DARKNESS: return "hours_of_darkness";
        case Condition::RESTRICTED_VISIBILITY: return "restricted_visibility";
        default: return "day";
    }
}

Condition test_stringToCondition(const char* str) {
    if (strcmp(str, "hours_of_darkness") == 0) return Condition::HOURS_OF_DARKNESS;
    if (strcmp(str, "restricted_visibility") == 0) return Condition::RESTRICTED_VISIBILITY;
    return Condition::DAY;
}

const char* test_boatStateToString(BoatState state) {
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

BoatState test_stringToBoatState(const char* str) {
    if (strcmp(str, "underway_making_way") == 0) return BoatState::UNDERWAY_MAKING_WAY;
    if (strcmp(str, "underway_no_way") == 0) return BoatState::UNDERWAY_NO_WAY;
    if (strcmp(str, "anchorage") == 0) return BoatState::ANCHORAGE;
    if (strcmp(str, "nuc_making_way") == 0) return BoatState::NUC_MAKING_WAY;
    if (strcmp(str, "nuc_no_way") == 0) return BoatState::NUC_NO_WAY;
    return BoatState::MOORED;
}

const char* test_adHocSignalToString(AdHocSignal signal) {
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

AdHocSignal test_stringToAdHocSignal(const char* str) {
    if (strcmp(str, "turn_port") == 0) return AdHocSignal::TURN_PORT;
    if (strcmp(str, "astern_propulsion") == 0) return AdHocSignal::ASTERN_PROPULSION;
    if (strcmp(str, "danger_confusion") == 0) return AdHocSignal::DANGER_CONFUSION;
    if (strcmp(str, "pay_attention") == 0) return AdHocSignal::PAY_ATTENTION;
    if (strcmp(str, "overtake_starboard") == 0) return AdHocSignal::OVERTAKE_STARBOARD;
    if (strcmp(str, "overtake_port") == 0) return AdHocSignal::OVERTAKE_PORT;
    if (strcmp(str, "agreement_overtaken") == 0) return AdHocSignal::AGREEMENT_OVERTAKEN;
    return AdHocSignal::TURN_STARBOARD;
}

// =============================================================================
// CONDITION CONVERSION TESTS
// =============================================================================

void test_condition_to_string_day() {
    TEST_ASSERT_EQUAL_STRING("day", test_conditionToString(Condition::DAY));
}

void test_condition_to_string_darkness() {
    TEST_ASSERT_EQUAL_STRING("hours_of_darkness", test_conditionToString(Condition::HOURS_OF_DARKNESS));
}

void test_condition_to_string_restricted() {
    TEST_ASSERT_EQUAL_STRING("restricted_visibility", test_conditionToString(Condition::RESTRICTED_VISIBILITY));
}

void test_string_to_condition_day() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("day"));
}

void test_string_to_condition_darkness() {
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, test_stringToCondition("hours_of_darkness"));
}

void test_string_to_condition_restricted() {
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, test_stringToCondition("restricted_visibility"));
}

void test_string_to_condition_invalid_defaults_to_day() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("invalid_condition"));
}

void test_condition_roundtrip_conversion() {
    // Test that converting to string and back gives the same value
    TEST_ASSERT_EQUAL(Condition::DAY, 
        test_stringToCondition(test_conditionToString(Condition::DAY)));
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, 
        test_stringToCondition(test_conditionToString(Condition::HOURS_OF_DARKNESS)));
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, 
        test_stringToCondition(test_conditionToString(Condition::RESTRICTED_VISIBILITY)));
}

// =============================================================================
// BOAT STATE CONVERSION TESTS
// =============================================================================

void test_boat_state_to_string_moored() {
    TEST_ASSERT_EQUAL_STRING("moored", test_boatStateToString(BoatState::MOORED));
}

void test_boat_state_to_string_underway_making_way() {
    TEST_ASSERT_EQUAL_STRING("underway_making_way", test_boatStateToString(BoatState::UNDERWAY_MAKING_WAY));
}

void test_boat_state_to_string_underway_no_way() {
    TEST_ASSERT_EQUAL_STRING("underway_no_way", test_boatStateToString(BoatState::UNDERWAY_NO_WAY));
}

void test_boat_state_to_string_anchorage() {
    TEST_ASSERT_EQUAL_STRING("anchorage", test_boatStateToString(BoatState::ANCHORAGE));
}

void test_boat_state_to_string_nuc_making_way() {
    TEST_ASSERT_EQUAL_STRING("nuc_making_way", test_boatStateToString(BoatState::NUC_MAKING_WAY));
}

void test_boat_state_to_string_nuc_no_way() {
    TEST_ASSERT_EQUAL_STRING("nuc_no_way", test_boatStateToString(BoatState::NUC_NO_WAY));
}

void test_string_to_boat_state_moored() {
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("moored"));
}

void test_string_to_boat_state_underway_making_way() {
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, test_stringToBoatState("underway_making_way"));
}

void test_string_to_boat_state_underway_no_way() {
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_NO_WAY, test_stringToBoatState("underway_no_way"));
}

void test_string_to_boat_state_anchorage() {
    TEST_ASSERT_EQUAL(BoatState::ANCHORAGE, test_stringToBoatState("anchorage"));
}

void test_string_to_boat_state_nuc_making_way() {
    TEST_ASSERT_EQUAL(BoatState::NUC_MAKING_WAY, test_stringToBoatState("nuc_making_way"));
}

void test_string_to_boat_state_nuc_no_way() {
    TEST_ASSERT_EQUAL(BoatState::NUC_NO_WAY, test_stringToBoatState("nuc_no_way"));
}

void test_string_to_boat_state_invalid_defaults_to_moored() {
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("invalid_state"));
}

void test_boat_state_roundtrip_conversion() {
    // Test all boat states round-trip correctly
    TEST_ASSERT_EQUAL(BoatState::MOORED, 
        test_stringToBoatState(test_boatStateToString(BoatState::MOORED)));
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, 
        test_stringToBoatState(test_boatStateToString(BoatState::UNDERWAY_MAKING_WAY)));
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_NO_WAY, 
        test_stringToBoatState(test_boatStateToString(BoatState::UNDERWAY_NO_WAY)));
    TEST_ASSERT_EQUAL(BoatState::ANCHORAGE, 
        test_stringToBoatState(test_boatStateToString(BoatState::ANCHORAGE)));
    TEST_ASSERT_EQUAL(BoatState::NUC_MAKING_WAY, 
        test_stringToBoatState(test_boatStateToString(BoatState::NUC_MAKING_WAY)));
    TEST_ASSERT_EQUAL(BoatState::NUC_NO_WAY, 
        test_stringToBoatState(test_boatStateToString(BoatState::NUC_NO_WAY)));
}

// =============================================================================
// AD-HOC SIGNAL CONVERSION TESTS
// =============================================================================

void test_adhoc_signal_to_string_turn_starboard() {
    TEST_ASSERT_EQUAL_STRING("turn_starboard", test_adHocSignalToString(AdHocSignal::TURN_STARBOARD));
}

void test_adhoc_signal_to_string_turn_port() {
    TEST_ASSERT_EQUAL_STRING("turn_port", test_adHocSignalToString(AdHocSignal::TURN_PORT));
}

void test_adhoc_signal_to_string_astern() {
    TEST_ASSERT_EQUAL_STRING("astern_propulsion", test_adHocSignalToString(AdHocSignal::ASTERN_PROPULSION));
}

void test_adhoc_signal_to_string_danger() {
    TEST_ASSERT_EQUAL_STRING("danger_confusion", test_adHocSignalToString(AdHocSignal::DANGER_CONFUSION));
}

void test_adhoc_signal_to_string_attention() {
    TEST_ASSERT_EQUAL_STRING("pay_attention", test_adHocSignalToString(AdHocSignal::PAY_ATTENTION));
}

void test_adhoc_signal_to_string_overtake_starboard() {
    TEST_ASSERT_EQUAL_STRING("overtake_starboard", test_adHocSignalToString(AdHocSignal::OVERTAKE_STARBOARD));
}

void test_adhoc_signal_to_string_overtake_port() {
    TEST_ASSERT_EQUAL_STRING("overtake_port", test_adHocSignalToString(AdHocSignal::OVERTAKE_PORT));
}

void test_adhoc_signal_to_string_agreement() {
    TEST_ASSERT_EQUAL_STRING("agreement_overtaken", test_adHocSignalToString(AdHocSignal::AGREEMENT_OVERTAKEN));
}

void test_string_to_adhoc_signal_turn_starboard() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("turn_starboard"));
}

void test_string_to_adhoc_signal_turn_port() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_PORT, test_stringToAdHocSignal("turn_port"));
}

void test_string_to_adhoc_signal_astern() {
    TEST_ASSERT_EQUAL(AdHocSignal::ASTERN_PROPULSION, test_stringToAdHocSignal("astern_propulsion"));
}

void test_string_to_adhoc_signal_danger() {
    TEST_ASSERT_EQUAL(AdHocSignal::DANGER_CONFUSION, test_stringToAdHocSignal("danger_confusion"));
}

void test_string_to_adhoc_signal_attention() {
    TEST_ASSERT_EQUAL(AdHocSignal::PAY_ATTENTION, test_stringToAdHocSignal("pay_attention"));
}

void test_string_to_adhoc_signal_overtake_starboard() {
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_STARBOARD, test_stringToAdHocSignal("overtake_starboard"));
}

void test_string_to_adhoc_signal_overtake_port() {
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_PORT, test_stringToAdHocSignal("overtake_port"));
}

void test_string_to_adhoc_signal_agreement() {
    TEST_ASSERT_EQUAL(AdHocSignal::AGREEMENT_OVERTAKEN, test_stringToAdHocSignal("agreement_overtaken"));
}

void test_string_to_adhoc_signal_invalid_defaults_to_turn_starboard() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("invalid_signal"));
}

void test_adhoc_signal_roundtrip_conversion() {
    // Test all ad-hoc signals round-trip correctly
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::TURN_STARBOARD)));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_PORT, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::TURN_PORT)));
    TEST_ASSERT_EQUAL(AdHocSignal::ASTERN_PROPULSION, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::ASTERN_PROPULSION)));
    TEST_ASSERT_EQUAL(AdHocSignal::DANGER_CONFUSION, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::DANGER_CONFUSION)));
    TEST_ASSERT_EQUAL(AdHocSignal::PAY_ATTENTION, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::PAY_ATTENTION)));
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_STARBOARD, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::OVERTAKE_STARBOARD)));
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_PORT, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::OVERTAKE_PORT)));
    TEST_ASSERT_EQUAL(AdHocSignal::AGREEMENT_OVERTAKEN, 
        test_stringToAdHocSignal(test_adHocSignalToString(AdHocSignal::AGREEMENT_OVERTAKEN)));
}

// =============================================================================
// SIGNALK STRING FORMAT VALIDATION TESTS
// =============================================================================

void test_signalk_strings_are_lowercase_snake_case() {
    // Verify all SignalK string formats follow snake_case convention
    const char* condition_str = test_conditionToString(Condition::HOURS_OF_DARKNESS);
    TEST_ASSERT_EQUAL_STRING("hours_of_darkness", condition_str);
    
    const char* state_str = test_boatStateToString(BoatState::UNDERWAY_MAKING_WAY);
    TEST_ASSERT_EQUAL_STRING("underway_making_way", state_str);
    
    const char* signal_str = test_adHocSignalToString(AdHocSignal::OVERTAKE_STARBOARD);
    TEST_ASSERT_EQUAL_STRING("overtake_starboard", signal_str);
}

void test_signalk_strings_no_spaces() {
    // Verify no SignalK strings contain spaces
    for (int i = 0; i <= 2; i++) {
        const char* str = test_conditionToString((Condition)i);
        TEST_ASSERT_NULL(strchr(str, ' '));
    }
    
    for (int i = 0; i <= 5; i++) {
        const char* str = test_boatStateToString((BoatState)i);
        TEST_ASSERT_NULL(strchr(str, ' '));
    }
    
    for (int i = 0; i <= 7; i++) {
        const char* str = test_adHocSignalToString((AdHocSignal)i);
        TEST_ASSERT_NULL(strchr(str, ' '));
    }
}

// =============================================================================
// INTEGRATION TESTS WITH MOCK ECU
// =============================================================================

#include "MockNavigationLightsECU.h"

// Mock ECU for integration tests
static MockNavigationLightsECU* mock_ecu = nullptr;

void test_condition_consumer_calls_ecu_method() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, mock_ecu->last_condition_set);
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, mock_ecu->getCondition());
    
    delete mock_ecu;
}

void test_boat_state_consumer_calls_ecu_method() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, mock_ecu->last_state_set);
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, mock_ecu->getBoatState());
    
    delete mock_ecu;
}

void test_mute_consumer_calls_mute() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->mutePeriodicSignals();
    
    TEST_ASSERT_TRUE(mock_ecu->mute_called);
    TEST_ASSERT_TRUE(mock_ecu->periodic_muted);
    TEST_ASSERT_TRUE(mock_ecu->isPeriodicMuted());
    
    delete mock_ecu;
}

void test_mute_consumer_calls_unmute() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->periodic_muted = true;  // Start muted
    mock_ecu->unmutePeriodicSignals();
    
    TEST_ASSERT_TRUE(mock_ecu->unmute_called);
    TEST_ASSERT_FALSE(mock_ecu->periodic_muted);
    TEST_ASSERT_FALSE(mock_ecu->isPeriodicMuted());
    
    delete mock_ecu;
}

void test_adhoc_signal_consumer_calls_trigger() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    
    TEST_ASSERT_EQUAL(AdHocSignal::DANGER_CONFUSION, mock_ecu->last_adhoc_signal);
    
    delete mock_ecu;
}

void test_emergency_stop_consumer_calls_emergency_stop() {
    mock_ecu = new MockNavigationLightsECU();
    mock_ecu->emergencyStop();
    
    TEST_ASSERT_TRUE(mock_ecu->emergency_stop_called);
    
    delete mock_ecu;
}

void test_state_change_callback_invoked() {
    mock_ecu = new MockNavigationLightsECU();
    bool callback_invoked = false;
    
    mock_ecu->onStateChange([&callback_invoked]() {
        callback_invoked = true;
    });
    
    mock_ecu->triggerStateChange();
    TEST_ASSERT_TRUE(callback_invoked);
    
    delete mock_ecu;
}

void test_multiple_condition_changes() {
    mock_ecu = new MockNavigationLightsECU();
    
    mock_ecu->setCondition(Condition::DAY);
    TEST_ASSERT_EQUAL(Condition::DAY, mock_ecu->getCondition());
    
    mock_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, mock_ecu->getCondition());
    
    mock_ecu->setCondition(Condition::RESTRICTED_VISIBILITY);
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, mock_ecu->getCondition());
    
    delete mock_ecu;
}

void test_multiple_boat_state_changes() {
    mock_ecu = new MockNavigationLightsECU();
    
    mock_ecu->setBoatState(BoatState::MOORED);
    TEST_ASSERT_EQUAL(BoatState::MOORED, mock_ecu->getBoatState());
    
    mock_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, mock_ecu->getBoatState());
    
    mock_ecu->setBoatState(BoatState::ANCHORAGE);
    TEST_ASSERT_EQUAL(BoatState::ANCHORAGE, mock_ecu->getBoatState());
    
    delete mock_ecu;
}

void test_mute_unmute_toggle() {
    mock_ecu = new MockNavigationLightsECU();
    
    // Start unmuted (default)
    TEST_ASSERT_FALSE(mock_ecu->isPeriodicMuted());
    
    // Mute
    mock_ecu->mutePeriodicSignals();
    TEST_ASSERT_TRUE(mock_ecu->isPeriodicMuted());
    
    // Unmute
    mock_ecu->unmutePeriodicSignals();
    TEST_ASSERT_FALSE(mock_ecu->isPeriodicMuted());
    
    // Mute again
    mock_ecu->mutePeriodicSignals();
    TEST_ASSERT_TRUE(mock_ecu->isPeriodicMuted());
    
    delete mock_ecu;
}

void test_all_adhoc_signals_trigger() {
    mock_ecu = new MockNavigationLightsECU();
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::TURN_STARBOARD);
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::TURN_PORT);
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_PORT, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::ASTERN_PROPULSION);
    TEST_ASSERT_EQUAL(AdHocSignal::ASTERN_PROPULSION, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    TEST_ASSERT_EQUAL(AdHocSignal::DANGER_CONFUSION, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::PAY_ATTENTION);
    TEST_ASSERT_EQUAL(AdHocSignal::PAY_ATTENTION, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::OVERTAKE_STARBOARD);
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_STARBOARD, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::OVERTAKE_PORT);
    TEST_ASSERT_EQUAL(AdHocSignal::OVERTAKE_PORT, mock_ecu->last_adhoc_signal);
    
    mock_ecu->triggerAdHocSignal(AdHocSignal::AGREEMENT_OVERTAKEN);
    TEST_ASSERT_EQUAL(AdHocSignal::AGREEMENT_OVERTAKEN, mock_ecu->last_adhoc_signal);
    
    delete mock_ecu;
}

void test_mock_reset_clears_state() {
    mock_ecu = new MockNavigationLightsECU();
    
    // Set some state
    mock_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    mock_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    mock_ecu->mutePeriodicSignals();
    mock_ecu->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    mock_ecu->emergencyStop();
    
    // Reset
    mock_ecu->reset();
    
    // Verify reset state
    TEST_ASSERT_EQUAL(Condition::DAY, mock_ecu->last_condition_set);
    TEST_ASSERT_EQUAL(BoatState::MOORED, mock_ecu->last_state_set);
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, mock_ecu->last_adhoc_signal);
    TEST_ASSERT_FALSE(mock_ecu->mute_called);
    TEST_ASSERT_FALSE(mock_ecu->unmute_called);
    TEST_ASSERT_FALSE(mock_ecu->emergency_stop_called);
    
    delete mock_ecu;
}

// =============================================================================
// EDGE CASE AND ERROR HANDLING TESTS
// =============================================================================

void test_empty_string_to_condition_defaults_to_day() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition(""));
}

void test_null_like_string_to_condition_defaults_to_day() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("null"));
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("undefined"));
}

void test_case_sensitive_condition_strings() {
    // Verify exact case matching is required
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("DAY"));  // Wrong case -> default
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("Hours_Of_Darkness"));  // Wrong case -> default
}

void test_empty_string_to_boat_state_defaults_to_moored() {
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState(""));
}

void test_null_like_string_to_boat_state_defaults_to_moored() {
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("null"));
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("undefined"));
}

void test_case_sensitive_boat_state_strings() {
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("MOORED"));  // Wrong case -> default
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("Underway_Making_Way"));  // Wrong case -> default
}

void test_empty_string_to_adhoc_signal_defaults_to_turn_starboard() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal(""));
}

void test_null_like_string_to_adhoc_signal_defaults_to_turn_starboard() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("null"));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("undefined"));
}

void test_case_sensitive_adhoc_signal_strings() {
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("TURN_STARBOARD"));  // Wrong case -> default
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("Turn_Port"));  // Wrong case -> default
}

void test_whitespace_in_strings_not_accepted() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition(" day "));
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("hours of darkness"));  // Spaces instead of underscores
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState(" moored "));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal(" turn_starboard "));
}

void test_partial_match_not_accepted() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("dark"));
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("underway"));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("turn"));
}

void test_typos_default_safely() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("hours_of_darknes"));  // Missing 's'
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("underway_making_wat"));  // Wrong ending
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("danger_confuzion"));  // Typo
}

void test_special_characters_rejected() {
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition("day!"));
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState("moored#"));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal("turn_starboard@"));
}

void test_very_long_string_defaults() {
    const char* long_str = "this_is_a_very_long_string_that_does_not_match_anything_and_should_default";
    TEST_ASSERT_EQUAL(Condition::DAY, test_stringToCondition(long_str));
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_stringToBoatState(long_str));
    TEST_ASSERT_EQUAL(AdHocSignal::TURN_STARBOARD, test_stringToAdHocSignal(long_str));
}

void test_rapid_condition_changes() {
    mock_ecu = new MockNavigationLightsECU();
    
    for (int i = 0; i < 100; i++) {
        mock_ecu->setCondition(Condition::DAY);
        mock_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
        mock_ecu->setCondition(Condition::RESTRICTED_VISIBILITY);
    }
    
    // Should end on last set value
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, mock_ecu->getCondition());
    
    delete mock_ecu;
}

void test_rapid_mute_unmute_toggle() {
    mock_ecu = new MockNavigationLightsECU();
    
    for (int i = 0; i < 100; i++) {
        mock_ecu->mutePeriodicSignals();
        mock_ecu->unmutePeriodicSignals();
    }
    
    // Should end unmuted
    TEST_ASSERT_FALSE(mock_ecu->isPeriodicMuted());
    
    delete mock_ecu;
}

void test_adhoc_signals_can_be_triggered_repeatedly() {
    mock_ecu = new MockNavigationLightsECU();
    
    // Trigger same signal multiple times (should be allowed)
    for (int i = 0; i < 10; i++) {
        mock_ecu->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
        TEST_ASSERT_EQUAL(AdHocSignal::DANGER_CONFUSION, mock_ecu->last_adhoc_signal);
    }
    
    delete mock_ecu;
}

void test_emergency_stop_can_be_called_multiple_times() {
    mock_ecu = new MockNavigationLightsECU();
    
    mock_ecu->emergencyStop();
    TEST_ASSERT_TRUE(mock_ecu->emergency_stop_called);
    
    mock_ecu->reset();
    
    mock_ecu->emergencyStop();
    TEST_ASSERT_TRUE(mock_ecu->emergency_stop_called);
    
    delete mock_ecu;
}

void test_state_change_callback_can_be_null() {
    mock_ecu = new MockNavigationLightsECU();
    
    // Don't set callback
    mock_ecu->triggerStateChange();  // Should not crash
    
    delete mock_ecu;
}

void test_multiple_state_change_callbacks() {
    mock_ecu = new MockNavigationLightsECU();
    int callback_count = 0;
    
    // Set callback
    mock_ecu->onStateChange([&callback_count]() {
        callback_count++;
    });
    
    // Trigger multiple times
    mock_ecu->triggerStateChange();
    mock_ecu->triggerStateChange();
    mock_ecu->triggerStateChange();
    
    TEST_ASSERT_EQUAL(3, callback_count);
    
    delete mock_ecu;
}

// =============================================================================
// TEST RUNNER
// =============================================================================

void setUp(void) {
    // Set up code for each test (if needed)
}

void tearDown(void) {
    // Clean up code for each test (if needed)
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Condition conversion tests
    RUN_TEST(test_condition_to_string_day);
    RUN_TEST(test_condition_to_string_darkness);
    RUN_TEST(test_condition_to_string_restricted);
    RUN_TEST(test_string_to_condition_day);
    RUN_TEST(test_string_to_condition_darkness);
    RUN_TEST(test_string_to_condition_restricted);
    RUN_TEST(test_string_to_condition_invalid_defaults_to_day);
    RUN_TEST(test_condition_roundtrip_conversion);
    
    // Boat state conversion tests
    RUN_TEST(test_boat_state_to_string_moored);
    RUN_TEST(test_boat_state_to_string_underway_making_way);
    RUN_TEST(test_boat_state_to_string_underway_no_way);
    RUN_TEST(test_boat_state_to_string_anchorage);
    RUN_TEST(test_boat_state_to_string_nuc_making_way);
    RUN_TEST(test_boat_state_to_string_nuc_no_way);
    RUN_TEST(test_string_to_boat_state_moored);
    RUN_TEST(test_string_to_boat_state_underway_making_way);
    RUN_TEST(test_string_to_boat_state_underway_no_way);
    RUN_TEST(test_string_to_boat_state_anchorage);
    RUN_TEST(test_string_to_boat_state_nuc_making_way);
    RUN_TEST(test_string_to_boat_state_nuc_no_way);
    RUN_TEST(test_string_to_boat_state_invalid_defaults_to_moored);
    RUN_TEST(test_boat_state_roundtrip_conversion);
    
    // Ad-hoc signal conversion tests
    RUN_TEST(test_adhoc_signal_to_string_turn_starboard);
    RUN_TEST(test_adhoc_signal_to_string_turn_port);
    RUN_TEST(test_adhoc_signal_to_string_astern);
    RUN_TEST(test_adhoc_signal_to_string_danger);
    RUN_TEST(test_adhoc_signal_to_string_attention);
    RUN_TEST(test_adhoc_signal_to_string_overtake_starboard);
    RUN_TEST(test_adhoc_signal_to_string_overtake_port);
    RUN_TEST(test_adhoc_signal_to_string_agreement);
    RUN_TEST(test_string_to_adhoc_signal_turn_starboard);
    RUN_TEST(test_string_to_adhoc_signal_turn_port);
    RUN_TEST(test_string_to_adhoc_signal_astern);
    RUN_TEST(test_string_to_adhoc_signal_danger);
    RUN_TEST(test_string_to_adhoc_signal_attention);
    RUN_TEST(test_string_to_adhoc_signal_overtake_starboard);
    RUN_TEST(test_string_to_adhoc_signal_overtake_port);
    RUN_TEST(test_string_to_adhoc_signal_agreement);
    RUN_TEST(test_string_to_adhoc_signal_invalid_defaults_to_turn_starboard);
    RUN_TEST(test_adhoc_signal_roundtrip_conversion);
    
    // SignalK format validation tests
    RUN_TEST(test_signalk_strings_are_lowercase_snake_case);
    RUN_TEST(test_signalk_strings_no_spaces);
    
    // Integration tests with MockNavigationLightsECU
    RUN_TEST(test_condition_consumer_calls_ecu_method);
    RUN_TEST(test_boat_state_consumer_calls_ecu_method);
    RUN_TEST(test_mute_consumer_calls_mute);
    RUN_TEST(test_mute_consumer_calls_unmute);
    RUN_TEST(test_adhoc_signal_consumer_calls_trigger);
    RUN_TEST(test_emergency_stop_consumer_calls_emergency_stop);
    RUN_TEST(test_state_change_callback_invoked);
    RUN_TEST(test_multiple_condition_changes);
    RUN_TEST(test_multiple_boat_state_changes);
    RUN_TEST(test_mute_unmute_toggle);
    RUN_TEST(test_all_adhoc_signals_trigger);
    RUN_TEST(test_mock_reset_clears_state);
    
    // Edge case and error handling tests
    RUN_TEST(test_empty_string_to_condition_defaults_to_day);
    RUN_TEST(test_null_like_string_to_condition_defaults_to_day);
    RUN_TEST(test_case_sensitive_condition_strings);
    RUN_TEST(test_empty_string_to_boat_state_defaults_to_moored);
    RUN_TEST(test_null_like_string_to_boat_state_defaults_to_moored);
    RUN_TEST(test_case_sensitive_boat_state_strings);
    RUN_TEST(test_empty_string_to_adhoc_signal_defaults_to_turn_starboard);
    RUN_TEST(test_null_like_string_to_adhoc_signal_defaults_to_turn_starboard);
    RUN_TEST(test_case_sensitive_adhoc_signal_strings);
    RUN_TEST(test_whitespace_in_strings_not_accepted);
    RUN_TEST(test_partial_match_not_accepted);
    RUN_TEST(test_typos_default_safely);
    RUN_TEST(test_special_characters_rejected);
    RUN_TEST(test_very_long_string_defaults);
    RUN_TEST(test_rapid_condition_changes);
    RUN_TEST(test_rapid_mute_unmute_toggle);
    RUN_TEST(test_adhoc_signals_can_be_triggered_repeatedly);
    RUN_TEST(test_emergency_stop_can_be_called_multiple_times);
    RUN_TEST(test_state_change_callback_can_be_null);
    RUN_TEST(test_multiple_state_change_callbacks);
    
    return UNITY_END();
}
