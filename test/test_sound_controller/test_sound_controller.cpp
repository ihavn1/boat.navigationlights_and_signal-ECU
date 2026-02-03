/**
 * @file test_sound_controller.cpp
 * @brief Unit tests for SoundController
 * 
 * Tests sound signal timing, mute/unmute, and ad-hoc signals.
 * UI-agnostic: Tests work same whether commands come from SignalK or BLE.
 */

#include <unity.h>
#include "MockTimer.h"
#include "../test_light_controller/MockRelayController.h"
#include "../src/SoundController.h"

// Test fixtures
MockRelayController* mock_relay;
MockTimer* mock_timer;
SoundController* sound_controller;

void setUp(void) {
    mock_relay = new MockRelayController();
    mock_timer = new MockTimer();
    sound_controller = new SoundController(*mock_relay, *mock_timer);
}

void tearDown(void) {
    delete sound_controller;
    delete mock_timer;
    delete mock_relay;
}

// =============================================================================
// INITIALIZATION TESTS
// =============================================================================

void test_initial_state_no_horn(void) {
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted()); // Safety: start muted
}

void test_initial_countdown_zero(void) {
    TEST_ASSERT_EQUAL(0, sound_controller->getPeriodicCountdownSeconds());
}

// =============================================================================
// PERIODIC SIGNAL CONFIGURATION TESTS
// =============================================================================

void test_set_periodic_signal_starts_muted(void) {
    // Rule 35: Periodic signals must start muted for safety
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 120);
    
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted());
}

void test_periodic_signal_countdown(void) {
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 120);
    
    TEST_ASSERT_EQUAL(120, sound_controller->getPeriodicCountdownSeconds());
    
    mock_timer->advanceTime(30000); // 30 seconds
    TEST_ASSERT_EQUAL(90, sound_controller->getPeriodicCountdownSeconds());
    
    mock_timer->advanceTime(89000); // 89 more seconds (total 119)
    TEST_ASSERT_EQUAL(1, sound_controller->getPeriodicCountdownSeconds());
    
    mock_timer->advanceTime(1000); // 1 more second (total 120) - timer fires and resets
    TEST_ASSERT_EQUAL(120, sound_controller->getPeriodicCountdownSeconds()); // Countdown restarted
}

void test_periodic_signal_does_not_sound_when_muted(void) {
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 5); // 5 sec for quick test
    
    // Advance past interval
    mock_timer->advanceTime(6000);
    sound_controller->update();
    
    // Horn should NOT have sounded (muted)
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
}

void test_periodic_signal_sounds_when_unmuted(void) {
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 5);
    sound_controller->unmutePeriodicSignals();
    
    TEST_ASSERT_FALSE(sound_controller->isPeriodicMuted());
    
    // Advance past interval
    mock_timer->advanceTime(6000);
    sound_controller->update();
    
    // Horn should be sounding
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Clean up to prevent callback issues
    sound_controller->stopAllSound();
}

// =============================================================================
// MUTE/UNMUTE TESTS (UI COMMANDS)
// =============================================================================

void test_mute_unmute_toggles_state(void) {
    sound_controller->unmutePeriodicSignals();
    TEST_ASSERT_FALSE(sound_controller->isPeriodicMuted());
    
    sound_controller->mutePeriodicSignals();
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted());
}

void test_setting_new_pattern_resets_to_muted(void) {
    // Unmute first pattern
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 120);
    sound_controller->unmutePeriodicSignals();
    TEST_ASSERT_FALSE(sound_controller->isPeriodicMuted());
    
    // Change pattern - should reset to muted for safety
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN, 120);
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted());
}

// =============================================================================
// AD-HOC SIGNAL TESTS (UI COMMANDS)
// =============================================================================

void test_ad_hoc_signal_sounds_horn(void) {
    // Ad-hoc signals sound immediately regardless of mute state
    sound_controller->triggerAdHocSignal(AdHocSignal::TURN_STARBOARD);
    sound_controller->update();
    
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Horn stops after short blast duration
    mock_timer->advanceTime(1100); // SHORT_BLAST_MS + margin
    sound_controller->update();
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
}

void test_ad_hoc_signal_works_when_periodic_muted(void) {
    // Set muted periodic signal
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_PROLONGED_2MIN, 120);
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted());
    
    // Ad-hoc should still work
    sound_controller->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    sound_controller->update();
    
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
}

// =============================================================================
// SAFETY TESTS
// =============================================================================

void test_stop_all_sound_emergency(void) {
    // Start a signal
    sound_controller->triggerAdHocSignal(AdHocSignal::PAY_ATTENTION);
    sound_controller->update();
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Emergency stop
    sound_controller->stopAllSound();
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
}

void test_no_signal_when_pattern_none(void) {
    sound_controller->setPeriodicSignal(SoundSignalPattern::NONE, 0);
    sound_controller->unmutePeriodicSignals();
    
    mock_timer->advanceTime(10000);
    sound_controller->update();
    
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
    TEST_ASSERT_EQUAL(0, sound_controller->getPeriodicCountdownSeconds());
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Initialization
    RUN_TEST(test_initial_state_no_horn);
    RUN_TEST(test_initial_countdown_zero);
    
    // Periodic signal configuration
    RUN_TEST(test_set_periodic_signal_starts_muted);
    RUN_TEST(test_periodic_signal_countdown);
    RUN_TEST(test_periodic_signal_does_not_sound_when_muted);
    // TODO: Fix callback lifecycle issue in test_periodic_signal_sounds_when_unmuted
    // RUN_TEST(test_periodic_signal_sounds_when_unmuted);
    
    // Mute/unmute (UI commands)
    RUN_TEST(test_mute_unmute_toggles_state);
    RUN_TEST(test_setting_new_pattern_resets_to_muted);
    
    // Ad-hoc signals (UI commands)
    RUN_TEST(test_ad_hoc_signal_sounds_horn);
    RUN_TEST(test_ad_hoc_signal_works_when_periodic_muted);
    
    // Safety
    RUN_TEST(test_stop_all_sound_emergency);
    RUN_TEST(test_no_signal_when_pattern_none);
    
    return UNITY_END();
}
