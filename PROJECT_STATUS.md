# Navigation Lights and Signal ECU - Project Status

**Last Updated**: February 8, 2026

## Overview
ESP32-based ECU for controlling navigation lights and sound signals on pleasure boats <15m, implementing COLREGs (International Maritime Organization rules). Built on SensESP platform using SignalK protocol for communication.

## Implementation Status: **Phase 4 Complete, Phase 5 In Progress** 🚧 

### ✅ Phase 1: Project Foundation (100%)
- [platformio.ini](platformio.ini) - ESP32dev and native test environments
- [src/interfaces/IRelayController.h](src/interfaces/IRelayController.h) - 8-channel relay abstraction
- [src/interfaces/ITimer.h](src/interfaces/ITimer.h) - Timing abstraction
- [.gitignore](.gitignore) - Comprehensive exclusions
- [README.md](README.md) - Quick start guide

### ✅ Phase 2: State Machine (100%)
- [src/state_machine.h](src/state_machine.h) / [.cpp](src/state_machine.cpp) - COLREGs logic
- [test/test_state_machine/](test/test_state_machine/) - **20 tests passing**
  - All 18 COLREGs state combinations validated
  - Rules: 20, 21, 23, 25, 27, 30, 35
  - Conditions: Day / Hours of Darkness / Restricted Visibility
  - Boat States: Moored / Underway (making way/no way) / Anchorage / NUC (making way/no way)

### ✅ Phase 3: Hardware Controllers (100%)
- [src/ESP32RelayController.h](src/ESP32RelayController.h) / [.cpp](src/ESP32RelayController.cpp) - GPIO implementation with active-low safety
- [src/LightController.h](src/LightController.h) / [.cpp](src/LightController.cpp) - Light control logic
- [src/SoundController.h](src/SoundController.h) / [.cpp](src/SoundController.cpp) - Sound signal timing with queueing
- [test/test_light_controller/](test/test_light_controller/) - **9 tests passing**
- [test/test_sound_controller/](test/test_sound_controller/) - **16 tests passing** (includes unmute immediate playback + ad-hoc queueing)

### ✅ Phase 4: SignalK Integration (100%) - CODE COMPLETE
- ✅ [src/ESP32Timer.h](src/ESP32Timer.h) / [.cpp](src/ESP32Timer.cpp) - Production timer with platform-independent debug logging
- ✅ [src/NavigationLightsECU.h](src/NavigationLightsECU.h) / [.cpp](src/NavigationLightsECU.cpp) - UI-agnostic facade
- ✅ [src/main.cpp](src/main.cpp) - Complete hardware initialization + SignalK setup
- ✅ [src/signalk_integration.h](src/signalk_integration.h) / [.cpp](src/signalk_integration.cpp) - SensESP v3 API complete
  - **ObservableValue→SKOutput Pattern**: Bidirectional SignalK communication
  - **Periodic Updates**: 60s heartbeat calls `updateAllObservableValues(ecu)` (matches boat.light-signal-ECU)
  - **Reconnection Handling**: Auto-republishes all values 2s after connection
  - 5 input paths (PUT requests) + 13 output paths (status publishing)
- ✅ [test/test_signalk_integration/](test/test_signalk_integration/) - **74 tests passing** (conversions, integration, edge cases)

**Recent Feature Additions**:
- ✅ Ad-hoc signal queueing (2-second delay after signal completion)
- ✅ Unmute immediate playback (signals play immediately when unmuted)
- ✅ Platform-independent debug logging (NATIVE_BUILD flag + DEBUG macros)
- ✅ COLREGs Rule 35 corrections (making way = 1 blast, stopped = 2 blasts)

**Current Build**: `az-delivery-devkit-v4` environment with SensESP v3.2.2
- RAM: **9.4%** (50052 / 532480 bytes)
- Flash: **71.8%** (1411669 / 1966080 bytes) with min_spiffs.csv partition
- Status: **BUILD SUCCESSFUL** ✅

### 🚧 Phase 5: Hardware Integration Testing (In Progress)
- ⏳ Flash firmware to ESP32 Dev Kit C V4
- ⏳ Re-run 14 ESP32 embedded tests after recent code changes
- ⏳ Validate ad-hoc signal queueing timing (2-second delay)
- ⏳ Measure prolonged blast duration on real hardware
- ⏳ Test unmute immediate playback timing
- ⏳ Verify all 18 COLREGs light combinations on actual relays
- ⏳ Connect to SignalK server and validate bidirectional communication
- ⏳ Test periodic updates and reconnection handling

**Hardware Required**: ESP32 Dev Kit C V4 + 8-channel opto-isolated relay module (active-low)

### ⏳ Phase 6: Maritime Field Testing (0%)
- Install on test vessel
- Validate in actual maritime conditions
- Test visibility compliance (light visibility at required distances)
- Test sound signal audibility

