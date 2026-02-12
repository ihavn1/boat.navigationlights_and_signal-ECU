# Hardware Configuration - Navigation Lights ECU

## ESP32 Development Board

### Board Specifications
- **Model**: AZ-Delivery ESP32 Dev Kit C V4
- **Microcontroller**: ESP32-D0WD-V3
- **Architecture**: Dual-core Xtensa LX6
- **Clock Speed**: 240 MHz (both cores)
- **SRAM**: 520 KB
- **Flash Memory**: 4 MB
- **Operating Voltage**: 3.3V (logic level)
- **Input Voltage**: 5V via USB or VIN pin

### Wireless Capabilities
- **WiFi**: 802.11 b/g/n, 2.4 GHz only (SignalK + web UI)
- **Bluetooth**: BLE 4.2 (not used - WiFi-only design)
- **Antenna**: Built-in PCB trace antenna

---

## GPIO Pin Assignment

### Relay Control Outputs (9 Channels)

All relay outputs use **active-LOW logic** by default for production use with opto-isolated relay modules. This ensures relays are safely OFF (HIGH state) on boot, reset, or firmware crash.

| GPIO | Physical Pin | Function | Relay Module | COLREGs Rule | Purpose |
|------|--------------|----------|--------------|--------------|---------|
| **GPIO 25** | D25 | Masthead Light | CH1 | Rule 23 | White light forward, power-driven vessel |
| **GPIO 26** | D26 | Port Sidelight | CH2 | Rule 21(a) | Red light (port side, left) |
| **GPIO 27** | D27 | Starboard Sidelight | CH3 | Rule 21(a) | Green light (starboard side, right) |
| **GPIO 14** | D14 | Sternlight | CH4 | Rule 21(c) | White light aft (rear) |
| **GPIO 12** | D12 | All-round White | CH5 | Rule 30 | Anchorage light (visible 360°) |
| **GPIO 13** | D13 | All-round Red Upper | CH6 | Rule 27(a) | NUC upper red light (vertical) - **Optional** |
| **GPIO 15** | D15 | All-round Red Lower | CH7 | Rule 27(a) | NUC lower red light (vertical) - **Optional** |
| **GPIO 32** | D32 | Yellow Towing Light | CH8 | Rule 24 | Yellow light above sternlight (towing) - **Optional** |
| **GPIO 4** | D4 | Horn / Sound Signal | CH9 | Rule 35 | Sound signal device |

#### Optional Hardware (Runtime Configurable)

NUC and Towing lights are **optional** and can be configured via the SensESP web interface at `http://192.168.4.1/config`:

- **NUC Lights** (GPIO 13, 15): Configure if boat has NUC (Not Under Command) lights installed
- **Towing Lights** (GPIO 32): Configure if boat has yellow towing light installed
- All boat states remain selectable (for day shape reminders) even without lights installed
- Configuration persists across reboots (stored in LittleFS partition)

#### Relay Logic Modes

**Production Mode (Active-LOW)** - Default:
- `HIGH` (3.3V) = Relay OFF (safe state)
- `LOW` (0V) = Relay ON
- Use with: Opto-isolated relay modules (most common)

**Testing Mode (Active-HIGH)** - Optional:
- `LOW` (0V) = Relay OFF (safe state)
- `HIGH` (3.3V) = Relay ON
- Enable by uncommenting `ACTIVE_HIGH_RELAYS` in [platformio.ini](platformio.ini)
- Use with: LED test rigs, active-high relays

---

## System Pins (Auto-Configured)

### Status Indicator
| GPIO | Function | Direction | Used By | Behavior |
|------|----------|-----------|---------|----------|
| **GPIO 2** | Built-in LED | Output | SensESP | Blinks when transmitting SignalK data |

### Serial Console (Debug)
| GPIO | Function | Direction | Baud Rate | Purpose |
|------|----------|-----------|-----------|---------|
| **GPIO 1** | UART0 TX | Output | 115200 | Serial debug output |
| **GPIO 3** | UART0 RX | Input | 115200 | Serial debug input |

*Connected to USB-to-Serial chip for `pio device monitor`*

---

## Reserved Pins (DO NOT USE)

### Strapping Pins (Boot Mode Selection)
These pins are sampled at boot to determine operating mode:

