/**
 * @file test_sound_controller.cpp
 * @brief Unit tests for SoundController
 * 
 * Tests sound signal timing, mute/unmute, and ad-hoc signals.
 * UI-agnostic: Tests work same whether commands come from SignalK or web UI.
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

void test_ad_hoc_sos_signal_sounds_horn(void) {
    sound_controller->triggerAdHocSignal(AdHocSignal::SOS);
    sound_controller->update();

    TEST_ASSERT_TRUE(sound_controller->isHornActive());

    sound_controller->stopAllSound();
}

// =============================================================================
// UNMUTE IMMEDIATE PLAYBACK TESTS
// =============================================================================

void test_unmute_triggers_immediate_signal(void) {
    // Set periodic signal (starts muted)
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_2MIN, 120);
    TEST_ASSERT_TRUE(sound_controller->isPeriodicMuted());
    
    // Advance time so we're not at countdown=0
    mock_timer->advanceTime(10000);
    TEST_ASSERT_EQUAL(110, sound_controller->getPeriodicCountdownSeconds());
    
    // Unmute should trigger signal immediately
    sound_controller->unmutePeriodicSignals();
    sound_controller->update();
    
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    // Countdown should reset to full interval
    TEST_ASSERT_EQUAL(120, sound_controller->getPeriodicCountdownSeconds());
    
    sound_controller->stopAllSound();
}

void test_unmute_resets_countdown_timer(void) {
    // Set periodic signal and advance time
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN, 120);
    mock_timer->advanceTime(50000); // 50 seconds elapsed
    TEST_ASSERT_EQUAL(70, sound_controller->getPeriodicCountdownSeconds());
    
    // Unmute resets countdown
    sound_controller->unmutePeriodicSignals();
    TEST_ASSERT_EQUAL(120, sound_controller->getPeriodicCountdownSeconds());
    
    sound_controller->stopAllSound();
}

// =============================================================================
// AD-HOC SIGNAL QUEUEING TESTS
// =============================================================================

void test_adhoc_queues_when_signal_in_progress(void) {
    // This test verifies that ad-hoc signals can be queued when a signal is playing
    // Full integration testing of the timing should be done on real hardware
    
    // Start first ad-hoc signal
    sound_controller->triggerAdHocSignal(AdHocSignal::TURN_STARBOARD);
    sound_controller->update();
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Trigger second ad-hoc while first is playing - should queue (not fail)
    sound_controller->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    sound_controller->update();
    
    // First signal should still be playing (queue doesn't interrupt)
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Note: Full queueing behavior with delays requires real-time testing on ESP32
    sound_controller->stopAllSound();
}

void test_adhoc_queues_after_periodic_signal(void) {
    // This test verifies that ad-hoc signals can be queued during periodic signals
    // Full integration testing of the timing should be done on real hardware
    
    // Start periodic signal (unmuted)
    sound_controller->setPeriodicSignal(SoundSignalPattern::PROLONGED_2MIN, 5);
    sound_controller->unmutePeriodicSignals();
    sound_controller->update();
    
    // Periodic signal should be playing
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Trigger ad-hoc while periodic is playing - should queue (not fail)
    sound_controller->triggerAdHocSignal(AdHocSignal::TURN_PORT);
    
    // Periodic should continue (queue doesn't interrupt)
    TEST_ASSERT_TRUE(sound_controller->isHornActive());
    
    // Note: Full queueing behavior with delays requires real-time testing on ESP32
    sound_controller->stopAllSound();
}

void test_stop_all_clears_queue(void) {
    // Start signal and queue another
    sound_controller->triggerAdHocSignal(AdHocSignal::PAY_ATTENTION);
    sound_controller->update();
    sound_controller->triggerAdHocSignal(AdHocSignal::DANGER_CONFUSION);
    
    // Emergency stop should clear everything
    sound_controller->stopAllSound();
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
    
    // Advance past when queued signal would have played
    mock_timer->advanceTime(10000);
    sound_controller->update();
    
    // No signal should play (queue was cleared)
    TEST_ASSERT_FALSE(sound_controller->isHornActive());
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
    RUN_TEST(test_ad_hoc_sos_signal_sounds_horn);
    
    // Unmute immediate playback
    RUN_TEST(test_unmute_triggers_immediate_signal);
    RUN_TEST(test_unmute_resets_countdown_timer);
    
    // Ad-hoc signal queueing
    RUN_TEST(test_adhoc_queues_when_signal_in_progress);
    RUN_TEST(test_adhoc_queues_after_periodic_signal);
    RUN_TEST(test_stop_all_clears_queue);
    
    // Safety
    RUN_TEST(test_stop_all_sound_emergency);
    RUN_TEST(test_no_signal_when_pattern_none);
    
    return UNITY_END();
}