### ⏳ Phase 7: Optional Enhancements (0%)
- BLE fallback UI (when WiFi unavailable)
- Web configuration portal
- OTA firmware updates

## Test Coverage

**Total: 119 tests - All Passing ✅** (Native platform only)

### Native Tests (119 tests, ~7.8s)
| Test Suite | Tests | Files | Status |
|------------|-------|-------|--------|
| State Machine | 20 | [test_state_machine.cpp](test/test_state_machine/test_state_machine.cpp) | ✅ PASSED |
| Light Controller | 9 | [test_light_controller.cpp](test/test_light_controller/test_light_controller.cpp) | ✅ PASSED |
| Sound Controller | 16 | [test_sound_controller.cpp](test/test_sound_controller/test_sound_controller.cpp) | ✅ PASSED |
| SignalK Integration | 74 | [test_signalk_integration.cpp](test/test_signalk_integration/test_signalk_integration.cpp) | ✅ PASSED |

### ESP32 Embedded Tests (14 tests, ~17.8s) - ⚠️ Pending Re-validation
| Test Suite | Tests | Files | Status |
|------------|-------|-------|--------|
| SignalK ESP32 | 14 | [test_signalk_esp32.cpp](test/test_signalk_esp32/test_signalk_esp32.cpp) | ⚠️ NEEDS RE-RUN |

**Note**: These tests previously passed but need re-running after recent code changes:
- Ad-hoc signal queueing implementation
- Unmute immediate playback feature
- COLREGs Rule 35 corrections
- Platform-independent debug logging

### Unit Tests: **119 tests** - All Passing ✅
| Suite | Tests | Status |
|-------|-------|--------|
| State Machine | 20 | ✅ All Passing |
| Light Controller | 9 | ✅ All Passing |
| Sound Controller | 16 | ✅ All Passing |
| SignalK Integration | 74 | ✅ All Passing |

**SignalK Test Coverage** (74 tests):
- **Conversion Functions** (42 tests): enum↔string, all states/signals, roundtrip, invalid input handling
- **Integration Tests** (12 tests): MockECU interactions, callbacks, state transitions
- **Edge Cases** (20 tests): Empty/null strings, case sensitivity, whitespace, typos, rapid changes, callback safety

**Sound Controller Test Coverage** (16 tests):
- **Initial State** (2 tests): Horn inactive, countdown at zero
- **Periodic Signals** (7 tests): Mute/unmute, countdown, immediate playback on unmute
- **Ad-hoc Signals** (4 tests): Immediate playback, queueing behavior
- **Emergency** (2 tests): Stop all sound, queue clearing
- **Safety** (1 test): No signal when pattern is NONE

**Note**: Ad-hoc queueing tests verify behavioral correctness. Full timing validation (2-second delay) requires ESP32 hardware.

### Hardware Validation: **Phase 5 - In Progress** 🚧
- ESP32 Dev Kit C V4 + opto-isolated relay module required
- COLREGs logic fully validated in unit tests (119 passing)
- Hardware timing tests pending (queueing delay, prolonged blast duration)

## Architecture Highlights

### UI-Agnostic Design ⭐
`NavigationLightsECU` facade enables dual UI support:
- **Main UI**: SignalK over WiFi (pending API integration)
- **Fallback UI**: BLE (future enhancement)

Both UIs control the same underlying controllers.

### SOLID Principles Applied
- **Single Responsibility**: Separate state machine, light control, sound control
- **Open/Closed**: Easy to add new COLREGs rules without modifying core
- **Dependency Inversion**: Hardware abstracted behind interfaces (`IRelayController`, `ITimer`)
- **Interface Segregation**: Minimal, focused interfaces

### Safety Features
1. **Active-low relay control**: Relays OFF on boot/crash/reset
2. **Muted by default**: Periodic sound signals always start muted
3. **Emergency stop**: Single function disables all lights and horn
4. **Minimal relay switching**: Only changes relays when configuration changes

## GPIO Pin Mapping

### Relay Output Pins
| GPIO | Function | Relay CH | COLREGs Rule | Active-LOW* | Active-HIGH** |
|------|----------|----------|--------------|-------------|---------------|
| **25** | Masthead Light | 1 | Rule 23 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **26** | Port Sidelight | 2 | Rule 21 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **27** | Starboard Sidelight | 3 | Rule 21 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **14** | Sternlight | 4 | Rule 21 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **12** | All-round White | 5 | Rule 30 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **13** | All-round Red Upper | 6 | Rule 27 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **15** | All-round Red Lower | 7 | Rule 27 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |
| **4** | Horn | 8 | Rule 35 | LOW=ON, HIGH=OFF | HIGH=ON, LOW=OFF |

*Production mode (default) - opto-isolated relay module  
**Testing mode - set `ACTIVE_HIGH_RELAYS` in platformio.ini*

### System Pins (Reserved)
- **GPIO 2**: Built-in LED (SensESP status blink)
- **GPIO 0**: BOOT button (do not use)
- **GPIO 1/3**: UART TX/RX (Serial console)
- **GPIO 6-11**: Internal SPI flash (never use)