| GPIO | Function | Boot Requirement | Safe for Runtime? |
|------|----------|------------------|-------------------|
| **GPIO 0** | BOOT Button | Must be HIGH | ⚠️ Avoid (boot mode) |
| **GPIO 2** | Built-in LED | - | ✅ Used by SensESP |
| **GPIO 5** | Strapping | - | ⚠️ Avoid (timing sensitive) |
| **GPIO 12** | Strapping (MTDI) | - | ✅ Used for relay (tested safe) |
| **GPIO 15** | Strapping (MTDO) | Must be HIGH | ✅ Used for relay (tested safe) |

**Note**: GPIO 12 and 15 are strapping pins but validated safe for relay use in this application.

### Internal SPI Flash (NEVER USE)
| GPIO | Function | Reason |
|------|----------|--------|
| **GPIO 6** | Flash SCLK | Wired to SPI flash chip |
| **GPIO 7** | Flash SD0 | Wired to SPI flash chip |
| **GPIO 8** | Flash SD1 | Wired to SPI flash chip |
| **GPIO 9** | Flash SD2 | Wired to SPI flash chip |
| **GPIO 10** | Flash SD3 | Wired to SPI flash chip |
| **GPIO 11** | Flash CMD | Wired to SPI flash chip |

**⚠️ CRITICAL**: Using GPIO 6-11 will brick the device and prevent code execution.

### Additional Unavailable Pins
| GPIO | Reason |
|------|--------|
| **GPIO 16-17** | PSRAM (not populated on this board) |
| **GPIO 20, 24, 28-31, 37-39** | Not exposed on ESP32-D0WD-V3 |

---

## Available Unused Pins

### General Purpose I/O (Not Currently Used)
If future expansion is needed, these pins are available:

| GPIO | Notes |
|------|-------|
| **GPIO 5** | Strapping pin - use with caution |
| **GPIO 18** | Good for SPI SCLK |
| **GPIO 19** | Good for SPI MISO |
| **GPIO 21** | I2C SDA (if needed) |
| **GPIO 22** | I2C SCL (if needed) |
| **GPIO 23** | Good for SPI MOSI |
| **GPIO 32** | ADC1_CH4, can read analog 0-3.3V |
| **GPIO 33** | ADC1_CH5, can read analog 0-3.3V |
| **GPIO 34** | Input only, ADC1_CH6 |
| **GPIO 35** | Input only, ADC1_CH7 |
| **GPIO 36 (VP)** | Input only, ADC1_CH0 |
| **GPIO 39 (VN)** | Input only, ADC1_CH3 |

*Future possibilities: Oil pressure sensor, coolant temperature, bilge flood detection, etc.*

---

## Relay Module Connection

### Recommended Module
- **Type**: 8 or 16-channel opto-isolated relay module (9 channels required)
- **Control Logic**: Active-LOW trigger (standard for most modules)
- **Isolation**: Optical isolation between MCU and relay coils
- **Relay Type**: SPDT (Single Pole Double Throw) or SPST
- **Common Models**: SainSmart 16-channel, HiLetgo 16-relay, or use two 8-channel modules

### Wiring Schematic

```
ESP32 Dev Kit C V4          9+ Channel Relay Module
┌─────────────────┐         ┌──────────────────────┐
│                 │         │                      │
│  GPIO 25  ──────┼─────────┤ IN1  (Masthead)      │
│  GPIO 26  ──────┼─────────┤ IN2  (Port Side)     │
│  GPIO 27  ──────┼─────────┤ IN3  (Starboard)     │
│  GPIO 14  ──────┼─────────┤ IN4  (Stern)         │
│  GPIO 12  ──────┼─────────┤ IN5  (All-round Wht) │
│  GPIO 13  ──────┼─────────┤ IN6  (Red Upper)     │
│  GPIO 15  ──────┼─────────┤ IN7  (Red Lower)     │
│  GPIO 32  ──────┼─────────┤ IN8  (Yellow Towing) │
│  GPIO 4   ──────┼─────────┤ IN9  (Horn)          │
│                 │         │                      │
│  GND      ──────┼─────────┤ GND                  │
│  5V (VIN) ──────┼─────────┤ VCC  (if module      │
│                 │         │       needs 5V)      │
└─────────────────┘         └──────────────────────┘
```

### Power Considerations
- **ESP32 Current**: ~80-260 mA (WiFi active)
- **Relay Module**: Typically draws 15-20 mA per active relay
- **Recommended Supply**: 2A @ 5V minimum (when all relays active)
- **USB Power**: Sufficient for testing; use external PSU for marine deployment

---

## Network Configuration

