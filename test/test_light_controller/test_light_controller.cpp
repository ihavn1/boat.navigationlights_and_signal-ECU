/**
 * @file test_light_controller.cpp
 * @brief Unit tests for LightController
 * 
 * Tests light controller behavior with mock relay hardware.
 * Ensures correct relay activation based on COLREGs light configurations.
 */

#include <unity.h>
#include "MockRelayController.h"
#include "../src/LightController.h"

// Test fixtures
MockRelayController* mock_relay;
LightController* light_controller;

void setUp(void) {
    mock_relay = new MockRelayController();
    light_controller = new LightController(*mock_relay);
}

void tearDown(void) {
    delete light_controller;
    delete mock_relay;
}

// =============================================================================
// BASIC FUNCTIONALITY TESTS
// =============================================================================

void test_initial_state_all_lights_off(void) {
    // On construction, all lights should be off
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
    
    TEST_ASSERT_FALSE(light_controller->anyLightsActive());
}

void test_apply_single_light_configuration(void) {
    LightConfiguration config;
    config.masthead_light = true;
    
    light_controller->applyConfiguration(config);
    
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_TRUE(light_controller->anyLightsActive());
}

void test_apply_navigation_lights_configuration(void) {
    // Underway making way: masthead + sidelights + sternlight
    LightConfiguration config;
    config.masthead_light = true;
    config.port_sidelight = true;
    config.starboard_sidelight = true;
    config.sternlight = true;
    
    light_controller->applyConfiguration(config);
    
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
}

void test_apply_nuc_lights_configuration(void) {
    // NUC making way: red lights + sidelights + sternlight (no masthead)
    LightConfiguration config;
    config.allround_red_upper = true;
    config.allround_red_lower = true;
    config.port_sidelight = true;
    config.starboard_sidelight = true;
    config.sternlight = true;
    
    light_controller->applyConfiguration(config);
    
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
}

void test_apply_anchorage_light_configuration(void) {
    // Anchorage: single all-round white light
    LightConfiguration config;
    config.allround_white = true;
    
    light_controller->applyConfiguration(config);
    
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
}

// =============================================================================
// CONFIGURATION CHANGE TESTS
// =============================================================================

void test_change_configuration_turns_off_previous_lights(void) {
    // First configuration: navigation lights
    LightConfiguration config1;
    config1.masthead_light = true;
    config1.port_sidelight = true;
    config1.starboard_sidelight = true;
    config1.sternlight = true;
    light_controller->applyConfiguration(config1);
    
    // Change to anchorage: only all-round white
    LightConfiguration config2;
    config2.allround_white = true;
    light_controller->applyConfiguration(config2);
    
    // Previous lights should be off
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    
    // New light should be on
    TEST_ASSERT_TRUE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
}

void test_change_to_all_lights_off(void) {
    // Start with lights on
    LightConfiguration config1;
    config1.masthead_light = true;
    config1.sternlight = true;
    light_controller->applyConfiguration(config1);
    
    // Change to all off (daylight)
    LightConfiguration config2; // All false by default
    light_controller->applyConfiguration(config2);
    
    // All lights should be off
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
    
    TEST_ASSERT_FALSE(light_controller->anyLightsActive());
}

// =============================================================================
// SAFETY FUNCTION TESTS
// =============================================================================

void test_all_lights_off_function(void) {
    // Turn on some lights
    LightConfiguration config;
    config.masthead_light = true;
    config.port_sidelight = true;
    config.allround_white = true;
    light_controller->applyConfiguration(config);
    
    // Emergency off
    light_controller->allLightsOff();
    
    // All lights should be off
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(mock_relay->isActive(RelayChannel::ALLROUND_RED_LOWER));
    
    TEST_ASSERT_FALSE(light_controller->anyLightsActive());
}

// =============================================================================
// CONFIGURATION RETRIEVAL TESTS
// =============================================================================

void test_get_current_configuration(void) {
    LightConfiguration config;
    config.port_sidelight = true;
    config.starboard_sidelight = true;
    
    light_controller->applyConfiguration(config);
    
    LightConfiguration current = light_controller->getCurrentConfiguration();
    TEST_ASSERT_TRUE(current.port_sidelight);
    TEST_ASSERT_TRUE(current.starboard_sidelight);
    TEST_ASSERT_FALSE(current.masthead_light);
    TEST_ASSERT_FALSE(current.sternlight);
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Basic functionality
    RUN_TEST(test_initial_state_all_lights_off);
    RUN_TEST(test_apply_single_light_configuration);
    RUN_TEST(test_apply_navigation_lights_configuration);
    RUN_TEST(test_apply_nuc_lights_configuration);
    RUN_TEST(test_apply_anchorage_light_configuration);
    
    // Configuration changes
    RUN_TEST(test_change_configuration_turns_off_previous_lights);
    RUN_TEST(test_change_to_all_lights_off);
    
    // Safety functions
    RUN_TEST(test_all_lights_off_function);
    
    // Configuration retrieval
    RUN_TEST(test_get_current_configuration);
    
    return UNITY_END();
}
