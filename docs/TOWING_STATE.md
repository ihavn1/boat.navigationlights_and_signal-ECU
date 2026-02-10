# Towing State Implementation (COLREGs Rule 24)

## Overview

The navigation lights ECU supports the **TOWING** boat state as defined in COLREGs Rule 24 for vessels engaged in towing operations. This document describes the implementation, requirements, and testing approach.

## COLREGs Requirements

### Rule 24: Lights for Towing and Pushing
A power-driven vessel when towing shall exhibit:
- **Normal navigation lights** (masthead, sidelights, sternlight) as if making way
- **Plus**: A yellow towing light above the sternlight with same arc (135°, 67.5° from right aft on each side)

### Light Configuration by Condition

| Condition | Lights Required | Sound Signal |
|-----------|----------------|--------------|
| **Daylight** | None | None |
| **Hours of Darkness** | Masthead + Port Sidelight + Starboard Sidelight + Sternlight + **Yellow Towing Light** | None |
| **Restricted Visibility** | Same as darkness | 1 prolonged + 2 short blasts every 2 minutes (Rule 35c) |

### Key Implementation Details

1. **Additive Nature**: The yellow towing light is **added** to normal navigation lights, not a replacement
2. **Positioning**: Yellow towing light must be positioned **above** the sternlight
3. **Arc Coverage**: 135° arc (same as sternlight), showing 67.5° from aft on each side
4. **Making Way Status**: Towing vessels are always considered "making way" for light configuration purposes

## Hardware Implementation

### GPIO Assignment
- **GPIO 32**: Yellow towing light relay control (9th relay output)
- **Relay Module**: Requires 9 or 16-channel relay module (upgraded from 8-channel)

### Wiring
- Active-LOW relay control for safety (relays off on boot/crash)
- Yellow towing light relay connected to GPIO 32, IN8 on relay module
- Follow wiring diagram in [HARDWARE.md](../HARDWARE.md)

## Software Implementation

### State Machine (`src/state_machine.cpp`)

#### Enum Definition
```cpp
enum class BoatState : uint8_t {
    MOORED = 0,
    UNDERWAY_NO_WAY = 1,
    UNDERWAY_MAKING_WAY = 2,
    ANCHORAGE = 3,
    NUC_NO_WAY = 4,
    NUC_MAKING_WAY = 5,
    TOWING = 6          // New state for towing operations
};
```

#### Light Configuration
```cpp
struct LightConfiguration {
    // ... existing fields ...
    bool yellow_towing_light;  // New field for Rule 24
};
```

#### Logic (Rule 24 + Rule 35c)
```cpp
case BoatState::TOWING:
    // Rule 24: Vessel towing shows normal underway lights + yellow towing light
    config.masthead_light = true;
    config.port_sidelight = true;
    config.starboard_sidelight = true;
    config.sternlight = true;
    config.yellow_towing_light = true;  // Additional light above sternlight
    break;
```

For restricted visibility, sound signal is added:
```cpp
case BoatState::TOWING:
    // Rule 35(c): Towing vessel = 1 prolonged + 2 short blasts every 2min
    return SoundSignalPattern::PROLONGED_SHORT_SHORT_2MIN;
```

### SignalK Integration (`src/signalk_integration.cpp`)

#### String Conversion
- **SignalK string**: `"towing"` (lowercase, no spaces)
- **Enum**: `BoatState::TOWING`
- Bidirectional conversion via `sk_boatStateToString()` and `sk_stringToBoatState()`

#### SignalK Paths

**Input Path** (existing, now accepts "towing"):
```
electrical.switches.navigationLights.command.boatState
Value: "towing"
```

**Output Path** (new):
```
electrical.switches.navigationLights.lights.yellowTowingLight
Value: true/false
```

Total SignalK paths after towing state:
- **5 input paths** (unchanged count, "towing" added to boatState validation)
- **14 output paths** (+1 for yellowTowingLight)

### Web API (`src/web_api.cpp`)

#### Endpoint Updates

**GET /api/status** - Added yellow towing light status:
```json
{
  "lights": {
    "yellowTowingLight": true
  }
}
```

**POST /api/state** - Accepts "towing" as valid boat state:
```json
{
  "value": "towing"
}
```

### Web UI (`data/lights.html`, `lights.css`, `lights.js`)

#### UI Elements Added
1. **Towing Button**: `<button>🚢 Towing</button>` in boat state controls
2. **Yellow Light Indicator**: Displays when towing light active with amber glow effect
3. **State Label**: "Towing" label in status display

#### Visual Design
- **Color**: Amber (#ffbb33) with glow animation when active
- **Icon**: Yellow/amber circular indicator matching other light displays
- **Button**: Ship emoji (🚢) for easy recognition

## Testing

### Test Coverage (124 tests total, +5 for towing)

#### State Machine Tests (`test/test_state_machine/`)
1. **`test_day_towing_no_lights`**: Verifies no lights in daylight
2. **`test_darkness_towing_navigation_plus_yellow`**: Verifies all 5 lights active (masthead + sidelights + stern + yellow) in darkness
3. **`test_restricted_visibility_towing_lights_and_signal`**: Verifies lights + sound signal (▬▬ ●●) in fog

#### SignalK Integration Tests (`test/test_signalk_integration/`)
4. **`test_boat_state_to_string_towing`**: Enum `TOWING` → string `"towing"`
5. **`test_string_to_boat_state_towing`**: String `"towing"` → enum `TOWING`
6. **Updated `test_boat_state_roundtrip_conversion`**: Includes towing state validation

### Running Tests
```bash
# All native tests (fast, no hardware needed)
pio test -e native

# ESP32 embedded tests (requires hardware)
pio test -e esp32test
```

## Deployment Checklist

- [x] State machine logic updated (Rule 24 + Rule 35c)
- [x] Hardware layer expanded to 9 channels
- [x] GPIO 32 assigned for yellow towing light
- [x] SignalK integration updated ("towing" string + yellowTowingLight output)
- [x] Web API endpoints updated (GET /api/status, POST /api/state)
- [x] Web UI updated (towing button + yellow indicator)
- [x] All 124 unit tests passing
- [x] Documentation created
- [ ] Hardware tested with 9-channel relay module
- [ ] ESP32 embedded tests re-validated (14 tests pending)
- [ ] Maritime field testing (actual towing conditions)

## Hardware Upgrade Required

**Before deploying to production**, ensure:
1. **9-channel relay module** installed (or 16-channel, or dual 8-channel)
2. GPIO 32 wired to 9th relay (IN8 on module)
3. Yellow towing light connected to relay output
4. All wiring follows active-LOW safety pattern

## References

- **COLREGs Rule 24**: Towing and pushing (full text in Project Proposal)
- **COLREGs Rule 35(c)**: Sound signals for vessels towing in restricted visibility
- **Project Proposal**: [Project Proposal Navigation Lights and Signaling ECU.md](../Project%20Proposal%20Navigation%20Lights%20and%20Signaling%20ECU.md)
- **Hardware Details**: [HARDWARE.md](../HARDWARE.md)
- **SignalK Specification**: https://signalk.org/specification/latest/doc/

## Version History

- **2026-02-10**: Towing state added with full integration (Rule 24 + Rule 35c)
  - Initial implementation showed only yellow + stern (incorrect)
  - Corrected to show all navigation lights + yellow (per COLREGs table)
  - 124/124 tests passing after correction
