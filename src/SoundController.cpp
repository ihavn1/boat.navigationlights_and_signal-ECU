/**
 * @file SoundController.cpp
 * @brief Implementation of sound signal controller
 */

#include "SoundController.h"

SoundController::SoundController(IRelayController& relay_controller, ITimer& timer)
    : relay_controller_(relay_controller),
      timer_(timer),
      current_periodic_pattern_(SoundSignalPattern::NONE),
      periodic_interval_seconds_(0),
      periodic_muted_(true), // Safety: always start muted
      periodic_timer_id_(0),
      last_periodic_start_ms_(0),
      signal_in_progress_(false) {
}

SoundController::~SoundController() {
    // Cancel all timers to prevent callbacks after destruction
    stopAllSound();
}

void SoundController::update() {
    timer_.update();
}

void SoundController::setPeriodicSignal(SoundSignalPattern pattern, uint16_t interval_seconds) {
    // Cancel existing periodic timer if any
    if (periodic_timer_id_ != 0) {
        timer_.cancel(periodic_timer_id_);
        periodic_timer_id_ = 0;
    }
    
    current_periodic_pattern_ = pattern;
    periodic_interval_seconds_ = interval_seconds;
    periodic_muted_ = true; // Safety: always start muted on pattern change
    last_periodic_start_ms_ = timer_.millis(); // Reset countdown
    
    if (pattern != SoundSignalPattern::NONE && interval_seconds > 0) {
        scheduleNextPeriodicSignal();
    }
}

void SoundController::mutePeriodicSignals() {
    periodic_muted_ = true;
}

void SoundController::unmutePeriodicSignals() {
    periodic_muted_ = false;
}

uint16_t SoundController::getPeriodicCountdownSeconds() const {
    if (periodic_interval_seconds_ == 0 || current_periodic_pattern_ == SoundSignalPattern::NONE) {
        return 0;
    }
    
    uint32_t elapsed_ms = timer_.millis() - last_periodic_start_ms_;
    uint32_t interval_ms = static_cast<uint32_t>(periodic_interval_seconds_) * 1000;
    
    if (elapsed_ms >= interval_ms) {
        return 0; // Countdown expired
    }
    
    uint32_t remaining_ms = interval_ms - elapsed_ms;
    return static_cast<uint16_t>(remaining_ms / 1000);
}

void SoundController::triggerAdHocSignal(AdHocSignal signal) {
    if (signal_in_progress_) {
        return; // Don't interrupt ongoing signal
    }
    
    playAdHocPattern(signal);
}

void SoundController::stopAllSound() {
    stopHorn();
    signal_in_progress_ = false;
    
    if (periodic_timer_id_ != 0) {
        timer_.cancel(periodic_timer_id_);
        periodic_timer_id_ = 0;
    }
}

bool SoundController::isHornActive() const {
    return relay_controller_.isActive(RelayChannel::HORN);
}

// =============================================================================
// PRIVATE METHODS
// =============================================================================

void SoundController::startHorn() {
    relay_controller_.activate(RelayChannel::HORN);
}

void SoundController::stopHorn() {
    relay_controller_.deactivate(RelayChannel::HORN);
}

void SoundController::scheduleNextPeriodicSignal() {
    if (periodic_timer_id_ != 0) {
        timer_.cancel(periodic_timer_id_);
    }
    
    last_periodic_start_ms_ = timer_.millis();
    
    periodic_timer_id_ = timer_.scheduleRepeating(
        periodic_interval_seconds_ * 1000,
        [this]() { this->onPeriodicTimerExpired(); }
    );
}

void SoundController::onPeriodicTimerExpired() {
    if (!periodic_muted_ && current_periodic_pattern_ != SoundSignalPattern::NONE) {
        playPeriodicSignal();
    }
    
    last_periodic_start_ms_ = timer_.millis();
}

void SoundController::playPeriodicSignal() {
    // Simplified implementation: just sound short blast for now
    // Full implementation would play the complete pattern
    playShortBlast();
}

void SoundController::playAdHocPattern(AdHocSignal signal) {
    // Simplified implementation: just sound short blast for now
    // Full implementation would play complete patterns (multiple blasts with pauses)
    playShortBlast();
}

void SoundController::playShortBlast() {
    signal_in_progress_ = true;
    startHorn();
    
    timer_.scheduleOnce(SHORT_BLAST_MS, [this]() {
        this->stopHorn();
        this->signal_in_progress_ = false;
    });
}

void SoundController::playProlongedBlast() {
    signal_in_progress_ = true;
    startHorn();
    
    timer_.scheduleOnce(PROLONGED_BLAST_MS, [this]() {
        this->stopHorn();
        this->signal_in_progress_ = false;
    });
}
