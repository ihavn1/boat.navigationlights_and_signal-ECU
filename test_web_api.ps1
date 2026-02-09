#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Automated test suite for Navigation Lights ECU Web API
.DESCRIPTION
    Tests all REST API endpoints with various scenarios and reports results
.PARAMETER BaseUrl
    Base URL of the ECU (default: http://nav-lights-ecu.local)
.PARAMETER Verbose
    Show detailed test output
.EXAMPLE
    .\test_web_api.ps1
.EXAMPLE
    .\test_web_api.ps1 -BaseUrl "http://192.168.1.100"
#>

param(
    [string]$BaseUrl = "http://nav-lights-ecu.local",
    [switch]$Verbose
)

# Test results tracking
$script:TestsPassed = 0
$script:TestsFailed = 0
$script:TestsSkipped = 0

# Colors for output
$Green = "Green"
$Red = "Red"
$Yellow = "Yellow"
$Cyan = "Cyan"

function Write-TestHeader {
    param([string]$Message)
    Write-Host "`n========================================" -ForegroundColor $Cyan
    Write-Host $Message -ForegroundColor $Cyan
    Write-Host "========================================" -ForegroundColor $Cyan
}

function Write-TestResult {
    param(
        [string]$TestName,
        [bool]$Passed,
        [string]$Details = ""
    )
    
    if ($Passed) {
        Write-Host "  [PASS] " -ForegroundColor $Green -NoNewline
        Write-Host "$TestName" -ForegroundColor White
        if ($Verbose -and $Details) {
            Write-Host "    $Details" -ForegroundColor Gray
        }
        $script:TestsPassed++
    } else {
        Write-Host "  [FAIL] " -ForegroundColor $Red -NoNewline
        Write-Host "$TestName" -ForegroundColor White
        if ($Details) {
            Write-Host "    ERROR: $Details" -ForegroundColor Red
        }
        $script:TestsFailed++
    }
}

function Test-Endpoint {
    param(
        [string]$Method,
        [string]$Path,
        [hashtable]$Body = $null,
        [string]$TestName,
        [scriptblock]$Validator
    )
    
    try {
        $uri = "$BaseUrl$Path"
        
        if ($Verbose) {
            Write-Host "    -> $Method $uri" -ForegroundColor Gray
            if ($Body) {
                Write-Host "      Body: $($Body | ConvertTo-Json -Compress)" -ForegroundColor Gray
            }
        }
        
        $params = @{
            Uri = $uri
            Method = $Method
            ContentType = "application/json"
            ErrorAction = "Stop"
        }
        
        if ($Body) {
            $params.Body = ($Body | ConvertTo-Json)
        }
        
        $response = Invoke-RestMethod @params
        
        if ($Verbose) {
            Write-Host "      Response: $($response | ConvertTo-Json -Compress)" -ForegroundColor Gray
        }
        
        # Run validator
        if ($Validator) {
            $validationResult = & $Validator $response
            if ($validationResult -is [string]) {
                Write-TestResult -TestName $TestName -Passed $false -Details $validationResult
            } else {
                Write-TestResult -TestName $TestName -Passed $validationResult
            }
        } else {
            Write-TestResult -TestName $TestName -Passed $true
        }
        
        return $response
        
    } catch {
        # Check if this is an HTTP error with a response body
        if ($_.Exception.Response) {
            $statusCode = [int]$_.Exception.Response.StatusCode
            
            # PowerShell automatically populates ErrorDetails.Message with response body
            $responseBody = $_.ErrorDetails.Message
            
            if (!$responseBody) {
                # Fallback to reading stream manually if ErrorDetails not populated
                try {
                    $reader = New-Object System.IO.StreamReader($_.Exception.Response.GetResponseStream())
                    $responseBody = $reader.ReadToEnd()
                    $reader.Close()
                } catch {
                    $responseBody = "{}"
                }
            }
            
            if ($Verbose) {
                Write-Host "      Error Response ($statusCode): $responseBody" -ForegroundColor Gray
            }
            
            # Try to parse as JSON
            try {
                $errorResponse = $responseBody | ConvertFrom-Json
                
                # Run validator with error response
                if ($Validator) {
                    $validationResult = & $Validator $errorResponse $statusCode
                    if ($validationResult -is [string]) {
                        Write-TestResult -TestName $TestName -Passed $false -Details $validationResult
                    } else {
                        Write-TestResult -TestName $TestName -Passed $validationResult
                    }
                } else {
                    Write-TestResult -TestName $TestName -Passed $false -Details "HTTP $statusCode"
                }
                
                return $errorResponse
            } catch {
                Write-TestResult -TestName $TestName -Passed $false -Details "HTTP $statusCode - Could not parse error response"
                return $null
            }
        } else {
            # Non-HTTP error (network, timeout, etc)
            Write-TestResult -TestName $TestName -Passed $false -Details $_.Exception.Message
            return $null
        }
    }
}

# ============================================================================
# Health & Status Tests
# ============================================================================

Write-TestHeader "Health & Status Endpoints"

Test-Endpoint -Method GET -Path "/api/health" -TestName "GET /api/health" -Validator {
    param($r)
    if ($r.healthy -ne $true) { return "healthy should be true" }
    if ($null -eq $r.uptime) { return "uptime missing" }
    if ($null -eq $r.freeHeap) { return "freeHeap missing" }
    if ($null -eq $r.wifiRSSI) { return "wifiRSSI missing" }
    if ($null -eq $r.signalkConnected) { return "signalkConnected missing" }
    return $true
}

$statusResponse = Test-Endpoint -Method GET -Path "/api/status" -TestName "GET /api/status" -Validator {
    param($r)
    if (-not $r.condition) { return "condition missing" }
    if (-not $r.boatState) { return "boatState missing" }
    if ($null -eq $r.periodicMuted) { return "periodicMuted missing" }
    if ($null -eq $r.periodicCountdown) { return "periodicCountdown missing" }
    if (-not $r.lights) { return "lights object missing" }
    if (-not $r.horn) { return "horn object missing" }
    return $true
}

# ============================================================================
# Condition Tests
# ============================================================================

Write-TestHeader "Condition Control Tests"

Test-Endpoint -Method POST -Path "/api/condition" -TestName "Set condition: day" `
    -Body @{ value = "day" } -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.condition -ne "day") { return "condition should be day" }
    return $true
}

Start-Sleep -Milliseconds 500

Test-Endpoint -Method POST -Path "/api/condition" -TestName "Set condition: hours_of_darkness" `
    -Body @{ value = "hours_of_darkness" } -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.condition -ne "hours_of_darkness") { return "condition should be hours_of_darkness" }
    return $true
}

