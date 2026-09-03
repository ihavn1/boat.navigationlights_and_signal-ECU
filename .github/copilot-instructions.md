# boat.helm-ecu - AI Coding Instructions

## Project Overview
ESP32-based ECU for controlling navigation lights and sound signals on pleasure boats <15m, implementing COLREGs (International Maritime Organization rules). Built on SensESP platform using SignalK protocol for communication.

## Architecture & Core Components

### Hardware Stack
- **MCU**: AZ-Delivery ESP32 Dev Kit C V4
- **Outputs**: Opto-isolated relay module (active-low control) for navigation lights (masthead, sidelights, sternlight, all-round, NUC lights) and horn
- **Communication**: WiFi only (SignalK primary, web UI fallback)

### Software Stack
- **Framework**: SensESP (ESP32 framework wrapping SignalK)
- **Protocol**: SignalK for state management and remote control
- **Safety**: Active-low relay control (relays OFF on boot/crash)

### State Machine Design
The system operates on two-dimensional state:
1. **Conditions**: Day / Hours of darkness / Restricted visibility
2. **Boat State**: Moored / Underway (making way/no way) / Anchorage / Not Under Command (making way/no way)

Each condition+state combination maps to specific light configurations and periodic sound signal patterns (see project proposal table).

## Code Quality & Testing

### SOLID Principles (Apply Where Appropriate)
- **Single Responsibility**: Separate concerns - state machine, light control, sound control, SignalK communication
- **Open/Closed**: Design for COLREGs rule extensions without modifying core logic
- **Dependency Inversion**: Abstract hardware interfaces (relay control, timer) for testability
- **Interface Segregation**: Minimal, focused interfaces between components

### Clean Code Practices
- Descriptive names matching COLREGs terminology (e.g., `MastheadLight`, `ProlongedBlast`)
- Small, focused functions (embedded constraints considered)
- Avoid magic numbers - use named constants for timings (e.g., `SHORT_BLAST_MS = 1000`)
- Comment COLREGs rule references (e.g., `// Rule 25(d): NUC vessel shows two all-round red lights`)

### Unit Testing Strategy
- **Framework**: Use PlatformIO's Unity test framework (native to embedded projects)
- **Parallel development**: Write tests alongside implementation (TDD encouraged for state machine logic)
- **Test structure**: `test/` directory with test files per component
- **Mock hardware**: Abstract GPIO/relay control behind interfaces for testing without hardware
- **Platform-independent logging**: Use DEBUG_PRINT/PRINTLN/PRINTF macros that map to Serial.x on ESP32, printf on native (conditional compilation with NATIVE_BUILD flag)
- **Test coverage priorities**:
  1. State machine transitions (all condition+state combinations)
  2. COLREGs rule mapping (verify correct lights/signals per state)
  3. Sound signal timing (blast durations, periodic intervals, queueing behavior)
  4. Safety conditions (relay states on boot, invalid transitions, queue clearing)
  5. Unmute immediate playback and countdown reset
- **Run tests**: `pio test -e native` for fast native testing, `pio test -e esp32test` for hardware validation
- **CI consideration**: Structure tests to run on native platform (x86) for fast feedback
- **Hardware timing tests**: Some timing-dependent features (e.g., ad-hoc queueing delay) require ESP32 hardware validation due to MockTimer limitations

## COLREGs Implementation Rules

### Light Control Patterns
- **Making way (darkness)**: Sidelights + Sternlight + Masthead light
- **NUC (darkness)**: Two all-round red lights (vertical) + sidelights/sternlight if making way
- **Anchorage (darkness)**: Single all-round white light
- Reference the state table in `Project Proposal boat.helm-ecu.md` for complete mapping

