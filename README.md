# Navigation Lights and Signal ECU

ESP32-based controller for COLREGs-compliant navigation lights and sound signals on pleasure boats <15m. Integrated with SignalK for remote monitoring and control.

## Status: ✅ Code Complete + Web API Integrated 🌐

**Test Coverage**: 148 tests passing (119 unit + 29 Web API integration)  
**Build**: Successful (72.5% flash, 9.4% RAM)  
**Code Status**: Complete and validated via comprehensive automated testing  
**Hardware Status**: Running on ESP32 with Web API operational  
**SignalK**: Full bidirectional integration with periodic updates  
**Web UI**: Phase 1 complete - REST API backend with 100% test coverage

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
# Native unit tests (fast, ~7.8s)
pio test -e native

# ESP32 embedded tests (requires hardware, ~17.8s) - pending re-run
pio test -e esp32test

# Web API integration tests (requires running ESP32, ~6.5s)
.\test_web_api.ps1

# Run all tests
pio test -e native && .\test_web_api.ps1
```

## Project Structure+ Web API setup
│   ├── interfaces/                 # Hardware abstraction layer
│   │   ├── IRelayController.h      # Relay interface
│   │   └── ITimer.h                # Timer interface
│   ├── state_machine.cpp/.h        # COLREGs state management (✅ 20 tests)
│   ├── LightController.cpp/.h      # Light relay control (✅ 9 tests)
│   ├── SoundController.cpp/.h      # Horn/signal control (✅ 16 tests)
│   ├── NavigationLightsECU.cpp/.h  # ECU facade (✅ tested)
│   ├── signalk_integration.cpp/.h  # SignalK layer (✅ 74 tests)
│   ├── web_api.cpp/.h              # REST API backend (✅ 29 tests)
│   ├── ESP32RelayController.cpp/.h # GPIO implementation
│   └── ESP32Timer.cpp/.h           # FreeRTOS timer
├── test/
│   ├── test_state_machine/         # 20 COLREGs tests
│   ├── test_light_controller/      # 9 relay tests
│   ├── test_sound_controller/      # 16 timing tests
│   ├── test_signalk_integration/   # 74 SignalK tests
│   └── test_signalk_esp32/         # 14 ESP32 hardware tests (⚠️ pending re-run)
├── test_web_api.ps1                # 29 Web API integration tests (✅ passing
│   ├── test_sound_controller/      # 16 timing tests
│   ├── test_signalk_integration/   # 74 SignalK tests
│   └── test_signalk_esp32/         # 14 ESP32 hardware tests (⚠️ pending re-run)
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
- ✅ Ad-hoc signal queueing (auto-plays after 2s delay if signal in progress)

### SignalK Integration
- ✅ Bidirectional communication (PUT requests + delta p

### Web API (Fallback Control)
- ✅ 7 REST API endpoints (2 GET, 5 POST)
- ✅ Browser-accessible control interface at `http://nav-lights-ecu.local/api/*`
- ✅ JSON request/response with proper error handling (HTTP 200/400)
- ✅ Automated test suite: [test_web_api.ps1](test_web_api.ps1) (29/29 passing)
- ✅ Health monitoring (uptime, heap, WiFi RSSI, SignalK connection status)
- ✅ Full ECU control when SignalK unavailable
- 📋 Phase 2: HTML/CSS/JS frontend (planned)ublishing)
- ✅ 5 input paths (condition, state, mute, ad-hoc signals, emergency stop)
- ✅ 13 output paths (status + lights + horn + heartbeat)
- ✅ Periodic updates every 60s (heartbeat pattern)
- ✅ Auto-reconnection with state republishing
- ✅ Base path: `electrical.switches.navigationLights.*`

### Safety Features
- ✅ Active-low relay control (OFF on boot/crash)
- ✅ Periodic signals always start muted (unmute triggers immediate playback)
- ✅ Ad-hoc signal queueing (prevents overlapping horn signals, auto-plays after 2s delay)
- ✅ Emergency stop function (clears all queues and timers)
- ✅ Safe defaults (Day, Moored, all lights OFF)

