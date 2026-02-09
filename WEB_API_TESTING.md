# Web API Testing Guide - Phase 1 ✅ COMPLETE

**Status**: All 29 automated tests passing (100%)  
**Test Suite**: [test_web_api.ps1](test_web_api.ps1)  
**Last Run**: February 9, 2026

---

## Automated Testing (Recommended)

Run the comprehensive automated test suite:

```powershell
# Run all 29 tests (quiet mode)
.\test_web_api.ps1

# Run with verbose HTTP output
.\test_web_api.ps1 -Verbose

# Test against specific IP
.\test_web_api.ps1 -BaseUrl "http://10.100.100.244"
```

**Test Coverage**:
- ✅ 2 health/status tests
- ✅ 4 condition control tests (3 valid + 1 error)
- ✅ 7 boat state tests (6 valid + 1 error)
- ✅ 2 mute control tests
- ✅ 9 ad-hoc signal tests (8 valid + 1 error)
- ✅ 2 emergency stop tests
- ✅ 3 error handling tests (missing fields)

**Result**: 29/29 passing (100%)

---

## Manual Testing (Legacy)

The following manual tests were used during development. For production validation, use the automated test suite above.

---

# Manual Web API Testing Guide

## Prerequisites
- ESP32 with firmware flashed
- ESP32 connected to WiFi network
- Know the IP address or use mDNS: `nav-lights-ecu.local`

## Automated Testing (Recommended)

Run the complete test suite automatically:

```powershell
# Run all tests with default URL
.\test_web_api.ps1

# Run with custom URL
.\test_web_api.ps1 -BaseUrl "http://192.168.1.100"

# Run with verbose output
.\test_web_api.ps1 -Verbose
```

The automated test suite (`test_web_api.ps1`) performs:
- ✓ Health and status endpoint validation
- ✓ All condition transitions (day, hours_of_darkness, restricted_visibility)
- ✓ All boat state transitions (6 states)
- ✓ Mute/unmute functionality
- ✓ All 8 ad-hoc signal types
- ✓ Emergency stop and verification
- ✓ Error handling and validation
- ✓ Response format checking

**Results**: Pass/fail summary with color-coded output and exit code for CI/CD integration.

---

## Manual Testing Tools

### Option 1: curl (Command Line)
```bash
# Find your ESP32 IP if mDNS doesn't work
ping nav-lights-ecu.local

# Or use IP directly
ESP32_IP=nav-lights-ecu.local  # or 192.168.x.x
```

### Option 2: PowerShell (Windows)
```powershell
$ESP32_IP = "nav-lights-ecu.local"  # or actual IP
```

---

## Test 1: Health Check

### curl
```bash
curl http://$ESP32_IP/api/health
```

### PowerShell
```powershell
Invoke-WebRequest -Uri "http://$ESP32_IP/api/health" | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "healthy": true,
  "uptime": 123,
  "freeHeap": 450000,
  "wifiRSSI": -45,
  "signalkConnected": false
}
```

---

## Test 2: Get Status

### curl
```bash
curl http://$ESP32_IP/api/status
```

### PowerShell
```powershell
Invoke-WebRequest -Uri "http://$ESP32_IP/api/status" | Select-Object -Expand Content | ConvertFrom-Json | ConvertTo-Json -Depth 10
```

**Expected Response:**
```json
{
  "condition": "day",
  "boatState": "moored",
  "periodicMuted": true,
  "periodicCountdown": 0,
  "lights": {
    "masthead": false,
    "portSidelight": false,
    "starboardSidelight": false,
    "sternlight": false,
    "allroundWhite": false,
    "allroundRedUpper": false,
    "allroundRedLower": false
  },
  "horn": {
    "active": false
  },
  "signalkConnected": false,
  "uptime": 123,
  "freeHeap": 450000
}
```

---

## Test 3: Set Condition to Darkness

### curl
```bash
curl -X POST http://$ESP32_IP/api/condition \
  -H "Content-Type: application/json" \
  -d '{"value":"hours_of_darkness"}'
```

### PowerShell
```powershell
$body = @{
    value = "hours_of_darkness"
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://$ESP32_IP/api/condition" `
  -Method POST `
  -ContentType "application/json" `
  -Body $body | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "success": true,
  "condition": "hours_of_darkness"
}
```

**Verify:** Check status again - lights should still be OFF (moored state)

---

## Test 4: Set Boat State to Underway

### curl
```bash
curl -X POST http://$ESP32_IP/api/state \
  -H "Content-Type: application/json" \
  -d '{"value":"underway_making_way"}'
```

### PowerShell
```powershell
$body = @{
    value = "underway_making_way"
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://$ESP32_IP/api/state" `
  -Method POST `
  -ContentType "application/json" `
  -Body $body | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "success": true,
  "boatState": "underway_making_way"
}
```

**Verify:** Check status - navigation lights should now be ON
- masthead: true
- portSidelight: true
- starboardSidelight: true
- sternlight: true

---

## Test 5: Unmute Periodic Signals

### curl
```bash
curl -X POST http://$ESP32_IP/api/mute \
  -H "Content-Type: application/json" \
  -d '{"muted":false}'
```

### PowerShell
```powershell
$body = @{
    muted = $false
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://$ESP32_IP/api/mute" `
  -Method POST `
  -ContentType "application/json" `
  -Body $body | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "success": true,
  "periodicMuted": false
}
```

---

## Test 6: Trigger Ad-Hoc Signal

### curl
```bash
curl -X POST http://$ESP32_IP/api/signal \
  -H "Content-Type: application/json" \
  -d '{"signal":"turn_starboard"}'
