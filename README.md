# Navigation Lights and Signal ECU

ESP32-based controller for COLREGs-compliant navigation lights and sound signals on pleasure boats <15m. Integrated with SignalK for remote monitoring and control.

## Status: ✅ Code Complete + Web UI Ready for Upload 🌐

**Test Coverage**: 151 tests expected (122 unit + 29 Web API integration)  
**Build**: Successful (72.7% flash, 9.4% RAM)  
**Code Status**: Complete and validated via comprehensive automated testing  
**Hardware Status**: Running on ESP32 with Web API operational  
**SignalK**: Full bidirectional integration with periodic updates  
**Web UI**: Phases 6-8 complete - Backend API + responsive frontend deployed  
**Latest**: Towing state added (COLREGs Rule 24) with full test coverage

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

### Upload Web UI Files
```powershell
# Build SPIFFS filesystem from data/ folder
# (contains lights.html, lights.css, lights.js, iha-logo.png)
pio run --target buildfs

# Upload web files to ESP32 SPIFFS partition
pio run --target uploadfs

# Monitor boot messages to verify filesystem mount
pio device monitor
```

**Web UI Files** (data/ folder):
- `lights.html` - Main UI (16.4KB) with boat diagram and COLREGs day shapes
- `lights.css` - Styling (16.8KB) with animated backgrounds
- `lights.js` - Frontend logic (20.8KB) with progress bar and real-time updates
- `iha-logo.png` - Company logo (18.4KB)

**Access web UI**: `http://nav-lights-ecu.local/lights` (or `http://10.100.100.244/lights`)  
**Complete testing guide**: [PHASE3_SPIFFS_UPLOAD.md](PHASE3_SPIFFS_UPLOAD.md)

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

## Over-The-Air (OTA) Updates

Once the ESP32 is connected to WiFi, firmware and filesystem can be updated wirelessly without USB cable access.

### Initial Setup (USB Required)
```bash
# First-time flash via USB
pio run --target upload

# Upload web UI files via USB
pio run --target uploadfs
```

### Wireless Updates (After Initial Setup)
```bash
# Update firmware over WiFi
pio run --target upload --upload-port nav-lights-ecu.local

# Update web UI files over WiFi
pio run --target uploadfs --upload-port nav-lights-ecu.local
```

**OTA Authentication**: 
- Password automatically read from `src/secrets.h` (define `OTA_PASSWORD`)
- Configured via `read_secrets.py` script (no passwords in git-tracked files)
- If hostname resolution fails, use IP address: `--upload-port 10.100.100.244`

**Requirements**:
- ESP32 powered on and connected to same network
- mDNS/Bonjour enabled (for `.local` hostname resolution)
- Firewall allows mDNS (port 5353) and OTA (port 3232)

**Safety**: If OTA fails, ESP32 keeps running old firmware. Fall back to USB if needed.

**Use Case**: Particularly useful when ECU is installed in hard-to-reach locations on the boat.

## Project Structure