### Sound Signal Requirements
- **Periodic signals**: Auto-repeat at specified intervals (e.g., 2min), with mute/unmute capability (always start muted on boot)
- **Unmute behavior**: When unmuting, signal plays immediately and countdown resets to full interval (for immediate COLREGs compliance)
- **Countdown timer**: Track seconds until next periodic signal (expose via SignalK)
- **Semi-automatic signals**: One-shot blasts triggered ad-hoc (e.g., "●" = 1 turn to starboard, "●●●●●" = danger/confusion)
- **Ad-hoc queueing**: If ad-hoc signal requested while signal playing, queues and plays automatically after 2-second delay when current signal completes (prevents overlapping horn signals)
- **Timing**: Short blast ≈1s, Prolonged blast 4-6s, Pause 1s, Queue delay 2s
- **Rule 35 signals**: Making way = 1 prolonged blast (Rule 35a), Making no way/stopped = 2 prolonged blasts (Rule 35b)

## Critical Development Workflows

### Build & Flash
- **Required**: Use PlatformIO (standard for SensESP projects - better dependency management and ESP32 toolchain)
- Target: `esp32dev` board for ESP32 Dev Kit C V4
- Dependencies: SensESP library (includes SignalK client)
- Build: `pio run` | Flash: `pio run --target upload` | Monitor: `pio device monitor`
- **OTA Updates**: After initial USB flash, wireless updates via `pio run --target upload --upload-port boat-helm-ecu.local`
  - Password authentication from `src/secrets.h` (OTA_PASSWORD)
  - Auto-configured via `read_secrets.py` pre-upload script
  - Filesystem updates: `pio run --target uploadfs --upload-port boat-helm-ecu.local`

### Testing Workflow
- **Framework**: Unity (PlatformIO built-in), run with `pio test -e native` for fast host testing
- **Test directory**: `test/test_*/` folders with test files and mock implementations
- **Current status**: 119/119 tests passing (20 state machine + 9 light + 16 sound + 74 signalk)
- **Safety tests**: Verify all relays inactive on boot (active-low requirement), queue clearing on emergency stop
- **State transitions**: Test each condition+state combination against COLREGs table
- **Timing tests**: Validate sound signal durations, periodic intervals, and queueing behavior (full timing validation requires ESP32 hardware)
- **Mock hardware**: Dependency injection with MockRelayController and MockTimer for unit testing
- **Platform-independent code**: Use `#ifndef NATIVE_BUILD` guards and DEBUG macros for Serial calls
- **ESP32 hardware tests**: Run `pio test -e esp32test` to validate on actual hardware (14 tests, pending re-run after recent changes)

## Project-Specific Conventions