Start-Sleep -Milliseconds 500

Test-Endpoint -Method POST -Path "/api/condition" -TestName "Set condition: restricted_visibility" `
    -Body @{ value = "restricted_visibility" } -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.condition -ne "restricted_visibility") { return "condition should be restricted_visibility" }
    return $true
}

Start-Sleep -Milliseconds 500

Test-Endpoint -Method POST -Path "/api/condition" -TestName "Invalid condition (should fail)" `
    -Body @{ value = "invalid_condition" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

# ============================================================================
# Boat State Tests
# ============================================================================

Write-TestHeader "Boat State Control Tests"

$states = @(
    "moored",
    "underway_making_way",
    "underway_no_way",
    "anchorage",
    "nuc_making_way",
    "nuc_no_way"
)

foreach ($state in $states) {
    Test-Endpoint -Method POST -Path "/api/state" -TestName "Set state: $state" `
        -Body @{ value = $state } -Validator {
        param($r)
        if ($r.success -ne $true) { return "success should be true" }
        if ($r.boatState -ne $state) { return "boatState should be $state" }
        return $true
    }
    Start-Sleep -Milliseconds 500
}

Test-Endpoint -Method POST -Path "/api/state" -TestName "Invalid state (should fail)" `
    -Body @{ value = "invalid_state" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

# ============================================================================
# Mute Control Tests
# ============================================================================

Write-TestHeader "Mute Control Tests"

Test-Endpoint -Method POST -Path "/api/mute" -TestName "Mute periodic signals" `
    -Body @{ muted = $true } -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.periodicMuted -ne $true) { return "periodicMuted should be true" }
    return $true
}

Start-Sleep -Milliseconds 500

Test-Endpoint -Method POST -Path "/api/mute" -TestName "Unmute periodic signals" `
    -Body @{ muted = $false } -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.periodicMuted -ne $false) { return "periodicMuted should be false" }
    return $true
}

# ============================================================================
# Ad-Hoc Signal Tests
# ============================================================================

Write-TestHeader "Ad-Hoc Signal Tests"

$signals = @(
    "turn_starboard",
    "turn_port",
    "astern_propulsion",
    "danger_confusion",
    "pay_attention",
    "overtake_starboard",
    "overtake_port",
    "agreement_overtaken"
)

foreach ($signal in $signals) {
    Test-Endpoint -Method POST -Path "/api/signal" -TestName "Trigger signal: $signal" `
        -Body @{ signal = $signal } -Validator {
        param($r)
        if ($r.success -ne $true) { return "success should be true" }
        if ($r.triggered -ne $signal) { return "triggered should be $signal" }
        return $true
    }
    Start-Sleep -Milliseconds 2500  # Wait for signal to complete
}

Test-Endpoint -Method POST -Path "/api/signal" -TestName "Invalid signal (should fail)" `
    -Body @{ signal = "invalid_signal" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

# ============================================================================
# Emergency Stop Test
# ============================================================================

Write-TestHeader "Emergency Stop Test"

Test-Endpoint -Method POST -Path "/api/emergency" -TestName "Emergency stop" `
    -Body @{} -Validator {
    param($r)
    if ($r.success -ne $true) { return "success should be true" }
    if ($r.stopped -ne $true) { return "stopped should be true" }
    return $true
}

Start-Sleep -Milliseconds 500

# Verify all lights are off after emergency stop
$verifyResponse = Test-Endpoint -Method GET -Path "/api/status" -TestName "Verify emergency stop cleared lights" -Validator {
    param($r)
    $lights = $r.lights
    if ($lights.masthead -or $lights.portSidelight -or $lights.starboardSidelight -or 
        $lights.sternlight -or $lights.allroundWhite -or $lights.allroundRedUpper -or 
        $lights.allroundRedLower) {
        return "Some lights still active after emergency stop"
    }
    if ($r.horn.active) { return "Horn still active after emergency stop" }
    return $true
}

# ============================================================================
# Error Handling Tests
# ============================================================================

Write-TestHeader "Error Handling Tests"

# Missing required fields
Test-Endpoint -Method POST -Path "/api/condition" -TestName "Missing value field" `
    -Body @{ wrong_field = "test" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

Test-Endpoint -Method POST -Path "/api/mute" -TestName "Missing muted field" `
    -Body @{ wrong_field = "test" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

Test-Endpoint -Method POST -Path "/api/signal" -TestName "Missing signal field" `
    -Body @{ wrong_field = "test" } -Validator {
    param($r, $statusCode)
    if ($statusCode -ne 400) { return "Expected HTTP 400, got $statusCode" }
    if ($r.success -eq $true) { return "success should be false for errors" }
    if (-not $r.error) { return "error message missing" }
    return $true
}

# ============================================================================
# Test Summary
# ============================================================================

Write-Host "`n========================================" -ForegroundColor $Cyan
Write-Host "Test Summary" -ForegroundColor $Cyan
Write-Host "========================================" -ForegroundColor $Cyan

$total = $script:TestsPassed + $script:TestsFailed + $script:TestsSkipped

Write-Host "Total Tests:  " -NoNewline
Write-Host $total -ForegroundColor White

Write-Host "Passed:       " -NoNewline
Write-Host $script:TestsPassed -ForegroundColor $Green

if ($script:TestsFailed -gt 0) {
    Write-Host "Failed:       " -NoNewline
    Write-Host $script:TestsFailed -ForegroundColor $Red
}

if ($script:TestsSkipped -gt 0) {
    Write-Host "Skipped:      " -NoNewline
    Write-Host $script:TestsSkipped -ForegroundColor $Yellow
}

$passRate = [math]::Round(($script:TestsPassed / $total) * 100, 1)
Write-Host "Pass Rate:    " -NoNewline
$passRateColor = if ($passRate -eq 100) { $Green } elseif ($passRate -ge 90) { $Yellow } else { $Red }
Write-Host "$passRate%" -ForegroundColor $passRateColor

Write-Host "`n"

# Exit code
exit $script:TestsFailed
