# Web-Based Fallback UI - Implementation Plan

**Project**: boat.helm-ecu
**Feature**: Custom Web Control Interface  
**Date**: February 9, 2026  
**Status**: ✅ Phase 1 Complete (Backend API)

---

## Executive Summary

Implement a web-based control interface running directly on the ESP32 ECU, accessible via browser from any device on the boat's WiFi network. This fallback UI provides full ECU control when the SignalK server is unavailable, using the existing SensESP AsyncWebServer infrastructure.

**Key Benefits:**
- Universal browser access (no app installation)
- Parallel operation with SensESP configuration UI
- Minimal resource overhead (~50KB flash, ~30KB RAM)
- Scalable pattern for other boat ECUs

---

## Architecture Overview

### System Context
```
Boat WiFi Network
├── SignalK Server (Primary Control Path)
│   └── Node-RED Dashboard → SignalK PUT → ECU
│
├── ESP32 boat.helm-ecu
│   ├── Port 80: AsyncWebServer (SensESP)
│   │   ├── /                    → SensESP Status Page
│   │   ├── /config              → SensESP Configuration
│   │   ├── /control             → SensESP Control Menu
│   │   └── /lights              → Custom Control UI (NEW)
│   │       └── /api/*           → REST API Endpoints (NEW)
│   └── SignalK WebSocket Client
│
└── Browsers (Fallback Control Path)
    └── Any device → http://boat-helm-ecu.local/lights
```

### Technology Stack
- **Backend**: SensESP v3.2.2 (AsyncWebServer, ESPAsyncWebServer library)
- **Frontend**: Vanilla HTML5 + CSS3 + JavaScript (no frameworks, minimal size)
- **API**: RESTful JSON endpoints
- **Storage**: SPIFFS for static files (HTML/CSS/JS)
- **Integration**: Direct calls to existing `NavigationLightsECU` facade

---

## API Design

### REST Endpoints

#### Base Path: `/api/`

#### Status Endpoints (GET)
```
GET /api/status
Response: {
  "condition": "day|hours_of_darkness|restricted_visibility",
  "boatState": "moored|underway_making_way|underway_no_way|anchorage|nuc_making_way|nuc_no_way",
  "periodicMuted": true|false,
  "periodicCountdown": 120,  // seconds
  "lights": {
    "masthead": true|false,
    "portSidelight": true|false,
    "starboardSidelight": true|false,
    "sternlight": true|false,
    "allroundWhite": true|false,
    "allroundRedUpper": true|false,
    "allroundRedLower": true|false
  },
  "horn": {
    "active": true|false
  },
  "signalkConnected": true|false,
  "uptime": 12345,  // seconds
  "freeHeap": 98765  // bytes
}

GET /api/health
Response: {
  "healthy": true,
  "uptime": 12345,
  "freeHeap": 98765,
  "wifiRSSI": -45,
  "signalkConnected": true|false
}
```

#### Control Endpoints (POST)

```
POST /api/condition
Body: { "value": "day|hours_of_darkness|restricted_visibility" }
Response: { "success": true, "condition": "hours_of_darkness" }

POST /api/state
Body: { "value": "moored|underway_making_way|underway_no_way|anchorage|nuc_making_way|nuc_no_way" }
Response: { "success": true, "boatState": "underway_making_way" }

POST /api/mute
Body: { "muted": true|false }
Response: { "success": true, "periodicMuted": true }

POST /api/signal
Body: { "signal": "turn_starboard|turn_port|astern_propulsion|danger_confusion|pay_attention|overtake_starboard|overtake_port|agreement_overtaken|sos" }
Response: { "success": true, "triggered": "turn_starboard" }

**SOS Behavior**:
- Horn plays ●●● ▬▬ ▬▬ ▬▬ ●●●
- In hours of darkness or restricted visibility, masthead + anchor light flash in sync with horn
- Guarded control remains open after sending so SOS can be re-sent until the user closes the cover

POST /api/emergency
Body: {}
Response: { "success": true, "stopped": true }
```

#### Error Response Format
```
{
  "success": false,
  "error": "Invalid condition value",
  "code": 400
}
```

---

## Frontend Design

### UI Layout (Mobile-First Responsive)