### WiFi Setup (First Boot)
1. ESP32 creates Access Point: **"Configure nav-lights-ecu"**
2. Connect with phone/laptop
3. Captive portal opens automatically (or go to 192.168.4.1)
4. Enter WiFi SSID and password
5. Configure SignalK server IP address
6. Save and reboot

### Production Network
- **DHCP Range**: 10.100.100.100 - 10.100.100.250
- **Expected IP**: Assigned by DHCP server on boat network
- **Hostname**: `nav-lights-ecu.local` (mDNS)
- **SignalK Server**: Configurable via web UI (no hardcoded IP)

### Accessing the Device
After WiFi configuration:
- **Web UI**: `http://nav-lights-ecu.local` or `http://<DHCP-IP>`
- **SignalK Path**: `electrical.switches.navigationLights.*`
- **Serial Console**: `pio device monitor` (115200 baud)

---

## Safety Features

### Hardware Safety
1. **Active-LOW Default**: All relay pins initialize HIGH (relays OFF) before relay module powers up
2. **Brownout Protection**: ESP32 built-in brownout detector resets MCU on low voltage
3. **Watchdog Timer**: ESP32 hardware watchdog prevents firmware hangs

### Software Safety
1. **Boot State**: All relays OFF, periodic sound signals muted
2. **Emergency Stop**: Single function (`deactivateAll()`) turns off all outputs
3. **State Validation**: Invalid state transitions rejected by state machine
4. **Minimal Switching**: Relays only change on explicit state changes (not every loop)

---

## Testing & Validation

### LED Test Rig (Active-HIGH Mode)
For bench testing without relays:
1. Edit [platformio.ini](platformio.ini)
2. Uncomment: `-D ACTIVE_HIGH_RELAYS`
3. Connect LEDs: GPIO → 220Ω resistor → LED → GND
4. Flash firmware: `pio run --target upload`
5. LEDs will light when corresponding relay would activate

### Production Deployment Checklist
- [ ] Comment out `ACTIVE_HIGH_RELAYS` flag (back to active-LOW)
- [ ] Reflash firmware
- [ ] Verify serial output shows "Relay Mode: ACTIVE-LOW (Production)"
- [ ] Test with multimeter: All GPIO pins HIGH at boot (3.3V)
- [ ] Connect relay module
- [ ] Test COLREGs light patterns (Day/Moored, Underway, Anchorage, NUC)
- [ ] Validate sound signal timing (short blast ~1s, prolonged 4-6s)
- [ ] Configure WiFi and SignalK server via web UI
- [ ] Verify SignalK bidirectional communication

---

## Troubleshooting

### Device Won't Boot
- **Check GPIO 0**: Must be HIGH (not shorted to GND)
- **Check GPIO 12**: Verify not pulled LOW during boot
- **Serial Output**: Connect `pio device monitor` to see boot messages

### WiFi Won't Connect
- **AP Mode**: Look for "Configure nav-lights-ecu" WiFi network
- **Serial Console**: Check for error messages (wrong password, out of range)
- **DHCP**: Ensure router/server is assigning 10.100.100.x addresses

### Relays Stuck ON
- **Check Mode**: Verify `ACTIVE_HIGH_RELAYS` flag matches hardware
- **Wiring**: Confirm relay module uses same logic (most are active-LOW)
- **Power Cycle**: ESP32 initializes pins HIGH by default (safe for active-LOW)

### Built-in LED Always ON
- **Different Board**: Some ESP32 variants use active-LOW LED (HIGH = ON)
- **Pin Conflict**: Verify not using GPIO 2 for anything else
- **Expected**: LED stays ON if not connected to SignalK server yet

---

## Expansion Ideas

### Future Hardware Enhancements
- **GPS Module**: UART2 on GPIO 16/17 for COLREGs autopilot rules
- **Oil Pressure Sensor**: Analog input GPIO 32/33
- **Engine Temperature**: I2C temp sensor on GPIO 21/22
- **Bilge Alarm**: Digital input GPIO 34/35
- **Bluetooth**: ESP32 has BLE 4.2 capability (currently unused)

### Additional Relays
If more than 8 channels needed:
- **I2C I/O Expander**: PCF8574 or MCP23017 on GPIO 21/22
- **Shift Register**: 74HC595 for 16+ outputs

---

## References

- [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [ESP32 Pinout Diagram](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [SensESP Documentation](https://signalk.org/SensESP/)
- [COLREGs - International Maritime Rules](https://www.imo.org/en/About/Conventions/Pages/COLREG.aspx)
