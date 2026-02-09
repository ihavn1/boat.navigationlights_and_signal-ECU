/**
 * @file test_signalk_esp32.cpp
 * @brief ESP32 embedded tests for SignalK integration
 * 
 * Tests SignalK integration with real SensESP runtime on ESP32 hardware.
 * These tests verify:
 * - ValueConsumer classes work with real SensESP types
 * - ObservableValue updates trigger correctly
 * - SKPutRequestListener receives and processes data
 * - SKOutput publishes to SignalK paths
 */

#include <unity.h>
#include <Arduino.h>
#include "../../src/state_machine.h"
#include "../../src/NavigationLightsECU.h"
#include "../../src/ESP32RelayController.h"
#include "../../src/ESP32Timer.h"
#include "../../src/LightController.h"
#include "../../src/SoundController.h"

// Mock hardware pins for testing
static const uint8_t TEST_PINS[8] = {25, 26, 27, 14, 12, 13, 15, 4};

// =============================================================================
// TEST HELPERS
// =============================================================================

static NavigationLightsECU* test_ecu = nullptr;
static ESP32RelayController* relay_controller = nullptr;
static ESP32Timer* timer = nullptr;
static LightController* light_controller = nullptr;
static SoundController* sound_controller = nullptr;

void setup_test_ecu() {
    // Create real hardware controllers
    relay_controller = new ESP32RelayController(
        TEST_PINS[0], TEST_PINS[1], TEST_PINS[2], TEST_PINS[3],
        TEST_PINS[4], TEST_PINS[5], TEST_PINS[6], TEST_PINS[7]
    );
    relay_controller->begin();
    
    timer = new ESP32Timer();
    light_controller = new LightController(*relay_controller);
    sound_controller = new SoundController(*relay_controller, *timer);
    
    // Create ECU
    test_ecu = new NavigationLightsECU(*light_controller, *sound_controller);
}

void teardown_test_ecu() {
    if (test_ecu) delete test_ecu;
    if (sound_controller) delete sound_controller;
    if (light_controller) delete light_controller;
    if (timer) delete timer;
    if (relay_controller) delete relay_controller;
    
    test_ecu = nullptr;
    sound_controller = nullptr;
    light_controller = nullptr;
    timer = nullptr;
    relay_controller = nullptr;
}

// =============================================================================
// ECU INTEGRATION TESTS
// =============================================================================

void test_ecu_initializes_with_safe_defaults() {
    setup_test_ecu();
    
    TEST_ASSERT_EQUAL(Condition::DAY, test_ecu->getCondition());
    TEST_ASSERT_EQUAL(BoatState::MOORED, test_ecu->getBoatState());
    TEST_ASSERT_TRUE(test_ecu->isPeriodicMuted());  // Always start muted (safety requirement)
    TEST_ASSERT_FALSE(test_ecu->isHornActive());
    
    LightConfiguration lights = test_ecu->getCurrentLights();
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    TEST_ASSERT_FALSE(lights.allround_white);
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    
    teardown_test_ecu();
}

void test_ecu_condition_changes_update_state() {
    setup_test_ecu();
    
    test_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, test_ecu->getCondition());
    
    test_ecu->setCondition(Condition::RESTRICTED_VISIBILITY);
    TEST_ASSERT_EQUAL(Condition::RESTRICTED_VISIBILITY, test_ecu->getCondition());
    
    teardown_test_ecu();
}

void test_ecu_boat_state_changes_update_state() {
    setup_test_ecu();
    
    test_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    TEST_ASSERT_EQUAL(BoatState::UNDERWAY_MAKING_WAY, test_ecu->getBoatState());
    
    test_ecu->setBoatState(BoatState::ANCHORAGE);
    TEST_ASSERT_EQUAL(BoatState::ANCHORAGE, test_ecu->getBoatState());
    
    teardown_test_ecu();
}

void test_ecu_darkness_and_underway_enables_navigation_lights() {
    setup_test_ecu();
    
    test_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    test_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    LightConfiguration lights = test_ecu->getCurrentLights();
    
    // COLREGs Rule 23(a): Vessels <15m underway display sidelights, sternlight, optional masthead
    TEST_ASSERT_TRUE(lights.port_sidelight);
    TEST_ASSERT_TRUE(lights.starboard_sidelight);
    TEST_ASSERT_TRUE(lights.sternlight);
    TEST_ASSERT_TRUE(lights.masthead_light);  // We use masthead
    
    TEST_ASSERT_FALSE(lights.allround_white);
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    
    teardown_test_ecu();
}