```
boat.navigationlights_and_signal-ECU/
├── src/                           # C++ source code
│   ├── interfaces/                 # Hardware abstraction layer
│   │   ├── IRelayController.h      # Relay interface
│   │   └── ITimer.h                # Timer interface
│   ├── state_machine.cpp/.h        # COLREGs state management (✅ 20 tests)
│   ├── LightController.cpp/.h      # Light relay control (✅ 9 tests)
│   ├── SoundController.cpp/.h      # Horn/signal control (✅ 17 tests)
│   ├── NavigationLightsECU.cpp/.h  # ECU facade (✅ tested)
│   ├── signalk_integration.cpp/.h  # SignalK layer (✅ 76 tests)
│   ├── web_api.cpp/.h              # REST API backend (✅ 29 tests)
│   ├── ESP32RelayController.cpp/.h # GPIO implementation
│   ├── ESP32Timer.cpp/.h           # FreeRTOS timer
│   └── main.cpp                    # SensESP initialization
├── data/                          # Web UI files (uploaded to SPIFFS)
│   ├── lights.html                 # Main UI (16.4KB)
│   ├── lights.css                  # Styles with animations (16.8KB)
│   ├── lights.js                   # Frontend logic (20.8KB)
│   └── iha-logo.png                # Company logo (18.4KB)
├── test/                          # Unit and integration tests
│   ├── test_state_machine/         # 23 COLREGs tests (includes towing)
│   ├── test_light_controller/      # 9 relay tests
│   ├── test_sound_controller/      # 17 timing tests (includes SOS)
│   ├── test_signalk_integration/   # 76 SignalK tests (includes SOS)
│   └── test_signalk_esp32/         # 14 ESP32 hardware tests (⚠️ pending re-run)
├── test_web_api.ps1               # 29 Web API integration tests (✅ passing)
├── platformio.ini                 # Build configuration + custom partitions
├── partitions_custom.csv          # Flash layout (single SPIFFS for config + web UI)
└── Project Proposal...md          # COLREGs requirements
```

## Hardware Configuration

The ECU supports **runtime configuration** of optional light hardware via the SensESP web interface:

### Configuring Hardware
1. Navigate to `http://192.168.4.1/config` (or `http://nav-lights-ecu.local/config`)
2. Find the **Hardware** section
3. Configure available hardware:
   - **NUC Lights**: Check if boat has NUC (Not Under Command) lights installed
   - **Towing Lights**: Check if boat has yellow towing light installed
4. Click **Save** - ESP32 will restart and load configuration
5. Configuration persists across reboots (stored in SPIFFS)

**Note**: If you change the partition table, run a full flash erase before uploading. This clears old filesystem data that may no longer align with the new layout.

```bash
pio run -t erase
pio run -t upload
pio run -t uploadfs
```

### Effects of Configuration
- **State Selection**: All boat states (NUC, Towing, etc.) remain available for **day shape reminders** (black balls/diamonds)
- **Light Control**: When hardware is not installed, lights are simply not activated (relays stay OFF)
- **Web UI**: Fallback interface automatically hides unavailable light indicators in boat diagram
- **SignalK**: Full state information always published regardless of hardware configuration

This allows the ECU to be used on any boat configuration, from minimal (masthead, sidelights, sternlight only) to fully-equipped commercial vessels.

## Security

The SensESP web UI supports Digest Authentication for configuration pages.

- **Protected (LAN)**: When accessed via normal WiFi (e.g., `nav-lights-ecu.local`), `/config` requires Digest Auth credentials.
- **Not protected (AP mode)**: When connected to the device AP at `192.168.4.1`, SensESP bypasses auth for the captive portal.
- **Required credentials**: Copy [src/secrets.example.h](src/secrets.example.h) to `src/secrets.h` and set `ADMIN_USER`/`ADMIN_PASS`. The `src/secrets.h` file is ignored by git.

If you need the AP mode protected as well, the HTTP server auth bypass can be removed in firmware.

## Features

### COLREGs Compliance
- ✅ Rules 20, 21, 23, 24, 25, 27, 30, 35 implemented
- ✅ 21 condition/state combinations validated (7 boat states × 3 conditions)
- ✅ Navigation lights (masthead, sidelights, sternlight) - **Required**
- ✅ Yellow towing light (Rule 24 - above sternlight) - **Optional, runtime-configurable**
- ✅ NUC lights (2× red all-round vertical) - **Optional, runtime-configurable**
- ✅ Anchorage light (all-round white) - **Required**
- ✅ Sound signals (short/prolonged blasts, periodic patterns)
- ✅ Ad-hoc signal queueing (auto-plays after 2s delay if signal in progress)
- ✅ SOS ad-hoc signal (●●● ▬▬ ▬▬ ▬▬ ●●●) with horn-synced masthead + anchor flashing in darkness/restricted visibility

