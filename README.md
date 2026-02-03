# Navigation Lights and Signal ECU

ESP32-based controller for COLREGs-compliant navigation lights and sound signals on pleasure boats <15m.

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
# Run all tests
pio test

# Run tests on host (fast, CI-friendly)
pio test -e native
```

## Project Structure
```
├── src/
│   ├── main.cpp              # Main entry point
│   ├── interfaces/           # Hardware abstraction layer
│   │   ├── IRelayController.h
│   │   └── ITimer.h
│   ├── state_machine.cpp/.h  # COLREGs state management (TODO)
│   ├── light_controller.cpp/.h # Light relay control (TODO)
│   └── sound_controller.cpp/.h # Horn/signal control (TODO)
├── test/                     # Unit tests (TODO)
├── platformio.ini           # PlatformIO configuration
└── Project Proposal...md    # COLREGs requirements & tables
```

## Documentation
- See [.github/copilot-instructions.md](.github/copilot-instructions.md) for architecture & development guidelines
- See [Project Proposal Navigation Lights and Signaling ECU.md](Project%20Proposal%20Navigation%20Lights%20and%20Signaling%20ECU.md) for COLREGs rules & requirements

## Hardware
- **MCU**: AZ-Delivery ESP32 Dev Kit C V4
- **Relays**: Opto-isolated module (active-low control)
- **Outputs**: Masthead, sidelights, sternlight, all-round lights, NUC lights, horn

## Status
🚧 **Phase 1 Complete**: Project foundation with PlatformIO setup and hardware abstractions