```
┌─────────────────────────────────────┐
│  Navigation Lights Control         │ <- Header
│  ◉ SignalK Unavailable (Fallback) │ <- Status Banner (conditional)
├─────────────────────────────────────┤
│  Condition                          │
│  ┌───────┐ ┌───────┐ ┌───────┐    │
│  │ Day   │ │ Dark  │ │ Fog   │    │ <- Button Grid
│  └───────┘ └───────┘ └───────┘    │
│                                     │
│  Boat State                         │
│  ┌─────────┐ ┌─────────┐          │
│  │ Moored  │ │ Underway│          │
│  ├─────────┤ ├─────────┤          │
│  │Anchorage│ │   NUC   │          │
│  └─────────┘ └─────────┘          │
│                                     │
│  Sound Signals                      │
│  Periodic: [Muted ✓]               │ <- Toggle
│  Next signal: 120s [▓▓▓░░░░░] 0%  │ <- Countdown + Progress Bar
│                                     │
│  Ad-Hoc Signals                     │
│  ┌────┐┌────┐┌────┐┌────┐         │
│  │ ●  ││ ●● ││●●● ││●●●●│         │ <- Symbol Buttons
│  └────┘└────┘└────┘└────┘         │
│  ┌────┐┌────┐┌────┐┌────┐         │
│  │ ▬▬ ││▬▬●│││▬▬●●││▬●▬●│         │
│  └────┘└────┘└────┘└────┘         │
│  SOS Distress (guarded hold)       │
│                                     │
│  Active Lights: MST PSL SSL ST     │ <- Status Chips
│                                     │
│  [🛑 Emergency Stop]               │ <- Safety Button
├─────────────────────────────────────┤
│  ⚙️ Settings | 🔄 Refresh          │ <- Footer Actions
└─────────────────────────────────────┘
```

### Visual Design Principles

#### Color Scheme (Maritime Theme)
```css
:root {
  /* Base colors */
  --color-water: #1a3a52;        /* Dark blue-grey */
  --color-deck: #2d4a5e;         /* Lighter blue-grey */
  --color-navigation-red: #d32f2f;   /* Port side */
  --color-navigation-green: #388e3c; /* Starboard side */
  --color-warning: #ffa726;
  --color-danger: #e53935;
  --color-success: #66bb6a;
  
  /* UI elements */
  --color-primary: #1976d2;
  --color-active: #4caf50;
  --color-inactive: #757575;
  --color-text: #ffffff;
  --color-text-dim: #b0bec5;
  --color-background: #263238;
  --color-card: #37474f;
  
  /* Spacing */
  --spacing-xs: 4px;
  --spacing-sm: 8px;
  --spacing-md: 16px;
  --spacing-lg: 24px;
  
  /* Typography */
  --font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  --font-size-sm: 14px;
  --font-size-md: 16px;
  --font-size-lg: 20px;
  
  /* Touch targets (minimum 44x44px) */
  --button-min-height: 48px;
}
```

#### Responsive Breakpoints
```css
/* Mobile-first approach */
@media (min-width: 768px) { /* Tablet */ }
@media (min-width: 1024px) { /* Desktop */ }
```

#### Button States
- **Default**: Grey background, white text
- **Active/Selected**: Blue background, bold
- **Disabled**: Grey with opacity 0.5
- **Hover**: Slightly lighter background
- **Pressed**: Scale 0.95 transform

#### Status Indicators
- **Light ON**: Green chip with light name
- **Light OFF**: Grey chip or hidden
- **Horn Active**: Red pulsing indicator
- **SignalK Connected**: Green dot in header
- **SignalK Disconnected**: Yellow banner with warning
- **SOS Distress**: Guarded control (lift cover + 3-second hold) with clear warning text

---

## Implementation Phases

### Phase 1: Backend API (Priority: High)
**Estimated Time**: 4-6 hours

#### Tasks
1. **Create API Handler File** (`src/web_api.cpp/.h`)
   - Include AsyncWebServer, ArduinoJson
   - Create handler functions for each endpoint
   - Implement request parsing and validation
   - Implement JSON response generation

2. **Integrate with NavigationLightsECU**
   - Pass ECU pointer to API handlers
   - Add mutex for thread-safe access (if needed)
   - Implement error handling

3. **Register Routes in main.cpp**
   ```cpp
   // Get SensESP's AsyncWebServer
   auto* server = sensesp_app->get_http_server();
   
   // Register API endpoints
   setupWebAPI(server, ecu);
   ```