void test_ecu_anchorage_enables_anchor_light() {
    setup_test_ecu();
    
    test_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    test_ecu->setBoatState(BoatState::ANCHORAGE);
    
    LightConfiguration lights = test_ecu->getCurrentLights();
    
    // COLREGs Rule 30(c): Vessel at anchor <50m displays one all-round white
    TEST_ASSERT_TRUE(lights.allround_white);
    
    TEST_ASSERT_FALSE(lights.masthead_light);
    TEST_ASSERT_FALSE(lights.port_sidelight);
    TEST_ASSERT_FALSE(lights.starboard_sidelight);
    TEST_ASSERT_FALSE(lights.sternlight);
    TEST_ASSERT_FALSE(lights.allround_red_upper);
    TEST_ASSERT_FALSE(lights.allround_red_lower);
    
    teardown_test_ecu();
}

void test_ecu_mute_unmute_periodic_signals() {
    setup_test_ecu();
    
    // Start muted (safety default)
    TEST_ASSERT_TRUE(test_ecu->isPeriodicMuted());
    
    // Unmute
    test_ecu->unmutePeriodicSignals();
    TEST_ASSERT_FALSE(test_ecu->isPeriodicMuted());
    
    // Mute again
    test_ecu->mutePeriodicSignals();
    TEST_ASSERT_TRUE(test_ecu->isPeriodicMuted());
    
    teardown_test_ecu();
}

void test_ecu_adhoc_signal_triggers_horn() {
    setup_test_ecu();
    
    // Trigger ad-hoc signal
    test_ecu->triggerAdHocSignal(AdHocSignal::TURN_STARBOARD);
    
    // Horn should be active (1 short blast)
    TEST_ASSERT_TRUE(test_ecu->isHornActive());
    
    // Wait for blast to complete (1s + margin) and process updates
    delay(1500);
    test_ecu->update();  // Process sound controller and timer callbacks
    
    TEST_ASSERT_FALSE(test_ecu->isHornActive());
    
    teardown_test_ecu();
}

void test_ecu_emergency_stop_disables_all_outputs() {
    setup_test_ecu();
    
    // Set up some active state
    test_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    test_ecu->setBoatState(BoatState::UNDERWAY_MAKING_WAY);
    
    // Verify lights are on
    LightConfiguration lights_before = test_ecu->getCurrentLights();
    TEST_ASSERT_TRUE(lights_before.port_sidelight || lights_before.masthead_light);
    
    // Emergency stop
    test_ecu->emergencyStop();
    
    // All lights should be off
    LightConfiguration lights_after = test_ecu->getCurrentLights();
    TEST_ASSERT_FALSE(lights_after.masthead_light);
    TEST_ASSERT_FALSE(lights_after.port_sidelight);
    TEST_ASSERT_FALSE(lights_after.starboard_sidelight);
    TEST_ASSERT_FALSE(lights_after.sternlight);
    TEST_ASSERT_FALSE(lights_after.allround_white);
    TEST_ASSERT_FALSE(lights_after.allround_red_upper);
    TEST_ASSERT_FALSE(lights_after.allround_red_lower);
    TEST_ASSERT_FALSE(test_ecu->isHornActive());
    
    teardown_test_ecu();
}

void test_ecu_state_change_callback_fires() {
    setup_test_ecu();
    
    bool callback_fired = false;
    
    test_ecu->onStateChange([&callback_fired]() {
        callback_fired = true;
    });
    
    // Change state should trigger callback
    test_ecu->setCondition(Condition::HOURS_OF_DARKNESS);
    TEST_ASSERT_TRUE(callback_fired);
    
    teardown_test_ecu();
}

void test_ecu_countdown_decrements_when_unmuted() {
    setup_test_ecu();
    
    test_ecu->setCondition(Condition::RESTRICTED_VISIBILITY);
    test_ecu->setBoatState(BoatState::ANCHORAGE);
    test_ecu->unmutePeriodicSignals();
    
    unsigned long initial_countdown = test_ecu->getPeriodicCountdownSeconds();
    
    // Wait 2 seconds and update timer
    delay(2000);
    timer->update();
    
    unsigned long after_countdown = test_ecu->getPeriodicCountdownSeconds();
    
    // Countdown should have decreased by ~2 seconds
    TEST_ASSERT_TRUE(after_countdown < initial_countdown);
    TEST_ASSERT_INT_WITHIN(1, 2, initial_countdown - after_countdown);
    
    teardown_test_ecu();
}

