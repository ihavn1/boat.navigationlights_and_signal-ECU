/**
 * @file MockRelayController.h
 * @brief Mock implementation of IRelayController for unit testing
 */

#ifndef MOCK_RELAY_CONTROLLER_H
#define MOCK_RELAY_CONTROLLER_H

#include "../src/interfaces/IRelayController.h"

/**
 * @class MockRelayController
 * @brief Test double for relay controller (no hardware required)
 */
class MockRelayController : public IRelayController {
public:
    MockRelayController() {
        for (int i = 0; i < 8; i++) {
            relay_states_[i] = false;
        }
        begin_called_ = false;
        deactivate_all_called_ = false;
    }

    bool begin() override {
        begin_called_ = true;
        return true;
    }

    void activate(RelayChannel channel) override {
        relay_states_[static_cast<uint8_t>(channel)] = true;
    }

    void deactivate(RelayChannel channel) override {
        relay_states_[static_cast<uint8_t>(channel)] = false;
    }

    bool isActive(RelayChannel channel) const override {
        return relay_states_[static_cast<uint8_t>(channel)];
    }

    void deactivateAll() override {
        for (int i = 0; i < 8; i++) {
            relay_states_[i] = false;
        }
        deactivate_all_called_ = true;
    }

    // Test helper methods
    bool wasBeginCalled() const { return begin_called_; }
    bool wasDeactivateAllCalled() const { return deactivate_all_called_; }

private:
    bool relay_states_[8];
    bool begin_called_;
    bool deactivate_all_called_;
};

#endif // MOCK_RELAY_CONTROLLER_H
