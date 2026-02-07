/**
 * @file SoundController.cpp
 * @brief Implementation of sound signal controller
 */

#include "SoundController.h"
#include <Arduino.h>

SoundController::SoundController(IRelayController& relay_controller, ITimer& timer)
    : relay_controller_(relay_controller),
      timer_(timer),
      current_periodic_pattern_(SoundSignalPattern::NONE),
      periodic_interval_seconds_(0),
      periodic_muted_(true), // Safety: always start muted
      periodic_timer_id_(0),
      last_periodic_start_ms_(0),
      signal_in_progress_(false),
      sequence_index_(0),
      sequence_timer_id_(0),
      sequence_horn_active_(false),
      sequence_state_start_ms_(0) {
}

SoundController::~SoundController() {
    // Cancel all timers to prevent callbacks after destruction
    stopAllSound();
}

void SoundController::update() {
    timer_.update();
    
    // Update sequence playback if active
    if (signal_in_progress_ && !current_sequence_.empty()) {
        updateSequencePlayback();
    }
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
        Serial.println("[HORN] Signal already in progress, ignoring new trigger");
        return; // Don't interrupt ongoing signal
    }
    
    // Set flag immediately to prevent race condition
    signal_in_progress_ = true;
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
    Serial.println("[HORN] Horn STARTED");
    relay_controller_.activate(RelayChannel::HORN);
}

void SoundController::stopHorn() {
    Serial.println("[HORN] Horn STOPPED");
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
    std::vector<BlastStep> sequence;
    
    switch (current_periodic_pattern_) {
        case SoundSignalPattern::NONE:
            return; // No signal to play
            
        case SoundSignalPattern::PROLONGED_2MIN:
            // ▬▬ - 1 prolonged blast (Rule 35: underway making no way, restricted visibility)
            sequence = {{true}};
            Serial.println("[HORN] Periodic: 1 prolonged blast");
            break;
            
        case SoundSignalPattern::PROLONGED_PROLONGED_2MIN:
            // ▬▬ ▬▬ - 2 prolonged blasts (Rule 35: underway making way, restricted visibility)
            sequence = {{true}, {true}};
            Serial.println("[HORN] Periodic: 2 prolonged blasts");
            break;
            
        case SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN:
            // ▬▬ ● ● - 1 prolonged + 2 short blasts (Rule 35: NUC, restricted visibility)
            sequence = {{true}, {false}, {false}};
            Serial.println("[HORN] Periodic: 1 prolonged + 2 short blasts");
            break;
            
        case SoundSignalPattern::SHORT_PROLONGED_SHORT:
            // ● ▬▬ ● - Short, prolonged, short (Rule 35: anchorage warning)
            sequence = {{false}, {true}, {false}};
            Serial.println("[HORN] Periodic: Short-Prolonged-Short");
            break;
    }
    
    if (!sequence.empty()) {
        playSequence(sequence);
    }
}

void SoundController::playAdHocPattern(AdHocSignal signal) {
    std::vector<BlastStep> sequence;
    
    switch (signal) {
        case AdHocSignal::TURN_STARBOARD:
            // ● - 1 short blast
            sequence = {{false}};
            break;
        case AdHocSignal::TURN_PORT:
            // ●● - 2 short blasts
            sequence = {{false}, {false}};
            break;
        case AdHocSignal::ASTERN_PROPULSION:
            // ●●● - 3 short blasts
            sequence = {{false}, {false}, {false}};
            break;
        case AdHocSignal::DANGER_CONFUSION:
            // ●●●●● - 5 short blasts
            sequence = {{false}, {false}, {false}, {false}, {false}};
            break;
        case AdHocSignal::PAY_ATTENTION:
            // ▬ - 1 prolonged blast (vessel approaching bend/obstruction)
            sequence = {{true}};
            break;
        case AdHocSignal::OVERTAKE_STARBOARD:
            // ▬ ▬ ● - 2 prolonged + 1 short (Rule 34(c)(i))
            sequence = {{true}, {true}, {false}};
            break;
        case AdHocSignal::OVERTAKE_PORT:
            // ▬ ▬ ●● - 2 prolonged + 2 short (Rule 34(c)(ii))
            sequence = {{true}, {true}, {false}, {false}};
            break;
        case AdHocSignal::AGREEMENT_OVERTAKEN:
            // ▬ ● ▬ ● - prolonged, short, prolonged, short (Rule 34(c))
            sequence = {{true}, {false}, {true}, {false}};
            break;
    }
    
    Serial.printf("[HORN] Playing ad-hoc signal with %d blasts\n", sequence.size());
    playSequence(sequence);
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

void SoundController::playSequence(const std::vector<BlastStep>& sequence) {
    if (sequence.empty()) {
        return;
    }
    
    current_sequence_ = sequence;
    sequence_index_ = 0;
    signal_in_progress_ = true;
    sequence_state_start_ms_ = timer_.millis();
    
    // Start first blast immediately
    bool is_prolonged = current_sequence_[0].is_prolonged;
    Serial.printf("[HORN] Playing blast 1/%d (%s)\n", 
                  sequence.size(),
                  is_prolonged ? "prolonged" : "short");
    startHorn();
    sequence_horn_active_ = true;
}

void SoundController::updateSequencePlayback() {
    if (current_sequence_.empty()) {
        // Safety check - shouldn't happen
        if (sequence_horn_active_) {
            stopHorn();
        }
        signal_in_progress_ = false;
        sequence_horn_active_ = false;
        return;
    }
    
    uint32_t elapsed = timer_.millis() - sequence_state_start_ms_;
    
    if (sequence_horn_active_) {
        // Currently playing a blast - check if it should end
        bool is_prolonged = current_sequence_[sequence_index_].is_prolonged;
        uint32_t blast_duration = is_prolonged ? PROLONGED_BLAST_MS : SHORT_BLAST_MS;
        
        if (elapsed >= blast_duration) {
            // Blast finished - stop horn
            stopHorn();
            sequence_horn_active_ = false;
            sequence_state_start_ms_ = timer_.millis();
            
            // Move to next blast
            sequence_index_++;
            
            // Check if sequence is complete
            if (sequence_index_ >= current_sequence_.size()) {
                signal_in_progress_ = false;
                current_sequence_.clear();
                sequence_index_ = 0;
                Serial.println("[HORN] Sequence complete");
            }
        }
    } else {
        // Currently in pause between blasts
        if (elapsed >= PAUSE_MS) {
            // Check if we've played all blasts
            if (sequence_index_ >= current_sequence_.size()) {
                // All blasts complete
                signal_in_progress_ = false;
                current_sequence_.clear();
                sequence_index_ = 0;
                Serial.println("[HORN] Sequence complete");
                return;
            }
            
            // Start next blast (sequence_index was incremented after previous blast)
            bool is_prolonged = current_sequence_[sequence_index_].is_prolonged;
            Serial.printf("[HORN] Playing blast %d/%d (%s)\n", 
                          sequence_index_ + 1, 
                          current_sequence_.size(),
                          is_prolonged ? "prolonged" : "short");
            
            startHorn();
            sequence_horn_active_ = true;
            sequence_state_start_ms_ = timer_.millis();
        }
    }
}