### Network Configuration
- **WiFi**: Configurable via web portal (no hardcoded SSID)
- **DHCP Range**: 10.100.100.100 - 10.100.100.250
- **SignalK Server**: Configurable via web portal (no hardcoded IP)
- **Hostname**: nav-lights-ecu.local

## Next Steps

### Phase 5: Hardware Integration Testing (Current Focus)
1. **Flash and Test**: Load firmware on ESP32 Dev Kit C V4
2. **Re-run ESP32 Tests**: Validate 14 embedded tests with updated code
3. **Timing Validation**:
   - Ad-hoc queueing: Verify 2-second delay after signal completion
   - Unmute immediate playback: Measure response time (<100ms expected)
   - Prolonged blast duration: Measure actual duration (user reported shorter than 4-6s)
4. **Relay Testing**: Connect 8-channel opto-isolated relay module, verify active-low control
5. **COLREGs Validation**: Test all 18 light combinations on actual hardware
6. **SignalK Server Integration**: Connect to real SignalK server, test bidirectional communication

### Phase 6: Maritime Field Testing
1. Install on test vessel
2. Validate in actual maritime conditions (darkness, restricted visibility)
3. Test light visibility at required distances
4. Test sound signal audibility
5. Verify state transitions during actual vessel operation

### Phase 7: Optional Enhancements (Future)
1. BLE fallback UI for when WiFi unavailable
2. Web configuration portal (WiFi credentials, SignalK server)
3. Watchdog timer for firmware hang detection
4. OTA firmware updates

## Development Workflow

### Build Commands
```bash
# Build ESP32 firmware (without SignalK)
pio run -e esp32dev-signalk

# Flash to ESP32
pio run -e esp32dev-signalk -t upload

# Monitor serial output
pio device monitor

# Run unit tests (requires MinGW in PATH)
pio test -e native
```

### Project Structure
```
├── .github/
│   └── copilot-instructions.md     # AI agent guidance
├── src/
│   ├── interfaces/                 # Hardware abstractions
│   ├── state_machine.*             # COLREGs logic
│   ├── *Controller.*               # Light/sound/relay controllers
│   ├── NavigationLightsECU.*       # UI-agnostic facade
│   ├── signalk_integration.*       # SignalK layer (deferred)
│   └── main.cpp                    # Application entry
├── test/
│   ├── test_state_machine/         # 20 COLREGs tests
│   ├── test_light_controller/      # 9 light tests
│   └── test_sound_controller/      # 11 sound tests
├── platformio.ini                  # Build configuration
├── README.md                       # Quick start
├── PHASE4_STATUS.md                # Detailed Phase 4 status
└── Project Proposal Navigation Lights and Signaling ECU.md
```

## Known Issues & Notes

1. **Hardware Timing Validation Pending**: MockTimer in unit tests cannot accurately simulate asynchronous timer callbacks. Full timing validation of ad-hoc queueing (2-second delay) requires ESP32 hardware testing.
2. **Prolonged Blast Duration**: User reported prolonged blasts appear shorter than expected (4-6 seconds). Needs measurement on real hardware.
3. **ESP32 Tests Pending Re-run**: 14 embedded tests previously passed but need re-validation after recent code changes.

## Dependencies

- **Platform**: ESP32 (Arduino framework)
- **Library**: SensESP 3.2.2 (for future SignalK integration)
- **Test Framework**: Unity 2.6.0
- **Build Tool**: PlatformIO

## Documentation

- [Project Proposal](Project Proposal Navigation Lights and Signaling ECU.md) - Complete requirements, COLREGs tables, signal definitions
- [AI Coding Instructions](.github/copilot-instructions.md) - Architecture, conventions, workflows for AI agents
- [Phase 4 Status](PHASE4_STATUS.md) - Detailed SignalK integration status
- [README](README.md) - Quick start guide

## Summary

**Phase 4 Code Complete - Ready for Hardware Integration:**
- ✅ COLREGs state machine fully tested (20 unit tests)
- ✅ Hardware controllers implemented with safety features (25 unit tests for light + sound)
- ✅ SignalK integration complete (74 unit tests)
- ✅ UI-agnostic architecture supporting multiple control interfaces
- ✅ Build successful (71.8% flash, 9.4% RAM)
- ✅ All 119 native unit tests passing
- ✅ Recent features: Ad-hoc queueing, unmute immediate playback, platform-independent logging

**Current Status: Phase 5 - Hardware Integration Testing**
- 🚧 ESP32 hardware testing required
- 🚧 Timing validation (queueing delay, prolonged blast duration)
- 🚧 SignalK server integration testing
- 🚧 Full COLREGs validation on actual relays

**The firmware is code-complete and ready for hardware validation.**

---

*Last updated: February 8, 2026 - Phase 4 complete, Phase 5 in progress*
