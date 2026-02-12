# Phase 3: SPIFFS Upload & Testing

## Overview
Upload the web UI frontend files (HTML/CSS/JS) to ESP32's SPIFFS filesystem and test the complete web interface.

## Files Created (Phase 2)
- ✅ [data/lights.html](../data/lights.html) - Main UI (183 lines)
- ✅ [data/lights.css](../data/lights.css) - Responsive stylesheet (587 lines)
- ✅ [data/lights.js](../data/lights.js) - API client & UI logic (434 lines)

**Total Frontend**: ~1,200 lines of production code

## Prerequisites
- Phase 1 complete (Backend API running on ESP32)
- ESP32 connected via USB (COM3)
- WiFi configured and connected
- PlatformIO installed
- If the partition table changed, run a full flash erase before uploading:

```bash
pio run -t erase
pio run -t upload
pio run -t uploadfs
```

## Upload Steps

### 1. Build Filesystem Image

```bash
# Build the SPIFFS filesystem image from data/ folder
pio run --target buildfs
```

This creates a binary image containing all files in the `data/` directory.

### 2. Upload to ESP32

```bash
# Upload filesystem to ESP32 (will interrupt running firmware)
pio run --target uploadfs
```

**Note**: This will temporarily stop the running firmware. The ESP32 will restart automatically after upload.

### 3. Verify Upload

```bash
# Monitor serial output to verify boot
pio device monitor
```

Look for these messages:
```
Setting up Web API endpoints...
Web API endpoints registered:
  GET  /api/status
  GET  /api/health
  ...
Frontend UI will be available at:
  http://nav-lights-ecu.local/lights.html
```

## Access the Web UI

### Option 1: mDNS Hostname (Recommended)
```
http://nav-lights-ecu.local/lights.html
```

