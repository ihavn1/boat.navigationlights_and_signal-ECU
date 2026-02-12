# Runtime Hardware Configuration

## Overview

The Navigation Lights ECU supports **runtime configuration** of optional hardware through the SensESP web interface. This allows the same firmware to work on boats with different hardware configurations without recompilation.

## Configurable Hardware

### NUC (Not Under Command) Lights
- **GPIO Pins**: 13 (upper red) and 15 (lower red)
- **COLREGs**: Rule 27(a) - Two all-round red lights in vertical line
- **Default**: Installed (true)
- **Configuration Path**: `/Hardware/NUC_Lights`

### Towing Lights
- **GPIO Pin**: 32 (yellow towing light)
- **COLREGs**: Rule 24 - Yellow light above sternlight
- **Default**: Installed (true) 
- **Configuration Path**: `/Hardware/Towing_Lights`

## Configuration Interface

### Web UI Access
1. Navigate to SensESP configuration page:
   - `http://192.168.4.1/config` (Access Point mode)
   - `http://nav-lights-ecu.local/config` (WiFi client mode)
2. Find the **Hardware** section
3. Check/uncheck hardware options:
   - ☑ **NUC Lights Installed**
   - ☑ **Towing Lights Installed**
4. Click **Save Configuration**
5. ESP32 automatically restarts to apply changes

### Configuration Storage
- **Filesystem**: SPIFFS partition
- **Format**: JSON via SensESP's CheckboxConfig
- **Persistence**: Survives reboots, firmware updates preserve config
- **Location**: `/Hardware/NUC_Lights` and `/Hardware/Towing_Lights` in SPIFFS

### Configuration Security
- **Digest Auth (LAN)**: `/config` is protected when accessed via normal WiFi (e.g., `nav-lights-ecu.local`).
- **AP Mode**: SensESP bypasses auth for the captive portal at `192.168.4.1`.
- **Required credentials**: Copy [src/secrets.example.h](../src/secrets.example.h) to `src/secrets.h` and set `ADMIN_USER`/`ADMIN_PASS`.

## Implementation Details

### Software Architecture
```cpp
// Global configuration objects (main.cpp)
std::shared_ptr<CheckboxConfig> g_config_has_nuc;
std::shared_ptr<CheckboxConfig> g_config_has_towing;

// Runtime values (synchronized on boot)
bool g_has_nuc_lights;
bool g_has_towing_lights;

// SensESP registration
ConfigItem(g_config_has_nuc)
    ->set_title("NUC Lights")
    ->set_description("Does this boat have NUC lights installed?")
    ->set_requires_restart(true);
```

### Behavior by Configuration

#### When Hardware is **Installed** (checked)
- State can be selected via SignalK or web UI
- Lights activate according to COLREGs rules
- Light indicators visible in fallback web UI boat diagram
- Full COLREGs compliance maintained

#### When Hardware is **Not Installed** (unchecked)
- **State Selection**: Still available (for day shape reminders)
- **Relay Control**: Relays remain OFF (safe - no light activated)
- **Web UI**: Light indicators hidden in boat diagram
- **SignalK**: Full state information published (including config status)
- **Day Shapes**: Crew reminded to display black balls/diamonds

### Use Cases

#### Case 1: Minimal Pleasure Boat (<7m)
- **Installed**: Masthead, sidelights, sternlight, anchorage light
- **Not Installed**: NUC lights, towing lights
- **Benefit**: Same firmware as commercial vessels, no wasted GPIO

#### Case 2: Charter Yacht with Occasional Towing
- **Installed**: All standard lights + NUC lights
- **Not Installed**: Towing light (not needed)
- **Benefit**: Can select "Towing" state as day shape reminder

#### Case 3: Commercial Vessel (Fully Equipped)
- **Installed**: All lights including NUC and towing
- **Configuration**: All checkboxes enabled
- **Benefit**: Full COLREGs compliance for all operations

## Web API Integration

### Capabilities Endpoint
```json
GET /api/status
{
  "capabilities": {
    "hasNucLights": true,
    "hasTowingLights": false
  },
  "boatState": "underway_making_way",
  ...
}
```

### Frontend Adaptation
The fallback web UI (`/lights`) automatically:
1. Fetches capabilities on page load
2. Hides unavailable light indicators in boat diagram
3. Keeps all state buttons visible (for day shapes)
4. Updates in real-time based on configuration

## Testing

### Unit Tests
**Status**: No changes required ✅
- Tests validate LightController can apply any configuration
- Hardware abstraction (IRelayController) allows testing without GPIO
- Configuration logic tested separately from relay control

### Manual Testing Checklist
- [ ] Configure NUC lights OFF → verify relays 6,7 never activate in NUC state
- [ ] Configure towing lights OFF → verify relay 8 never activates in towing state
- [ ] Change configuration → verify restart occurs automatically
- [ ] Power cycle → verify configuration persisted
- [ ] Web UI → verify light indicators hidden when hardware not installed
- [ ] SignalK → verify state published correctly with config info

## Documentation Updates

Files updated to reflect runtime configuration:
- ✅ [README.md](../README.md) - Added Hardware Configuration section
- ✅ [HARDWARE.md](../HARDWARE.md) - Marked optional lights, added config section
- ✅ [docs/TOWING_STATE.md](TOWING_STATE.md) - Added runtime config note
- ✅ [.github/copilot-instructions.md](../.github/copilot-instructions.md) - Updated project status

## Future Enhancements

Potential additional configurable hardware:
- [ ] Masthead light (for sailing vessels <12m - optional)
- [ ] All-round yellow towing light count (1 for <50m, 2 for 50-200m, 3 for >200m)
- [ ] Sound signal device type (horn vs whistle vs bell)
- [ ] Relay polarity (active-low vs active-high per channel)

## Troubleshooting

### Configuration Not Persisting
- **Symptom**: Settings revert to defaults after restart
- **Cause**: SPIFFS not mounted or corrupted
- **Fix**: Check serial output for SPIFFS mount errors, reflash if needed

### Can't Access Config Page
- **Symptom**: `http://192.168.4.1/config` not loading
- **Cause**: Not connected to ECU's WiFi Access Point
- **Fix**: Connect to `nav-lights-ecu` WiFi network first

### Lights Activating When Disabled
- **Symptom**: NUC/Towing lights turn on despite configuration OFF
- **Cause**: Code not checking configuration flags
- **Fix**: Verify light controller checks `g_has_nuc_lights` / `g_has_towing_lights` before relay activation

## Related Documentation
- [HARDWARE.md](../HARDWARE.md) - GPIO pinout and wiring
- [README.md](../README.md) - Quick start and features
- [TOWING_STATE.md](TOWING_STATE.md) - Towing light specific details
