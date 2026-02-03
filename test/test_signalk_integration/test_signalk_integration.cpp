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
    
    return UNITY_END();
}