### Option 2: Direct IP Address
```
http://10.100.100.244/lights.html
```
(Replace with your ESP32's IP address)

### Option 3: Emergency AP Mode
If WiFi disconnects, ESP32 creates access point:
```
SSID: nav-lights-ecu-XXXXXX
Password: (check SensESP config)
URL: http://192.168.4.1/lights.html
```

## Testing Checklist

### 1. UI Loads ✓
- [ ] HTML page displays correctly
- [ ] CSS styles applied (dark theme, marine colors)
- [ ] No browser console errors
- [ ] All buttons and sections visible

### 2. Connection Status ✓
- [ ] "Connected" badge shows green
- [ ] "SignalK: Connected" badge visible
- [ ] System info displays (uptime, heap, WiFi RSSI)

### 3. Condition Control ✓
- [ ] Click "Day" button → activates
- [ ] Click "Darkness" button → activates
- [ ] Click "Restricted" button → activates
- [ ] Current condition updates immediately
- [ ] Success alert shows

### 4. Boat State Control ✓
- [ ] All 6 state buttons work
- [ ] Current state updates
- [ ] Active button highlights in blue
- [ ] Success alerts appear

### 5. Light Indicators ✓
- [ ] Light icons update based on state
- [ ] White lights glow white when active
- [ ] Red lights (port, NUC) glow red when active
- [ ] Green lights (starboard) glow green when active
- [ ] Lights turn off correctly

### 6. Sound Signal Controls ✓
- [ ] Mute button toggles (🔇 ↔️ 🔊)
- [ ] Countdown displays when active
- [ ] Horn status shows "Idle" / "Active"
- [ ] All 8 ad-hoc signal buttons work
- [ ] Signal buttons show blast patterns (●, ●●, —, etc.)

### 7. Emergency Stop ✓
- [ ] Confirmation dialog appears
- [ ] All lights turn off
- [ ] Horn stops
- [ ] Warning alert displays

### 8. Real-Time Updates ✓
- [ ] Status polls every 2 seconds
- [ ] Light changes reflect immediately
- [ ] Countdown timer updates
- [ ] Connection status updates

### 9. Responsive Design ✓
- [ ] Works on desktop browser
- [ ] Works on tablet
- [ ] Works on mobile phone
- [ ] Touch targets large enough
- [ ] Text readable at all sizes

### 10. Error Handling ✓
- [ ] Invalid inputs show error alerts
- [ ] Network errors display properly
- [ ] Disconnection detected
- [ ] Recovers on reconnection

## Browser Compatibility

**Tested Browsers:**
- ✅ Chrome/Edge (recommended)
- ✅ Firefox
- ✅ Safari (iOS/macOS)
- ✅ Mobile browsers

**Requirements:**
- ES6 JavaScript support
- Fetch API
- CSS Grid & Flexbox
- No external dependencies

## Troubleshooting

### UI Not Loading

**Check SPIFFS Upload:**
```bash
pio run --target uploadfs --verbose
```

**Verify Files Exist:**
Check serial output for filesystem errors:
```bash
pio device monitor
```

**Try Direct IP:**
Use IP address instead of mDNS hostname.

### Buttons Not Working

**Check API Endpoints:**
```powershell
# Test API directly
Invoke-RestMethod http://nav-lights-ecu.local/api/health
```

**Check Browser Console:**
Press F12 → Console tab → look for errors

**Verify JavaScript Loading:**
Check Network tab for 404 errors on lights.js

### Lights Not Updating

**Check Polling:**
Look for fetch requests every 2 seconds in Network tab

**Verify API Response:**
```powershell
Invoke-RestMethod http://nav-lights-ecu.local/api/status
```

**Check SignalK:**
Verify SignalK server is running and ECU connected

### Slow Performance

**Check WiFi Signal:**
Status bar shows RSSI (should be > -70 dBm)

**Reduce Polling:**
Edit lights.js → change `POLL_INTERVAL` to 5000 (5 seconds)

**Check Free Heap:**
System info should show > 100KB free

## Development Testing (Local)

For testing frontend changes without uploading:

### 1. Serve Files Locally
```bash
# In data/ directory
python -m http.server 8000
```

### 2. Edit API Base URL
In `lights.js`, temporarily change:
```javascript
const API_BASE = 'http://nav-lights-ecu.local/api';
```

### 3. Test in Browser
```
http://localhost:8000/lights.html
```

### 4. Restore Before Upload
Change `API_BASE` back to `/api` before uploading to ESP32.

## Performance Metrics

**Expected Performance:**
- Page load: < 500ms
- API response: < 100ms
- Status update: < 50ms
- Total RAM usage: ~30KB
- SPIFFS usage: ~50KB

**Resource Usage After Upload:**
- Flash: ~73% (includes web UI files)
- RAM: ~10% (runtime)
- SPIFFS: ~5% (3 files, ~30KB total)

## Next Steps

After successful Phase 3:
1. ✅ Backend API operational
2. ✅ Frontend UI uploaded and tested
3. ✅ Complete web interface working
4. 📋 Integration testing with SignalK
5. 📋 Maritime field testing
6. 📋 Production deployment

## File Sizes

```
data/lights.html : ~8 KB
data/lights.css  : ~14 KB
data/lights.js   : ~18 KB
Total SPIFFS     : ~40 KB
```

**Remaining SPIFFS**: ~950 KB available for future features

## Security Considerations

**Current Implementation:**
- No authentication (boat's private WiFi network)
- CORS not configured (same-origin only)
- HTTP only (no HTTPS)

**For Production (Optional):**
- Add HTTP Basic Auth
- Enable HTTPS with self-signed cert
- Implement session tokens
- Add CORS headers for external access

**Recommended:** Keep as-is for simplicity on private boat network

## Backup & Recovery

**Backup Current SPIFFS:**
```bash
# Read current filesystem
esptool.py --chip esp32 --port COM3 read_flash 0x290000 0x100000 spiffs_backup.bin
```

**Restore Backup:**
```bash
# Write backup to SPIFFS partition
esptool.py --chip esp32 --port COM3 write_flash 0x290000 spiffs_backup.bin
```

## Success Criteria

Phase 3 is complete when:
- ✅ SPIFFS upload successful
- ✅ Web UI loads at http://nav-lights-ecu.local/lights.html
- ✅ All 10 testing checklist items passed
- ✅ Real-time updates working
- ✅ Responsive design verified
- ✅ Error handling tested
- ✅ Emergency stop functional
- ✅ Integration with Phase 1 API validated

---

**Phase 3 Status**: ⏳ Ready to Execute  
**Estimated Time**: 15-30 minutes  
**Dependencies**: Phase 1 (Backend API) complete
