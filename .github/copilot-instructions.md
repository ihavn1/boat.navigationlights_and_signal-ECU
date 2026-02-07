# Navigation Lights and Signal ECU - AI Coding Instructions

## Project Overview
ESP32-based ECU for controlling navigation lights and sound signals on pleasure boats <15m, implementing COLREGs (International Maritime Organization rules). Built on SensESP platform using SignalK protocol for communication.

## Architecture & Core Components

### Hardware Stack
- **MCU**: AZ-Delivery ESP32 Dev Kit C V4
- **Outputs**: Opto-isolated relay module (active-low control) for navigation lights (masthead, sidelights, sternlight, all-round, NUC lights) and horn
- **Communication**: WiFi (SignalK), optional Bluetooth BLE (fallback UI)

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
- **Test coverage priorities**:
  1. State machine transitions (all condition+state combinations)
  2. COLREGs rule mapping (verify correct lights/signals per state)
  3. Sound signal timing (blast durations, periodic intervals)
  4. Safety conditions (relay states on boot, invalid transitions)
- **Run tests**: `pio test` (runs on host or target device)
- **CI consideration**: Structure tests to run on native platform (x86) for fast feedback

## COLREGs Implementation Rules

### Light Control Patterns
- **Making way (darkness)**: Sidelights + Sternlight + Masthead light
- **NUC (darkness)**: Two all-round red lights (vertical) + sidelights/sternlight if making way
- **Anchorage (darkness)**: Single all-round white light
- Reference the state table in `Project Proposal Navigation Lights and Signaling ECU.md` for complete mapping

### Sound Signal Requirements
- **Periodic signals**: Auto-repeat at specified intervals (e.g., 2min), with mute/unmute capability (always start muted on boot)
- **Countdown timer**: Track seconds until next periodic signal (expose via SignalK)
- **Semi-automatic signals**: One-shot blasts triggered ad-hoc (e.g., "●" = 1 turn to starboard, "●●●●●" = danger/confusion)
- **Timing**: Short blast ≈1s, Prolonged blast 4-6s, Pause 1s

## Critical Development Workflows

### Build & Flash
- **Required**: Use PlatformIO (standard for SensESP projects - better dependency management and ESP32 toolchain)
- Target: `esp32dev` board for ESP32 Dev Kit C V4
- Dependencies: SensESP library (includes SignalK client)
- Build: `pio run` | Flash: `pio run --target upload` | Monitor: `pio device monitor`

### Testing Workflow
- **Framework**: Unity (PlatformIO built-in), run with `pio test` or `pio test -e native` for host testing
- **Test directory**: `test/test_*.cpp` files (e.g., `test_state_machine.cpp`, `test_light_controller.cpp`)
- **Safety tests**: Verify all relays inactive on boot (active-low requirement)
- **State transitions**: Test each condition+state combination against COLREGs table
- **Timing tests**: Validate sound signal durations and periodic intervals
- **Mock hardware**: Use dependency injection to test controllers without physical relays

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
- Implement watchdog/heartbeat to detect firmware hangs
- Validate state transitions before activating relays (avoid invalid light combinations)

### File Organization (when implemented)
- `src/main.cpp`: SensESP initialization, SignalK setup, main loop
- `src/state_machine.cpp/.h`: Condition+state management and COLREGs rule engine
- `src/light_controller.cpp/.h`: Relay control for navigation lights
- `src/sound_controller.cpp/.h`: Horn control with timing for blasts and periodic signals
- `src/interfaces/`: Hardware abstraction interfaces (IRelayController, ITimer) for testability
- `test/test_*.cpp`: Unity unit tests for each component
- `platformio.ini`: ESP32 configuration, SensESP dependency, test environment setup

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
- Android BLE app as backup when WiFi/SignalK unavailable
- BLE service should mirror SignalK control surface (condition, state, signals)

## Key Files & References
- [HARDWARE.md](../HARDWARE.md) - **Complete GPIO pin mapping, reserved pins, wiring diagrams**
- [Project Proposal Navigation Lights and Signaling ECU.md](../Project%20Proposal%20Navigation%20Lights%20and%20Signaling%20ECU.md) - Complete COLREGs tables, signal definitions, and requirements
- Platform docs: SensESP GitHub repo, ESP32 Arduino core docs
- COLREGs reference: Convention on International Regulations for Preventing Collisions at Sea, 1972

## Development Priorities
1. Implement state machine with COLREGs rule mapping (with unit tests)
2. Light relay control with safety (active-low, boot state validation) + tests
3. Sound signal timing engine (short/prolonged blasts, periodic repeats) + tests
4. SignalK integration (bidirectional state sync)
5. Countdown timer for periodic signals
6. Mute/unmute functionality for periodic signals
7. (Optional) BLE fallback UI

**Note**: Write unit tests in parallel with each component - use TDD for state machine logic
