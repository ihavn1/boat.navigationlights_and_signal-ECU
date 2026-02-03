/**
 * @file test_state_machine.cpp
 * @brief Unit tests for COLREGs state machine
 * 
 * Tests all condition×state combinations against COLREGs requirements.
 * Uses TDD approach - tests written before implementation.
 */

#include <unity.h>
#include "../src/state_machine.h"

// Test fixtures
StateMachine* state_machine;

void setUp(void) {
    state_machine = new StateMachine();
}

void tearDown(void) {
    delete state_machine;
}

// =============================================================================
// DAYLIGHT TESTS (Rule 20 - lights not required)
// =============================================================================

void test_day_moored_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::MOORED);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    TEST_ASSERT_FALSE(lights.allround_white);
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_day_underway_no_way_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::UNDERWAY_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_day_underway_making_way_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_day_anchorage_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::ANCHORAGE);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.allround_white);
    // Day anchorage shows black ball shape, not lights
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_day_nuc_no_way_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::NUC_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    // Day NUC shows two black balls, not lights
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_day_nuc_making_way_no_lights(void) {
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::NUC_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    // Day NUC shows two black balls, not lights
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

// =============================================================================
// HOURS OF DARKNESS TESTS (Rule 20, 23, 25, 27, 30)
// =============================================================================

void test_darkness_moored_no_lights(void) {
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::MOORED);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    // Moored vessels don't show navigation lights
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_darkness_underway_no_way_navigation_lights(void) {
    // Rule 23: Power-driven vessel underway (even if not making way through water)
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::UNDERWAY_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.masthead_light);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_darkness_underway_making_way_navigation_lights(void) {
    // Rule 23: Power-driven vessel underway
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.masthead_light);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_darkness_anchorage_allround_white(void) {
    // Rule 30: Anchored vessel <50m shows one all-round white light
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::ANCHORAGE);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_white);
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_darkness_nuc_no_way_red_lights_only(void) {
    // Rule 27: NUC vessel shows two all-round red lights in vertical line
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::NUC_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_red_upper);
    TEST_ASSERT_TRUE(lights.allround_red_lower);
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_darkness_nuc_making_way_red_and_navigation_lights(void) {
    // Rule 27: NUC vessel making way shows red lights + sidelights + sternlight
    state_machine->setCondition(Condition::HOURS_OF_DARKNESS);
    state_machine->setBoatState(BoatState::NUC_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_red_upper);
    TEST_ASSERT_TRUE(lights.allround_red_lower);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    TEST_ASSERT_FALSE(lights.masthead_light); // NUC doesn't show masthead light
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

// =============================================================================
// RESTRICTED VISIBILITY TESTS (Rule 35 - sound signals required)
// =============================================================================

void test_restricted_visibility_moored_no_signal(void) {
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::MOORED);
    
    // Moored vessel doesn't sound signals
    TEST_ASSERT_EQUAL(SoundSignalPattern::NONE, state_machine->getPeriodicSoundSignal());
}

void test_restricted_visibility_underway_no_way_signal(void) {
    // Rule 35(c): Power-driven vessel underway but stopped - prolonged blast every 2min
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::UNDERWAY_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.masthead_light);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, state_machine->getPeriodicSoundSignal());
    TEST_ASSERT_EQUAL(120, state_machine->getPeriodicSignalIntervalSeconds());
}

void test_restricted_visibility_underway_making_way_signal(void) {
    // Rule 35(a): Power-driven vessel making way - prolonged blast every 2min
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.masthead_light);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, state_machine->getPeriodicSoundSignal());
    TEST_ASSERT_EQUAL(120, state_machine->getPeriodicSignalIntervalSeconds());
}

void test_restricted_visibility_anchorage_warning_signal(void) {
    // Rule 35(g): Anchored vessel - rapid ringing of bell, simplified as short-prolonged-short
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::ANCHORAGE);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_white);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::SHORT_PROLONGED_SHORT, state_machine->getPeriodicSoundSignal());
}

void test_restricted_visibility_nuc_no_way_signal(void) {
    // Rule 35(c): NUC vessel - prolonged + 2 short blasts every 2min
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::NUC_NO_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_red_upper);
    TEST_ASSERT_TRUE(lights.allround_red_lower);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN, state_machine->getPeriodicSoundSignal());
    TEST_ASSERT_EQUAL(120, state_machine->getPeriodicSignalIntervalSeconds());
}

void test_restricted_visibility_nuc_making_way_signal(void) {
    // Rule 35(c): NUC vessel - prolonged + 2 short blasts every 2min
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    state_machine->setBoatState(BoatState::NUC_MAKING_WAY);
    
    LightConfiguration lights = state_machine->getRequiredLights();
    TEST_ASSERT_TRUE(lights.allround_red_upper);
    TEST_ASSERT_TRUE(lights.allround_red_lower);
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    
    TEST_ASSERT_EQUAL(SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN, state_machine->getPeriodicSoundSignal());
    TEST_ASSERT_EQUAL(120, state_machine->getPeriodicSignalIntervalSeconds());
}

// =============================================================================
// STATE TRANSITION TESTS
// =============================================================================

void test_state_transitions(void) {
    // Verify state can be changed and retrieved correctly
    state_machine->setCondition(Condition::DAY);
    state_machine->setBoatState(BoatState::MOORED);
    TEST_ASSERT_EQUAL(Condition::DAY, state_machine->getCondition());
    TEST_ASSERT_EQUAL(BoatState::MOORED, state_machine->getBoatState());
    
    state_machine->setCondition(Condition::RESTRICTED_VISIBILITY);
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, state_machine->getCondition());
    
    state_machine->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, state_machine->getBoatState());
}

void test_initial_state(void) {
    // State machine should have default safe state on construction
    TEST_ASSERT_EQUAL(Condition::DAY, state_machine->getCondition());
    TEST_ASSERT_EQUAL(BoatState::MOORED, state_machine->getBoatState());
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Daylight tests
    RUN_TEST(test_day_moored_no_lights);
    RUN_TEST(test_day_underway_no_way_no_lights);
    RUN_TEST(test_day_underway_making_way_no_lights);
    RUN_TEST(test_day_anchorage_no_lights);
    RUN_TEST(test_day_nuc_no_way_no_lights);
    RUN_TEST(test_day_nuc_making_way_no_lights);
    
    // Hours of darkness tests
    RUN_TEST(test_darkness_moored_no_lights);
    RUN_TEST(test_darkness_underway_no_way_navigation_lights);
    RUN_TEST(test_darkness_underway_making_way_navigation_lights);
    RUN_TEST(test_darkness_anchorage_allround_white);
    RUN_TEST(test_darkness_nuc_no_way_red_lights_only);
    RUN_TEST(test_darkness_nuc_making_way_red_and_navigation_lights);
    
    // Restricted visibility tests
    RUN_TEST(test_restricted_visibility_moored_no_signal);
    RUN_TEST(test_restricted_visibility_underway_no_way_signal);
    RUN_TEST(test_restricted_visibility_underway_making_way_signal);
    RUN_TEST(test_restricted_visibility_anchorage_warning_signal);
    RUN_TEST(test_restricted_visibility_nuc_no_way_signal);
    RUN_TEST(test_restricted_visibility_nuc_making_way_signal);
    
    // State transition tests
    RUN_TEST(test_initial_state);
    RUN_TEST(test_state_transitions);
    
    return UNITY_END();
}
