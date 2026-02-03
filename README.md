# Navigation Lights and Signal ECU

ESP32-based controller for COLREGs-compliant navigation lights and sound signals on pleasure boats <15m. Integrated with SignalK for remote monitoring and control.

## Status: ✅ Production Ready

**Test Coverage**: 128 tests passing (114 native + 14 ESP32 embedded)  
**Build**: Successful (71.8% flash, 9.4% RAM)  
**Hardware**: Validated on ESP32 Dev Kit C V4  
**SignalK**: Full bidirectional integration with periodic updates

## Quick Start

### Build & Flash
```bash
# Build project
pio run

# Flash to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Run Tests
```bash
# Native tests (fast, ~8.5s)
pio test -e native

# ESP32 embedded tests (requires hardware, ~17.8s)
pio test -e esp32test

# Run all tests
pio test -e native && pio test -e esp32test
```

## Project Structure
```
├── src/
│   ├── main.cpp                    # Main entry + SignalK setup
│   ├── interfaces/                 # Hardware abstraction layer
│   │   ├── IRelayController.h      # Relay interface
│   │   └── ITimer.h                # Timer interface
│   ├── state_machine.cpp/.h        # COLREGs state management (✅ 20 tests)
│   ├── LightController.cpp/.h      # Light relay control (✅ 9 tests)
│   ├── SoundController.cpp/.h      # Horn/signal control (✅ 11 tests)
│   ├── NavigationLightsECU.cpp/.h  # ECU facade (✅ tested)
│   ├── signalk_integration.cpp/.h  # SignalK layer (✅ 74 tests)
│   ├── ESP32RelayController.cpp/.h # GPIO implementation
│   └── ESP32Timer.cpp/.h           # FreeRTOS timer
├── test/
│   ├── test_state_machine/         # 20 COLREGs tests
│   ├── test_light_controller/      # 9 relay tests
│   ├── test_sound_controller/      # 11 timing tests
│   ├── test_signalk_integration/   # 74 SignalK tests
│   └── test_signalk_esp32/         # 14 ESP32 hardware tests
├── platformio.ini                  # Build configuration
└── Project Proposal...md           # COLREGs requirements
```

## Features

### COLREGs Compliance
- ✅ Rules 20, 21, 23, 25, 27, 30, 35 implemented
- ✅ 18 condition/state combinations validated
- ✅ Navigation lights (masthead, sidelights, sternlight)
- ✅ NUC lights (2× red all-round vertical)
- ✅ Anchorage light (all-round white)
- ✅ Sound signals (short/prolonged blasts, periodic patterns)

### SignalK Integration
- ✅ Bidirectional communication (PUT requests + delta publishing)
- ✅ 5 input paths (condition, state, mute, ad-hoc signals, emergency stop)
- ✅ 13 output paths (status + lights + horn + heartbeat)
- ✅ Periodic updates every 60s (heartbeat pattern)
- ✅ Auto-reconnection with state republishing
- ✅ Base path: `electrical.switches.navigationLights.*`

### Safety Features
- ✅ Active-low relay control (OFF on boot/crash)
- ✅ Periodic signals always start muted
- ✅ Emergency stop function
- ✅ Safe defaults (Day, Moored, all lights OFF)

## Documentation
- [PROJECT_STATUS.md](PROJECT_STATUS.md) - Overall project status
- [PHASE4_STATUS.md](PHASE4_STATUS.md) - SignalK integration details
- [TEST_RESULTS.md](TEST_RESULTS.md) - Complete test documentation
- [.github/copilot-instructions.md](.github/copilot-instructions.md) - Development guidelines
- [Project Proposal Navigation Lights and Signaling ECU.md](Project%20Proposal%20Navigation%20Lights%20and%20Signaling%20ECU.md) - COLREGs requirements

## Hardware
- **MCU**: AZ-Delivery ESP32 Dev Kit C V4 (ESP32-D0WD-V3, 240MHz, 520KB RAM, 4MB Flash)
- **Relays**: 8-channel opto-isolated module (active-low control)
- **Outputs**: 
  - Navigation lights (masthead, port sidelight, starboard sidelight, sternlight)
  - Special lights (all-round white, 2× all-round red for NUC)
  - Sound signal (horn)
- **Communication**: WiFi (SignalK via SensESP), optional BLE (future)