### SignalK Integration
- ✅ Bidirectional communication (PUT requests + delta p

### Web UI (Fallback Control) 🌐
- ✅ **Backend API**: 7 REST endpoints (`/api/*`) with 29/29 tests passing
- ✅ **Frontend**: Responsive web interface (HTML/CSS/JS) - deployed
- ✅ **Mobile-First Design**: Touch-friendly controls, dark marine theme
- ✅ **Real-Time Updates**: 2-second polling with auto-reconnection
- ✅ **Complete Control**: Condition, boat state, mute, ad-hoc signals (including SOS), emergency stop
- ✅ **Visual Feedback**: Progress bar for periodic signal countdown, animated backgrounds
- ✅ **System Monitoring**: Uptime, heap, WiFi RSSI, connection status
- ✅ **Phase 8 Complete**: Frontend deployed with starboard boat diagram and COLREGs day shapes

**Access URL**:
- Primary: `http://nav-lights-ecu.local/lights`
- Direct IP: `http://10.100.100.244/lights`
- Emergency AP: `http://192.168.4.1/lights`

**Features**: Animated day/night/fog backgrounds, starboard boat diagram with navigation lights, black ball day shapes (COLREGs Rules 27 & 30), guarded SOS control, visual progress bar for periodic signal timing

**Testing**: See [PHASE3_SPIFFS_UPLOAD.md](PHASE3_SPIFFS_UPLOAD.md) for upload and testing guideublishing)
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
- [docs/TOWING_STATE.md](docs/TOWING_STATE.md) - Towing state implementation (COLREGs Rule 24)
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

#### Relay Outputs (9 channels)
| GPIO | Function | Relay | COLREGs Purpose | Polarity |
|------|----------|-------|-----------------|----------|
| **GPIO 25** | Masthead Light | CH1 | Rule 23 - Power-driven vessel | Active-LOW* |
| **GPIO 26** | Port Sidelight | CH2 | Rule 21 - Red (port side) | Active-LOW* |
| **GPIO 27** | Starboard Sidelight | CH3 | Rule 21 - Green (starboard) | Active-LOW* |
| **GPIO 14** | Sternlight | CH4 | Rule 21 - White (aft) | Active-LOW* |
| **GPIO 12** | All-round White | CH5 | Rule 30 - Anchorage | Active-LOW* |
| **GPIO 13** | All-round Red Upper | CH6 | Rule 27 - NUC (upper) | Active-LOW* |
| **GPIO 15** | All-round Red Lower | CH7 | Rule 27 - NUC (lower) | Active-LOW* |
| **GPIO 32** | Yellow Towing Light | CH8 | Rule 24 - Towing (above stern) | Active-LOW* |
| **GPIO 4** | Horn | CH9 | Rule 35 - Sound signals | Active-LOW* |

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
- **Type**: 9 or 16-channel opto-isolated relay module (upgraded from 8-channel for towing light)
- **Control Logic**: Active-LOW (production) / Active-HIGH (testing)
- **Safety**: All relays OFF on power-up, boot, or crash
- **Switching**: Minimized - only changes on state transitions

### Outputs
- **Navigation Lights**: Masthead, port sidelight, starboard sidelight, sternlight
- **Special Lights**: All-round white (anchorage), 2× all-round red (NUC - Not Under Command), yellow towing light (Rule 24)
- **Sound Signal**: Horn/buzzer for COLREGs sound patterns

### Communication
- **Primary**: WiFi (SignalK server via SensESP framework)
- **Network**: DHCP (10.100.100.100-250 range)
- **Hostname**: `nav-lights-ecu.local` (mDNS)
- **Web UI**: 
  - Built-in SensESP configuration portal (WiFi and SignalK settings)
  - Custom fallback control UI at `/lights` (works when SignalK unavailable)
  - Access from any device on boat's WiFi network via browser