4. **Test with curl/Postman**
   - Validate all GET endpoints return correct JSON
   - Test POST endpoints with valid/invalid data
   - Verify state changes propagate to hardware

#### File Structure
```
src/
├── web_api.h              # API endpoint declarations
├── web_api.cpp            # API implementation
└── main.cpp               # Route registration
```

#### Code Template
```cpp
// web_api.h
#ifndef WEB_API_H
#define WEB_API_H

#include <ESPAsyncWebServer.h>
#include "NavigationLightsECU.h"

void setupWebAPI(AsyncWebServer* server, NavigationLightsECU* ecu);

#endif

// web_api.cpp
#include "web_api.h"
#include <ArduinoJson.h>

// Global pointer (consider thread safety)
static NavigationLightsECU* g_ecu = nullptr;

void handleGetStatus(AsyncWebServerRequest* request) {
    StaticJsonDocument<512> doc;
    
    // Populate from g_ecu
    doc["condition"] = conditionToString(g_ecu->getCondition());
    doc["boatState"] = boatStateToString(g_ecu->getBoatState());
    // ... etc
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handlePostCondition(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
        return;
    }
    
    const char* value = doc["value"];
    Condition condition = stringToCondition(value);
    
    if (condition == Condition::DAY || /* valid */) {
        g_ecu->setCondition(condition);
        request->send(200, "application/json", "{\"success\":true}");
    } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid condition\"}");
    }
}

void setupWebAPI(AsyncWebServer* server, NavigationLightsECU* ecu) {
    g_ecu = ecu;
    
    // GET endpoints
    server->on("/api/status", HTTP_GET, handleGetStatus);
    server->on("/api/health", HTTP_GET, handleGetHealth);
    
    // POST endpoints (use onRequestBody for JSON body parsing)
    server->on("/api/condition", HTTP_POST, 
        [](AsyncWebServerRequest* request) { /* placeholder */ },
        nullptr,
        handlePostCondition
    );
    
    // ... register all endpoints
}
```

---

### Phase 2: Static Frontend Files (Priority: High)
**Estimated Time**: 6-8 hours

#### Tasks
1. **Create HTML Structure** (`data/lights/index.html`)
   - Semantic HTML5 markup
   - Mobile-first viewport settings
   - Inline critical CSS (above-the-fold)
   - Defer non-critical JavaScript

2. **CSS Styling** (`data/lights/styles.css`)
   - Maritime color scheme
   - Responsive grid layout
   - Touch-friendly button sizing (min 48px)
   - Loading states and animations

3. **JavaScript Logic** (`data/lights/app.js`)
   - API communication (fetch)
   - State management (simple object)
   - UI updates (DOM manipulation)
   - Auto-refresh (polling every 2 seconds)
   - Error handling and retry logic

4. **SPIFFS Upload**
   - Configure `platformio.ini` with data directory
   - Use PlatformIO "Upload File System image"

#### File Structure
```
data/
└── lights/
    ├── index.html         # Main UI page
    ├── styles.css         # Stylesheet
    └── app.js             # JavaScript logic
```

#### HTML Template (Minimal)
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Navigation Lights Control</title>
    <link rel="stylesheet" href="styles.css">