```

### PowerShell
```powershell
$body = @{
    signal = "turn_starboard"
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://$ESP32_IP/api/signal" `
  -Method POST `
  -ContentType "application/json" `
  -Body $body | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "success": true,
  "triggered": "turn_starboard"
}
```

**Verify:** Horn should sound 1 short blast

---

## Test 7: All Valid Ad-Hoc Signals

```bash
# Test all ad-hoc signals
signals=("turn_starboard" "turn_port" "astern_propulsion" "danger_confusion" "pay_attention" "overtake_starboard" "overtake_port" "agreement_overtaken")

for signal in "${signals[@]}"; do
  echo "Testing signal: $signal"
  curl -X POST http://$ESP32_IP/api/signal \
    -H "Content-Type: application/json" \
    -d "{\"signal\":\"$signal\"}"
  echo
  sleep 5  # Wait for signal to complete
done
```

---

## Test 8: Emergency Stop

### curl
```bash
curl -X POST http://$ESP32_IP/api/emergency \
  -H "Content-Type: application/json" \
  -d '{}'
```

### PowerShell
```powershell
Invoke-WebRequest -Uri "http://$ESP32_IP/api/emergency" `
  -Method POST `
  -ContentType "application/json" `
  -Body '{}' | Select-Object -Expand Content
```

**Expected Response:**
```json
{
  "success": true,
  "stopped": true
}
```

**Verify:** All lights OFF, horn silent

---

## Test 9: Error Handling - Invalid Condition

### curl
```bash
curl -X POST http://$ESP32_IP/api/condition \
  -H "Content-Type: application/json" \
  -d '{"value":"invalid_condition"}'
```

**Expected Response:**
```json
{
  "success": false,
  "error": "Invalid condition value",
  "code": 400
}
```

---

## Test 10: Error Handling - Missing Field

### curl
```bash
curl -X POST http://$ESP32_IP/api/condition \
  -H "Content-Type: application/json" \
  -d '{}'
```

**Expected Response:**
```json
{
  "success": false,
  "error": "Missing 'value' field",
  "code": 400
}
```

---

## Complete Test Sequence (End-to-End)

```bash
ESP32_IP=nav-lights-ecu.local

echo "=== 1. Initial Status ==="
curl http://$ESP32_IP/api/status

echo -e "\n=== 2. Set to Darkness ==="
curl -X POST http://$ESP32_IP/api/condition -H "Content-Type: application/json" -d '{"value":"hours_of_darkness"}'

echo -e "\n=== 3. Set to Underway ==="
curl -X POST http://$ESP32_IP/api/state -H "Content-Type: application/json" -d '{"value":"underway_making_way"}'

echo -e "\n=== 4. Check Lights ON ==="
curl http://$ESP32_IP/api/status | grep -A 10 '"lights"'

echo -e "\n=== 5. Trigger Signal ==="
curl -X POST http://$ESP32_IP/api/signal -H "Content-Type: application/json" -d '{"signal":"turn_starboard"}'

sleep 2

echo -e "\n=== 6. Emergency Stop ==="
curl -X POST http://$ESP32_IP/api/emergency -H "Content-Type: application/json" -d '{}'

echo -e "\n=== 7. Final Status (should be OFF) ==="
curl http://$ESP32_IP/api/status | grep -A 10 '"lights"'
```

---

## Monitoring Serial Output

While testing, keep the serial monitor open to see debug output:

```bash
pio device monitor
```

You should see messages like:
```
Setting up Web API endpoints...
Web API endpoints registered:
  GET  /api/status
  GET  /api/health
  POST /api/condition
  POST /api/state
  POST /api/mute
  POST /api/signal
  POST /api/emergency
```

---

## Troubleshooting

### Problem: Cannot connect to ESP32

**Check WiFi Connection:**
```bash
ping nav-lights-ecu.local
# or
ping 192.168.x.x
```

**Check Serial Monitor:**
```bash
pio device monitor
```
Look for WiFi connection messages

### Problem: 404 Not Found

**Verify endpoints are registered:**
Check serial output during boot for "Web API endpoints registered" message

**Try direct IP:**
```bash
curl http://192.168.1.100/api/status  # Use actual IP
```

### Problem: Invalid JSON error

**Verify Content-Type header:**
Must be `Content-Type: application/json`

**Check JSON syntax:**
Use a JSON validator or `jq`:
```bash
echo '{"value":"day"}' | jq .
```

### Problem: Lights don't change

**Check Serial Monitor:**
Look for state change messages

**Verify hardware:**
Check relay module is powered and connected

**Check active-low/high setting:**
See platformio.ini for `ACTIVE_HIGH_RELAYS` flag

---

## Success Criteria

✅ All GET endpoints return valid JSON  
✅ All POST endpoints accept valid input  
✅ Invalid input returns 400 error  
✅ Condition changes trigger light updates  
✅ State changes trigger light updates  
✅ Ad-hoc signals trigger horn  
✅ Emergency stop disables all outputs  
✅ Status endpoint reflects current state  

---

## Next Steps

Once all API tests pass:
1. ✅ Phase 1 Complete - Backend API working
2. ➡️ Proceed to Phase 2 - Frontend HTML/CSS/JS
3. Upload static files to SPIFFS
4. Test full web UI in browser