## Documentation
- [HARDWARE.md](HARDWARE.md) - **Complete GPIO pin mapping, wiring, and hardware guide**
- [PROJECT_STATUS.md](PROJECT_STATUS.md) - Overall project status
- [PHASE4_STATUS.md](PHASE4_STATUS.md) - SignalK integration details
- [TEST_RESULTS.md](TEST_RESULTS.md) - Complete test documentation
- [.github/copilot-instructions.md](.github/copilot-instructions.md) - Development guidelines
- [Project Proposal Navigation Lights and Signaling ECU.md](Project%20Proposal%20Navigation%20Lights%20and%20Signaling%20ECU.md) - COLREGs requirements

## Hardware

### MCU Specifications
- **Board**: AZ-Delivery ESP32 Dev Kit C V4
- **Chip**: ESP32-D0WD-V3 (Dual-core Xtensa LX6)
- **Clock**: 240 MHz
- **RAM**: 520 KB SRAM
- **Flash**: 4 MB
- **WiFi**: 802.11 b/g/n (built-in, 2.4 GHz)
- **Bluetooth**: BLE 4.2 (not used - WiFi only)

### GPIO Pin Mapping

#### Relay Outputs (8 channels)
| GPIO | Function | Relay | COLREGs Purpose | Polarity |
|------|----------|-------|-----------------|----------|
| **GPIO 25** | Masthead Light | CH1 | Rule 23 - Power-driven vessel | Active-LOW* |
| **GPIO 26** | Port Sidelight | CH2 | Rule 21 - Red (port side) | Active-LOW* |
| **GPIO 27** | Starboard Sidelight | CH3 | Rule 21 - Green (starboard) | Active-LOW* |
| **GPIO 14** | Sternlight | CH4 | Rule 21 - White (aft) | Active-LOW* |
| **GPIO 12** | All-round White | CH5 | Rule 30 - Anchorage | Active-LOW* |
| **GPIO 13** | All-round Red Upper | CH6 | Rule 27 - NUC (upper) | Active-LOW* |
| **GPIO 15** | All-round Red Lower | CH7 | Rule 27 - NUC (lower) | Active-LOW* |
| **GPIO 4** | Horn | CH8 | Rule 35 - Sound signals | Active-LOW* |

**Active-LOW** = Production mode (HIGH=OFF, LOW=ON) for opto-isolated relay safety  
*Set `ACTIVE_HIGH_RELAYS` in [platformio.ini](platformio.ini) for testing with active-HIGH hardware*

#### System Pins (Automatic)
| GPIO | Function | Used By | Notes |
|------|----------|---------|-------|
| **GPIO 2** | Built-in LED | SensESP | Blinks when sending SignalK data |
| **WiFi** | 802.11 b/g/n | SensESP | SignalK communication (10.100.100.x) |

#### Reserved/Unavailable Pins
*Do NOT use these GPIOs - they cause boot issues or are internally connected:*
- **GPIO 0**: BOOT button (must be HIGH at boot)
- **GPIO 1**: UART0 TX (Serial debug output)
- **GPIO 3**: UART0 RX (Serial debug input)  
- **GPIO 5**: Strapping pin (boot mode selection)
- **GPIO 6-11**: Connected to internal SPI flash (DO NOT USE)

### Relay Module
- **Type**: 8-channel opto-isolated relay module
- **Control Logic**: Active-LOW (production) / Active-HIGH (testing)
- **Safety**: All relays OFF on power-up, boot, or crash
- **Switching**: Minimized - only changes on state transitions

### Outputs
- **Navigation Lights**: Masthead, port sidelight, starboard sidelight, sternlight
- **Special Lights**: All-round white (anchorage), 2× all-round red (NUC - Not Under Command)
- **Sound Signal**: Horn/buzzer for COLREGs sound patterns

### Communication
- **Primary**: WiFi (SignalK server via SensESP framework)
- **Network**: DHCP (10.100.100.100-250 range)
- **Hostname**: `nav-lights-ecu.local` (mDNS)
- **Web UI**: 
  - Built-in SensESP configuration portal (WiFi and SignalK settings)
  - Custom fallback control UI at `/lights` (works when SignalK unavailable)
  - Access from any device on boat's WiFi network via browser