</head>
<body>
    <header>
        <h1>boat.helm-ecu</h1>
        <div id="connection-status" class="status-badge"></div>
    </header>

    <main>
        <!-- Condition Section -->
        <section class="control-section">
            <h2>Condition</h2>
            <div class="button-grid">
                <button class="btn-condition" data-value="day">Day</button>
                <button class="btn-condition" data-value="hours_of_darkness">Darkness</button>
                <button class="btn-condition" data-value="restricted_visibility">Fog</button>
            </div>
        </section>

        <!-- Boat State Section -->
        <section class="control-section">
            <h2>Boat State</h2>
            <div class="button-grid">
                <button class="btn-state" data-value="moored">Moored</button>
                <button class="btn-state" data-value="underway_making_way">Underway (Way)</button>
                <button class="btn-state" data-value="underway_no_way">Underway (Stopped)</button>
                <button class="btn-state" data-value="anchorage">Anchorage</button>
                <button class="btn-state" data-value="nuc_making_way">NUC (Way)</button>
                <button class="btn-state" data-value="nuc_no_way">NUC (Stopped)</button>
            </div>
        </section>

        <!-- Sound Signals Section -->
        <section class="control-section">
            <h2>Sound Signals</h2>
            <div class="mute-control">
                <button id="btn-mute" class="btn-toggle">Mute/Unmute</button>
                <div class="countdown" id="countdown">--</div>
            </div>
        </section>

        <!-- Ad-Hoc Signals Section -->
        <section class="control-section">
            <h2>Ad-Hoc Signals</h2>
            <div class="signal-grid">
                <button class="btn-signal" data-signal="turn_starboard" title="Turn to Starboard">●</button>
                <button class="btn-signal" data-signal="turn_port" title="Turn to Port">●●</button>
                <button class="btn-signal" data-signal="astern_propulsion" title="Astern Propulsion">●●●</button>
                <button class="btn-signal" data-signal="danger_confusion" title="Danger/Confusion">●●●●●</button>
                <button class="btn-signal" data-signal="pay_attention" title="Pay Attention">▬▬</button>
                <button class="btn-signal" data-signal="overtake_starboard" title="Overtake Starboard">▬▬●</button>
                <button class="btn-signal" data-signal="overtake_port" title="Overtake Port">▬▬●●</button>
                <button class="btn-signal" data-signal="agreement_overtaken" title="Agreement">▬●▬●</button>
            </div>
        </section>

        <!-- Status Display -->
        <section class="control-section">
            <h2>Active Lights</h2>
            <div id="lights-status" class="status-chips"></div>
        </section>

        <!-- Emergency Stop -->
        <section class="control-section">
            <button id="btn-emergency" class="btn-emergency">🛑 Emergency Stop</button>
        </section>
    </main>

    <footer>
        <a href="/config">⚙️ Settings</a>
        <button id="btn-refresh">🔄 Refresh</button>
    </footer>

    <script src="app.js"></script>
</body>
</html>
```

#### JavaScript Template (Core Logic)
```javascript
// app.js
const API_BASE = '/api';
let currentState = null;
let autoRefreshInterval = null;

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    setupEventListeners();
    startAutoRefresh();
    fetchStatus();
});

// Fetch current status
async function fetchStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        if (!response.ok) throw new Error('Failed to fetch status');
        
        currentState = await response.json();
        updateUI(currentState);
        updateConnectionStatus(true);
    } catch (error) {
        console.error('Status fetch error:', error);
        updateConnectionStatus(false);
    }
}

// Update UI based on state
function updateUI(state) {
    // Update condition buttons
    document.querySelectorAll('.btn-condition').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.value === state.condition);
    });
    
    // Update state buttons
    document.querySelectorAll('.btn-state').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.value === state.boatState);
    });
    
    // Update mute button
    document.getElementById('btn-mute').textContent = state.periodicMuted ? 'Unmute' : 'Mute';
    document.getElementById('btn-mute').classList.toggle('muted', state.periodicMuted);
    
    // Update countdown
    document.getElementById('countdown').textContent = `${state.periodicCountdown}s`;
    
    // Update lights status
    updateLightsDisplay(state.lights);
    
    // Update SignalK connection indicator
    const banner = document.getElementById('connection-status');
    if (!state.signalkConnected) {
        banner.textContent = '⚠️ Fallback Mode (SignalK Unavailable)';
        banner.classList.add('warning');
    } else {
        banner.textContent = '';
        banner.classList.remove('warning');
    }
}

// Update lights display
function updateLightsDisplay(lights) {
    const container = document.getElementById('lights-status');
    container.innerHTML = '';
    
    const lightNames = {
        masthead: 'MST',
        portSidelight: 'PSL',
        starboardSidelight: 'SSL',
        sternlight: 'ST',
        allroundWhite: 'AWH',
        allroundRedUpper: 'ARU',
        allroundRedLower: 'ARL'
    };
    
    for (const [key, label] of Object.entries(lightNames)) {
        if (lights[key]) {
            const chip = document.createElement('span');
            chip.className = 'chip chip-active';
            chip.textContent = label;
            container.appendChild(chip);
        }
    }
    
    if (container.children.length === 0) {
        container.textContent = 'No lights active';
    }
}