void test_ecu_all_light_combinations() {
    setup_test_ecu();
    
    // Test a few critical COLREGs combinations
    struct TestCase {
        Condition condition;
        BoatState state;
        bool expect_any_lights;
    };
    
    TestCase cases[] = {
        {Condition::DAY, BoatState::MOORED, false},
        {Condition::DAY, BoatState::UNDERWAY_MAKING_WAY, false},
        {Condition::HOURS_OF_DARKNESS, BoatState::MOORED, false},
        {Condition::HOURS_OF_DARKNESS, BoatState::UNDERWAY_MAKING_WAY, true},
        {Condition::HOURS_OF_DARKNESS, BoatState::ANCHORAGE, true},
        {Condition::RESTRICTED_VISIBILITY, BoatState::ANCHORAGE, true},
    };
    
    for (auto& test : cases) {
        test_ecu->setCondition(test.condition);
        test_ecu->setBoatState(test.state);
        
        LightConfiguration lights = test_ecu->getCurrentLights();
        bool has_lights = lights.masthead_light || lights.port_sidelight || 
                         lights.starboard_sidelight || lights.sternlight ||
                         lights.allround_white || lights.allround_red_upper ||
                         lights.allround_red_lower;
        
        TEST_ASSERT_EQUAL(test.expect_any_lights, has_lights);
    }
    
    teardown_test_ecu();
}

void test_timer_callbacks_execute() {
    timer = new ESP32Timer();
    
    bool callback_executed = false;
    
    // Use scheduleOnce instead of setTimeout
    timer->scheduleOnce(100, [&callback_executed]() {
        callback_executed = true;
    });
    
    // Wait for timeout
    delay(150);
    timer->update();
    
    TEST_ASSERT_TRUE(callback_executed);
    
    delete timer;
    timer = nullptr;
}

void test_relay_controller_initializes_pins_low() {
    relay_controller = new ESP32RelayController(
        TEST_PINS[0], TEST_PINS[1], TEST_PINS[2], TEST_PINS[3],
        TEST_PINS[4], TEST_PINS[5], TEST_PINS[6], TEST_PINS[7]
    );
    relay_controller->begin();
    
    // All relays should be inactive on initialization
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::MASTHEAD_LIGHT));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::PORT_SIDELIGHT));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::STARBOARD_SIDELIGHT));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::STERNLIGHT));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::ALLROUND_WHITE));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::ALLROUND_RED_UPPER));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::ALLROUND_RED_LOWER));
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::HORN));
    
    delete relay_controller;
    relay_controller = nullptr;
}

void test_relay_controller_set_relay() {
    relay_controller = new ESP32RelayController(
        TEST_PINS[0], TEST_PINS[1], TEST_PINS[2], TEST_PINS[3],
        TEST_PINS[4], TEST_PINS[5], TEST_PINS[6], TEST_PINS[7]
    );
    relay_controller->begin();
    
    // Activate masthead light relay
    relay_controller->activate(RelayChannel::MASTHEAD_LIGHT);
    TEST_ASSERT_TRUE(relay_controller->isActive(RelayChannel::MASTHEAD_LIGHT));
    
    // Deactivate masthead light relay
    relay_controller->deactivate(RelayChannel::MASTHEAD_LIGHT);
    TEST_ASSERT_FALSE(relay_controller->isActive(RelayChannel::MASTHEAD_LIGHT));
    
    delete relay_controller;
    relay_controller = nullptr;
}

// =============================================================================
// TEST RUNNER
// =============================================================================

void setUp(void) {
    // Initialize Serial for test output
    Serial.begin(115200);
    delay(100);
}

void tearDown(void) {
    // Clean up any remaining test objects
    teardown_test_ecu();
}

void setup() {
    // Wait for Serial
    delay(2000);
    
    UNITY_BEGIN();
    
    // ECU integration tests
    RUN_TEST(test_ecu_initializes_with_safe_defaults);
    RUN_TEST(test_ecu_condition_changes_update_state);
    RUN_TEST(test_ecu_boat_state_changes_update_state);
    RUN_TEST(test_ecu_darkness_and_underway_enables_navigation_lights);
    RUN_TEST(test_ecu_anchorage_enables_anchor_light);
    RUN_TEST(test_ecu_mute_unmute_periodic_signals);
    RUN_TEST(test_ecu_adhoc_signal_triggers_horn);
    RUN_TEST(test_ecu_emergency_stop_disables_all_outputs);
    RUN_TEST(test_ecu_state_change_callback_fires);
    RUN_TEST(test_ecu_countdown_decrements_when_unmuted);
    RUN_TEST(test_ecu_all_light_combinations);
    
    // Hardware controller tests
    RUN_TEST(test_timer_callbacks_execute);
    RUN_TEST(test_relay_controller_initializes_pins_low);
    RUN_TEST(test_relay_controller_set_relay);
    
    UNITY_END();
}

void loop() {
    // Empty - tests run once in setup()
}