### SignalK Data Model
- **Path conventions**: Follow official SignalK schema (https://signalk.org/specification/latest/doc/)
- **Inputs**: Receive condition/state selection and ad-hoc signal commands from SignalK server
- **Outputs**: Publish current condition, boat state, active lights, next signal countdown
- **On boot/reconnect**: Send complete ECU status to SignalK server
- **Likely paths**: `electrical.switches.*` for light controls, `navigation.state` for boat state, `notifications.*` for signals

### Safety-First Patterns
- All relay GPIO pins configured as OUTPUT with initial state LOW before relay module init (opto-isolated module requires active-low for safety)
- Periodic sound signals always start muted on boot/restart (must be manually unmuted)
- Ad-hoc signals queue when signal in progress (prevents overlapping/interrupted horn signals)
- Emergency stop clears all queues and cancels pending timers
- Implement watchdog/heartbeat to detect firmware hangs
- Validate state transitions before activating relays (avoid invalid light combinations)

### File Organization (implemented)
- `src/main.cpp`: SensESP initialization, SignalK setup, main loop
- `src/NavigationLightsECU.cpp/.h`: Top-level ECU logic coordinating all components
- `src/state_machine.cpp/.h`: Condition+state management and COLREGs rule engine
- `src/LightController.cpp/.h`: Relay control for navigation lights
- `src/SoundController.cpp/.h`: Horn control with timing, queueing, and countdown management
- `src/signalk_integration.cpp/.h`: SignalK consumer/producer functions for bidirectional communication
- `src/ESP32RelayController.cpp/.h`: ESP32 GPIO implementation of IRelayController
- `src/ESP32Timer.cpp/.h`: ESP32 FreeRTOS timer implementation of ITimer
- `src/interfaces/`: Hardware abstraction interfaces (IRelayController, ITimer) for testability
- `test/test_*/`: Unity unit tests per component with mock implementations
- `platformio.ini`: ESP32 and native test environment configurations

## External Dependencies & Integration

### SensESP Framework
- Provides SignalK client, WiFi management, web UI scaffolding
- Check SensESP docs for standard patterns (sensors, transforms, outputs)
- Use SensESP's built-in WiFi config portal for network setup

### SignalK Path Standards
- **Critical**: Always validate against official SignalK specification: https://signalk.org/specification/latest/doc/
- Use standard paths from spec where available (e.g., `electrical.switches.*`, `navigation.lights.*`)
- For custom paths (countdown timer, mute state), follow SignalK naming conventions (camelCase, hierarchical)
- Document any non-standard paths in code comments with rationale

### Fallback UI Option
- Web-based control UI hosted on ESP32 as backup when SignalK unavailable
- Accessible at `http://boat-helm-ecu.local/lights` from any device on boat's WiFi network
- Custom HTTP endpoints on SensESP's AsyncWebServer
- Responsive HTML/JavaScript interface mirroring SignalK control surface
- Works in parallel with SensESP configuration UI
- No app installation required - browser-only access
- Emergency AP mode: `http://192.168.4.1/lights` if WiFi down
- Scalable pattern for multiple ECUs on boat

## Key Files & References
- [HARDWARE.md](../HARDWARE.md) - **Complete GPIO pin mapping, reserved pins, wiring diagrams**
- [Project Proposal boat.helm-ecu.md](../Project%20Proposal%20boat.helm-ecu.md) - Complete COLREGs tables, signal definitions, and requirements
- Platform docs: SensESP GitHub repo, ESP32 Arduino core docs
- COLREGs reference: Convention on International Regulations for Preventing Collisions at Sea, 1972

## Current Project Status

### Completed Features ✅
1. ✅ State machine with COLREGs rule mapping (20 tests passing)
2. ✅ Light relay control with safety (9 tests passing)
3. ✅ Sound signal timing engine with queueing (16 tests passing)
4. ✅ SignalK integration with bidirectional communication (74 tests passing)
5. ✅ Countdown timer for periodic signals
6. ✅ Mute/unmute functionality with immediate playback
7. ✅ Ad-hoc signal queueing system (2-second delay after signal completion)
8. ✅ Platform-independent debug logging for native tests
9. ✅ All 119 unit tests passing on native platform
10. ✅ Web API backend (7 REST endpoints, 29 tests passing)
11. ✅ Fallback web UI (HTML/CSS/JS, deployed to SPIFFS)
12. ✅ Towing state (COLREGs Rule 24) with full implementation
13. ✅ Runtime hardware configuration (NUC/Towing lights configurable via SensESP web UI)
14. ✅ OTA firmware updates (password-protected wireless updates via read_secrets.py)

### Pending Work ⚠️
1. **Hardware Integration Testing**: Validate queueing timing and prolonged blast duration on ESP32 hardware
2. **ESP32 Test Re-run**: Re-validate 14 embedded tests after recent changes (ad-hoc queueing, unmute immediate playback, Rule 35 corrections)
3. **SensESP Integration**: Complete main.cpp with SensESP setup and SignalK client configuration
4. **Hardware Wiring**: Connect relay module, verify GPIO mapping from HARDWARE.md
5. **Maritime Field Testing**: Validate COLREGs compliance in actual conditions

**Note**: All core logic validated via TDD - 148 tests total (119 unit + 29 web API integration) covering state machine, light control, sound control, SignalK, and web API