// Send command to API
async function sendCommand(endpoint, data = {}) {
    try {
        const response = await fetch(`${API_BASE}/${endpoint}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            const error = await response.json();
            throw new Error(error.error || 'Command failed');
        }
        
        // Refresh status after command
        setTimeout(fetchStatus, 200);
        return true;
    } catch (error) {
        console.error('Command error:', error);
        alert(`Error: ${error.message}`);
        return false;
    }
}

// Event listeners
function setupEventListeners() {
    // Condition buttons
    document.querySelectorAll('.btn-condition').forEach(btn => {
        btn.addEventListener('click', () => {
            sendCommand('condition', { value: btn.dataset.value });
        });
    });
    
    // State buttons
    document.querySelectorAll('.btn-state').forEach(btn => {
        btn.addEventListener('click', () => {
            sendCommand('state', { value: btn.dataset.value });
        });
    });
    
    // Mute toggle
    document.getElementById('btn-mute').addEventListener('click', () => {
        const muted = !currentState.periodicMuted;
        sendCommand('mute', { muted });
    });
    
    // Ad-hoc signals
    document.querySelectorAll('.btn-signal').forEach(btn => {
        btn.addEventListener('click', () => {
            sendCommand('signal', { signal: btn.dataset.signal });
        });
    });
    
    // Emergency stop
    document.getElementById('btn-emergency').addEventListener('click', () => {
        if (confirm('EMERGENCY STOP: Disable all lights and sound?')) {
            sendCommand('emergency');
        }
    });
    
    // Manual refresh
    document.getElementById('btn-refresh').addEventListener('click', fetchStatus);
}

// Auto-refresh
function startAutoRefresh() {
    autoRefreshInterval = setInterval(fetchStatus, 2000); // Every 2 seconds
}

function updateConnectionStatus(connected) {
    const badge = document.getElementById('connection-status');
    if (!connected) {
        badge.textContent = '⚠️ Connection Lost';
        badge.classList.add('error');
    } else {
        badge.classList.remove('error');
    }
}
```

---

### Phase 3: Integration & Testing (Priority: High)
**Estimated Time**: 2-3 hours

#### Tasks
1. **SPIFFS Configuration**
   ```ini
   # platformio.ini
   [env:az-delivery-devkit-v4]
   board_build.filesystem = littlefs
   board_build.partitions = min_spiffs.csv
   ```

2. **Upload Static Files**
   ```bash
   pio run --target uploadfs
   ```

3. **Register Static File Handler**
   ```cpp
   // main.cpp
   server->serveStatic("/lights", SPIFFS, "/lights/").setDefaultFile("index.html");
   ```

4. **Testing Checklist**
   - [ ] API endpoints respond correctly
   - [ ] HTML page loads from ESP32
   - [ ] CSS styling renders correctly on mobile
   - [ ] JavaScript can fetch status
   - [ ] Commands trigger hardware changes
   - [ ] Auto-refresh updates UI
   - [ ] Error handling shows appropriate messages
   - [ ] Works in Chrome, Firefox, Safari, Edge
   - [ ] Works on iOS and Android browsers

---

### Phase 4: Polish & Optimization (Priority: Medium)
**Estimated Time**: 3-4 hours

#### Tasks
1. **Performance Optimization**
   - Minify HTML/CSS/JS before SPIFFS upload
   - Enable GZIP compression on AsyncWebServer
   - Optimize JSON response sizes
   - Reduce polling frequency when idle

2. **UX Enhancements**
   - Add loading spinners for async operations
   - Smooth transitions between states
   - Toast notifications for actions
   - Haptic feedback (vibration) on mobile
   - Offline detection and retry logic

3. **Accessibility**
   - ARIA labels for screen readers
   - Keyboard navigation support
   - High contrast mode support
   - Focus indicators

4. **Security (Optional)**
   - HTTP Basic Auth for control endpoints
   - CSRF token for POST requests
   - Rate limiting to prevent abuse

---

## Resource Budget

### Memory Requirements
```
Flash (SPIFFS):
- index.html:  ~3 KB
- styles.css:  ~5 KB
- app.js:      ~8 KB
- Total:       ~16 KB (minified: ~10 KB)

Flash (Code):
- web_api.cpp: ~15 KB
- ArduinoJson:  ~20 KB (already included for SignalK)
- Total new:    ~15 KB

RAM (Runtime):
- JSON buffers:        ~2 KB (stack allocated)
- AsyncWebServer req:  ~4 KB per request
- SPIFFS file handle:  ~1 KB
- Total overhead:      ~7 KB
```

### Current vs Post-Implementation
```
Current Build:
- Flash: 71.8% (1411 KB / 1966 KB) → 555 KB free
- RAM:   9.4% (50 KB / 532 KB) → 482 KB free

Estimated After Web UI:
- Flash: 73.3% (+1.5%) → ~525 KB free (safe)
- RAM:   10.8% (+1.4%) → ~475 KB free (safe)
```

**Conclusion**: Comfortably within budget ✅

---

## Testing Strategy

### Unit Tests (API Layer)
```cpp
// test/test_web_api/test_web_api.cpp
#include <unity.h>
#include "../../src/web_api.h"

void test_string_to_condition_valid() {
    TEST_ASSERT_EQUAL(Condition::DAY, stringToCondition("day"));
    TEST_ASSERT_EQUAL(Condition::HOURS_OF_DARKNESS, stringToCondition("hours_of_darkness"));
}

void test_string_to_condition_invalid() {
    TEST_ASSERT_EQUAL(Condition::DAY, stringToCondition("invalid")); // Default
}

void test_json_status_generation() {
    // Mock ECU with known state
    // Generate JSON
    // Parse and verify fields
}
```

### Integration Tests (ESP32 Hardware)
1. **API Endpoint Validation**
   - Hit each endpoint with curl
   - Verify JSON responses
   - Verify hardware state changes

2. **Frontend Testing**
   - Load UI in browser
   - Test each button
   - Verify state updates propagate

3. **Concurrent Access**
   - SignalK + Web UI simultaneously
   - Multiple browser tabs
   - Verify no race conditions

### Browser Compatibility Testing
- [ ] Chrome 90+ (Desktop)
- [ ] Firefox 88+ (Desktop)
- [ ] Safari 14+ (Desktop/iOS)
- [ ] Edge 90+ (Desktop)
- [ ] Chrome Mobile (Android)
- [ ] Safari Mobile (iOS)

---

## Deployment Checklist

### Pre-Deployment
- [ ] Code review completed
- [ ] All tests passing
- [ ] Memory usage within limits
- [ ] Documentation updated

### Deployment Steps
1. Build firmware with web UI code
2. Upload SPIFFS with static files
3. Flash firmware to ESP32
4. Verify system boots correctly
5. Test API endpoints
6. Test web UI access
7. Validate parallel SignalK operation

### Post-Deployment Validation
- [ ] Web UI accessible at `http://boat-helm-ecu.local/lights`
- [ ] All control functions working
- [ ] Auto-refresh functioning
- [ ] SignalK integration unaffected
- [ ] SensESP configuration UI still works
- [ ] Emergency AP mode works (`http://192.168.4.1/lights`)

---

## Future Enhancements (Post-MVP)

### Phase 5: Advanced Features (Optional)
1. **Real-time Updates via WebSocket**
   - Replace polling with WebSocket push
   - Reduce network traffic
   - Instant UI updates

2. **Historical Data Visualization**
   - Log state changes to SPIFFS
   - Display usage charts
   - Export logs for analysis

3. **Multi-Language Support**
   - Detect browser language
   - Translate UI labels
   - Store translations in JSON

4. **Progressive Web App (PWA)**
   - Service worker for offline capability
   - Add to home screen support
   - App-like experience

5. **Advanced Diagnostics**
   - GPIO voltage monitoring
   - Relay cycle counters
   - Firmware update UI
   - WiFi signal strength graph

### Phase 6: Scalability (Multi-ECU)
1. **Shared UI Components Library**
   - Extract reusable CSS/JS
   - Create component documentation
   - Version control for components

2. **ECU Discovery**
   - mDNS service browser
   - Auto-discover all boat ECUs
   - Unified dashboard

3. **Central Configuration**
   - Sync settings across ECUs
   - Backup/restore configurations
   - Fleet management

---

## Risk Mitigation

### Identified Risks

**Risk 1: Memory Overflow**
- *Probability*: Low
- *Impact*: High (crashes, reboots)
- *Mitigation*: Use stack-allocated JSON buffers, monitor heap usage, stress test with multiple concurrent requests

**Risk 2: AsyncWebServer Conflicts**
- *Probability*: Medium
- *Impact*: Medium (routes overlap, 404 errors)
- *Mitigation*: Use unique base path `/lights`, test all SensESP routes remain functional, avoid conflicting route patterns

**Risk 3: SPIFFS Upload Failure**
- *Probability*: Medium
- *Impact*: Medium (UI not accessible)
- *Mitigation*: Verify SPIFFS upload before firmware flash, implement fallback error page, document recovery procedure

**Risk 4: Browser Compatibility Issues**
- *Probability*: Low
- *Impact*: Low (UI doesn't work on some browsers)
- *Mitigation*: Use standard ES6 features only (or transpile), test on target devices, provide graceful degradation

**Risk 5: Race Conditions (Multi-UI)**
- *Probability*: Low
- *Impact*: High (inconsistent state, relay flickering)
- *Mitigation*: Use mutex in NavigationLightsECU, implement request throttling, add command queueing if needed

---

## Success Criteria

### Must Have (MVP)
- ✅ Web UI accessible from boat WiFi at `/lights`
- ✅ All condition/state controls functional
- ✅ Mute/unmute working
- ✅ Ad-hoc signals triggering correctly
- ✅ Real-time status display (2s refresh)
- ✅ Emergency stop working
- ✅ Mobile responsive design
- ✅ No regression in SignalK operation

### Should Have
- ✅ Connection status indicator
- ✅ Active lights display
- ✅ Countdown timer with visual progress bar
- ✅ Error messages for failed commands
- ✅ Works in emergency AP mode

### Nice to Have
- ⭕ WebSocket real-time updates
- ⭕ PWA offline capability
- ⭕ Data usage graphs
- ⭕ Multi-language support

---

## Timeline Estimate

**Total Estimated Time**: 15-21 hours

| Phase | Time | Dependencies |
|-------|------|--------------|
| Phase 1: Backend API | 4-6h | None |
| Phase 2: Frontend | 6-8h | None (parallel) |
| Phase 3: Integration | 2-3h | Phase 1 + 2 |
| Phase 4: Polish | 3-4h | Phase 3 |

**Development Schedule (Suggested):**
- Week 1: Backend API + Unit tests
- Week 2: Frontend development + UI testing
- Week 3: Integration + Browser testing
- Week 4: Polish + Deployment

---

## References

### Documentation
- [AsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [SensESP Documentation](https://signalk.org/SensESP/)
- [ESP32 SPIFFS Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html)

### Existing Code References
- [src/NavigationLightsECU.h](src/NavigationLightsECU.h) - Facade API
- [src/signalk_integration.cpp](src/signalk_integration.cpp) - Conversion functions
- [src/main.cpp](src/main.cpp) - Server initialization

### Similar Projects (Inspiration)
- ESPHome Web Server Component
- Tasmota Web UI
- WLED Web Interface

---

## Appendix

### A. platformio.ini Changes
```ini
[env:az-delivery-devkit-v4]
platform = espressif32
board = az-delivery-devkit-v4
framework = arduino
lib_deps = 
    signalk/SensESP@^3.2.0
    bblanchon/ArduinoJson@^6.21.3  # Already included by SensESP
build_flags = 
    -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_INFO
    -D ACTIVE_HIGH_RELAYS  # Remove for production
board_build.partitions = min_spiffs.csv
board_build.filesystem = littlefs  # Or spiffs
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
```

### B. SPIFFS File Structure
```
data/
└── lights/
    ├── index.html       # Main HTML page
    ├── styles.css       # Stylesheet
    ├── app.js           # JavaScript logic
    └── favicon.ico      # Optional: ECU icon
```

### C. API Response Examples

**Success Response:**
```json
{
  "success": true,
  "condition": "hours_of_darkness"
}
```

**Error Response:**
```json
{
  "success": false,
  "error": "Invalid condition value: 'twilight'",
  "code": 400
}
```

**Status Response:**
```json
{
  "condition": "hours_of_darkness",
  "boatState": "underway_making_way",
  "periodicMuted": false,
  "periodicCountdown": 75,
  "lights": {
    "masthead": true,
    "portSidelight": true,
    "starboardSidelight": true,
    "sternlight": true,
    "allroundWhite": false,
    "allroundRedUpper": false,
    "allroundRedLower": false
  },
  "horn": {
    "active": false
  },
  "signalkConnected": false,
  "uptime": 3625,
  "freeHeap": 456789
}
```

---

**Document Version**: 1.0  
**Last Updated**: February 9, 2026  
**Next Review**: After MVP completion
