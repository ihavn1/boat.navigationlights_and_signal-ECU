/**
 * Navigation Lights ECU - Frontend JavaScript
 * Handles API communication, UI updates, and real-time status monitoring
 */

// ============================================================================
// Configuration & State
// ============================================================================

const API_BASE = '/api';
const POLL_INTERVAL = 2000; // 2 seconds
const ALERT_TIMEOUT = 5000; // 5 seconds

let pollTimer = null;
let currentStatus = null;
let isConnected = false;

// ============================================================================
// Initialization
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
    console.log('Navigation Lights ECU - Initializing UI');
    
    // Set up event listeners
    setupConditionButtons();
    setupStateButtons();
    setupMuteButton();
    setupSignalButtons();
    setupEmergencyButton();
    
    // Start polling
    startPolling();
    
    // Initial status fetch
    fetchStatus();
});

// ============================================================================
// API Communication
// ============================================================================

/**
 * Make API request with error handling
 */
async function apiRequest(endpoint, options = {}) {
    const url = `${API_BASE}${endpoint}`;
    
    try {
        const response = await fetch(url, {
            ...options,
            headers: {
                'Content-Type': 'application/json',
                ...options.headers
            }
        });
        
        const data = await response.json();
        
        if (!response.ok) {
            throw new Error(data.error || `HTTP ${response.status}`);
        }
        
        return data;
    } catch (error) {
        console.error(`API Error (${endpoint}):`, error);
        throw error;
    }
}

/**
 * Fetch current ECU status
 */
async function fetchStatus() {
    try {
        const status = await apiRequest('/status');
        currentStatus = status;
        updateUI(status);
        setConnectionStatus(true);
    } catch (error) {
        setConnectionStatus(false);
        showAlert(`Failed to fetch status: ${error.message}`, 'error');
    }
}

/**
 * Fetch health information
 */
async function fetchHealth() {
    try {
        const health = await apiRequest('/health');
        updateHealthUI(health);
    } catch (error) {
        console.error('Health check failed:', error);
    }
}

// ============================================================================
// UI Update Functions
// ============================================================================

/**
 * Apply hardware capabilities (hide unavailable lights in diagram)
 * Note: State buttons remain visible for day shapes (black balls/diamonds)
 */
function applyCapabilities(capabilities) {
    console.log('Applying hardware capabilities:', capabilities);
    
    // Hide NUC lights in boat diagram if not installed
    // (but keep NUC state buttons - needed for day shapes)
    if (capabilities.hasNucLights === false) {
        const nucUpper = document.getElementById('nuc-upper');
        const nucLower = document.getElementById('nuc-lower');
        if (nucUpper) {
            nucUpper.style.display = 'none';
            console.log('NUC lights hidden in diagram (hardware not installed)');
        }
        if (nucLower) nucLower.style.display = 'none';
    }
    
    // Hide towing light in boat diagram if not installed
    // (but keep Towing state button - needed for day shapes)
    if (capabilities.hasTowingLights === false) {
        const towingLight = document.getElementById('yellow-towing-light');
        if (towingLight) {
            towingLight.style.display = 'none';
            console.log('Towing light hidden in diagram (hardware not installed)');
        }
    }
}

/**
 * Update all UI elements with current status
 */
function updateUI(status) {
    if (!status) return;
    
    // Apply hardware capabilities (only on first load)
    if (status.capabilities && !window.capabilitiesApplied) {
        applyCapabilities(status.capabilities);
        window.capabilitiesApplied = true;
    }
    
    // Update condition
    updateConditionUI(status.condition);
    
    // Update boat state
    updateStateUI(status.boatState);
    
    // Update day shapes (black balls)
    updateDayShapes(status.condition, status.boatState);
    
    // Update lights
    updateLightsUI(status.lights);
    
    // Update horn
    updateHornUI(status.horn);
    
    // Update mute status
    updateMuteUI(status.periodicMuted, status.periodicCountdown);
    
    // Update SignalK connection
    updateSignalKStatus(status.signalkConnected);
    
    // Update system info
    if (status.uptime !== undefined) {
        document.getElementById('uptime').textContent = formatUptime(status.uptime);
    }
    if (status.freeHeap !== undefined) {
        document.getElementById('freeHeap').textContent = formatBytes(status.freeHeap);
    }
}

/**
 * Update health UI elements
 */
function updateHealthUI(health) {
    if (!health) return;
    
    if (health.uptime !== undefined) {
        document.getElementById('uptime').textContent = formatUptime(health.uptime);
    }
    if (health.freeHeap !== undefined) {
        document.getElementById('freeHeap').textContent = formatBytes(health.freeHeap);
    }
    if (health.wifiRSSI !== undefined) {
        document.getElementById('wifiRSSI').textContent = `${health.wifiRSSI} dBm`;
    }
    if (health.signalkConnected !== undefined) {
        updateSignalKStatus(health.signalkConnected);
    }
}

/**
 * Update condition button states
 */
function updateConditionUI(condition) {
    const buttons = document.querySelectorAll('.condition-btn');
    buttons.forEach(btn => {
        const isActive = btn.dataset.value === condition;
        btn.classList.toggle('active', isActive);
    });
    
    const label = formatConditionLabel(condition);
    document.getElementById('currentCondition').textContent = label;
    
    // Update boat diagram background based on condition
    const boatContainer = document.getElementById('boatDiagramContainer');
    if (boatContainer) {
        // Remove all condition classes
        boatContainer.classList.remove('condition-day', 'condition-hours_of_darkness', 'condition-restricted_visibility');
        // Add current condition class
        boatContainer.classList.add('condition-' + condition);
    }
}

/**
 * Update boat state button states
 */
function updateStateUI(state) {
    const buttons = document.querySelectorAll('.state-btn');
    buttons.forEach(btn => {
        const isActive = btn.dataset.value === state;
        btn.classList.toggle('active', isActive);
    });
    
    const label = formatStateLabel(state);
    document.getElementById('currentState').textContent = label;
}

/**
 * Update day shape visibility (COLREGs Rules 27 & 30)
 * - Rule 30: Anchored vessel shows single black ball
 * - Rule 27: NUC vessel shows two black balls in vertical line
 */
function updateDayShapes(condition, boatState) {
    // Day shapes only visible during day or restricted visibility
    const isDayOrRestricted = condition === 'day' || condition === 'restricted_visibility';
    
    const anchorBall = document.getElementById('anchor-ball');
    const nucBallUpper = document.getElementById('nuc-ball-upper');
    
    if (anchorBall && nucBallUpper) {
        // Anchor ball: Show single ball for anchorage
        const isAnchored = boatState === 'anchorage';
        const showAnchorBall = isDayOrRestricted && isAnchored;
        anchorBall.style.display = showAnchorBall ? 'block' : 'none';
        
        // NUC balls: Show both balls (lower is same as anchor ball position + upper)
        const isNUC = boatState === 'nuc_making_way' || boatState === 'nuc_no_way';
        const showNUCBalls = isDayOrRestricted && isNUC;
        
        // When NUC, the anchor-ball element acts as the lower NUC ball
        if (showNUCBalls) {
            anchorBall.style.display = 'block';  // Lower NUC ball
            nucBallUpper.style.display = 'block';  // Upper NUC ball
        } else if (!showAnchorBall) {
            anchorBall.style.display = 'none';
            nucBallUpper.style.display = 'none';
        } else {
            // Anchor only (upper ball hidden)
            nucBallUpper.style.display = 'none';
        }
    }
}

/**
 * Update light indicators
 */
function updateLightsUI(lights) {
    if (!lights) return;
    
    Object.keys(lights).forEach(lightName => {
        const indicator = document.querySelector(`[data-light="${lightName}"] .light-icon`);
        if (indicator) {
            const isOn = lights[lightName];
            indicator.classList.toggle('on', isOn);
            indicator.classList.toggle('off', !isOn);
        }
    });
    
    // Update boat diagram
    updateBoatDiagram(lights);
}

/**
 * Update boat diagram SVG lights
 */
function updateBoatDiagram(lights) {
    applyLightsToBoatDiagram(lights);
}

/**
 * Apply light states to boat diagram SVG elements
 */
function applyLightsToBoatDiagram(lights) {
    // Map light names to SVG element IDs
    const lightMap = {
        'masthead': 'masthead-light',
        'sternlight': 'sternlight',
        'allroundWhite': 'allround-white',
        'allroundRedUpper': 'nuc-upper',
        'allroundRedLower': 'nuc-lower',
        'yellowTowingLight': 'yellow-towing-light'
    };
    
    // Update each light in the diagram
    Object.keys(lightMap).forEach(lightName => {
        const svgId = lightMap[lightName];
        const element = document.getElementById(svgId);
        
        if (element) {
            const isOn = lights[lightName];
            if (isOn) {
                element.classList.add('active');
            } else {
                element.classList.remove('active');
            }
        }
    });
    
    // Special handling: visible starboard sidelight represents both port and starboard
    // Illuminate it when either sidelight is active
    const starboardElement = document.getElementById('starboard-sidelight');
    if (starboardElement) {
        const sidelightsOn = lights['portSidelight'] || lights['starboardSidelight'];
        if (sidelightsOn) {
            starboardElement.classList.add('active');
        } else {
            starboardElement.classList.remove('active');
        }
    }
}

/**
 * Update horn status
 */
function updateHornUI(horn) {
    const hornStatus = document.getElementById('hornStatus');
    const hornState = document.getElementById('hornState');
    
    if (horn && horn.active) {
        hornStatus.classList.add('active');
        hornState.textContent = 'Active';
    } else {
        hornStatus.classList.remove('active');
        hornState.textContent = 'Idle';
    }
}

// Track maximum countdown to calculate progress percentage
let maxCountdown = 120; // Default 2 minutes (typical periodic interval)

/**
 * Update mute button, countdown, and progress bar
 */
function updateMuteUI(isMuted, countdown) {
    const muteBtn = document.getElementById('muteBtn');
    const muteIcon = muteBtn.querySelector('.mute-icon');
    const muteText = muteBtn.querySelector('.mute-text');
    const countdownEl = document.getElementById('countdown');
    const progressBar = document.getElementById('progressBar');
    
    if (isMuted) {
        muteBtn.classList.remove('active');
        muteIcon.textContent = '🔇';
        muteText.textContent = 'Periodic Muted';
        countdownEl.textContent = '--';
        progressBar.style.width = '0%';
        progressBar.classList.add('muted');
    } else {
        muteBtn.classList.add('active');
        muteIcon.textContent = '🔊';
        muteText.textContent = 'Periodic Active';
        countdownEl.textContent = countdown || '--';
        progressBar.classList.remove('muted');
        
        // Update max countdown if we see a higher value (signal just started)
        if (countdown > maxCountdown * 0.9) {
            maxCountdown = Math.max(maxCountdown, countdown);
        }
        
        // Calculate progress: bar fills up as countdown decreases
        // 0% when countdown = maxCountdown (just started)
        // 100% when countdown = 0 (about to play)
        const progress = countdown !== null && maxCountdown > 0
            ? Math.max(0, Math.min(100, ((maxCountdown - countdown) / maxCountdown) * 100))
            : 0;
        
        progressBar.style.width = `${progress}%`;
    }
}

/**
 * Update connection status badge
 */
function setConnectionStatus(connected) {
    isConnected = connected;
    const badge = document.getElementById('connectionStatus');
    
    if (connected) {
        badge.textContent = 'Connected';
        badge.classList.remove('disconnected');
        badge.classList.remove('blink');
        badge.classList.add('connected');
    } else {
        badge.textContent = 'Disconnected';
        badge.classList.remove('connected');
        badge.classList.add('disconnected');
        badge.classList.add('blink');
    }
}

/**
 * Update SignalK connection status
 */
function updateSignalKStatus(connected) {
    const badge = document.getElementById('signalkStatus');
    badge.textContent = `SignalK: ${connected ? 'Connected' : 'Disconnected'}`;
}

// ============================================================================
// Button Event Handlers
// ============================================================================

/**
 * Set up condition selection buttons
 */
function setupConditionButtons() {
    const buttons = document.querySelectorAll('.condition-btn');
    buttons.forEach(btn => {
        btn.addEventListener('click', async () => {
            const value = btn.dataset.value;
            await setCondition(value);
        });
    });
}

/**
 * Set up boat state selection buttons
 */
function setupStateButtons() {
    const buttons = document.querySelectorAll('.state-btn');
    buttons.forEach(btn => {
        btn.addEventListener('click', async () => {
            const value = btn.dataset.value;
            await setState(value);
        });
    });
}

/**
 * Set up mute/unmute button
 */
function setupMuteButton() {
    const btn = document.getElementById('muteBtn');
    btn.addEventListener('click', async () => {
        const currentlyMuted = currentStatus?.periodicMuted ?? true;
        await setMute(!currentlyMuted);
    });
}

/**
 * Set up ad-hoc signal buttons
 */
function setupSignalButtons() {
    const buttons = document.querySelectorAll('.signal-btn');
    buttons.forEach(btn => {
        btn.addEventListener('click', async () => {
            const signal = btn.dataset.signal;
            await triggerSignal(signal);
        });
    });
}

/**
 * Set up emergency stop button
 */
function setupEmergencyButton() {
    const btn = document.getElementById('emergencyBtn');
    btn.addEventListener('click', async () => {
        if (confirm('Emergency stop will turn off all lights and horn. Continue?')) {
            await emergencyStop();
        }
    });
}

// ============================================================================
// API Actions
// ============================================================================

/**
 * Set lighting condition
 */
async function setCondition(value) {
    try {
        disableButtons();
        const result = await apiRequest('/condition', {
            method: 'POST',
            body: JSON.stringify({ value })
        });
        
        if (result.success) {
            showAlert(`Condition set to: ${formatConditionLabel(result.condition)}`, 'success');
            await fetchStatus();
        }
    } catch (error) {
        showAlert(`Failed to set condition: ${error.message}`, 'error');
    } finally {
        enableButtons();
    }
}

/**
 * Set boat state
 */
async function setState(value) {
    try {
        disableButtons();
        const result = await apiRequest('/state', {
            method: 'POST',
            body: JSON.stringify({ value })
        });
        
        if (result.success) {
            showAlert(`Boat state set to: ${formatStateLabel(result.boatState)}`, 'success');
            await fetchStatus();
        }
    } catch (error) {
        showAlert(`Failed to set state: ${error.message}`, 'error');
    } finally {
        enableButtons();
    }
}

/**
 * Set mute status
 */
async function setMute(muted) {
    try {
        disableButtons();
        const result = await apiRequest('/mute', {
            method: 'POST',
            body: JSON.stringify({ muted })
        });
        
        if (result.success) {
            const status = muted ? 'muted' : 'active';
            showAlert(`Periodic signals ${status}`, 'success');
            await fetchStatus();
        }
    } catch (error) {
        showAlert(`Failed to set mute: ${error.message}`, 'error');
    } finally {
        enableButtons();
    }
}

/**
 * Trigger ad-hoc signal
 */
async function triggerSignal(signal) {
    try {
        disableButtons();
        const result = await apiRequest('/signal', {
            method: 'POST',
            body: JSON.stringify({ signal })
        });
        
        if (result.success) {
            showAlert(`Signal triggered: ${formatSignalLabel(signal)}`, 'success');
            await fetchStatus();
        }
    } catch (error) {
        showAlert(`Failed to trigger signal: ${error.message}`, 'error');
    } finally {
        enableButtons();
    }
}

/**
 * Emergency stop
 */
async function emergencyStop() {
    try {
        disableButtons();
        const result = await apiRequest('/emergency', {
            method: 'POST',
            body: JSON.stringify({})
        });
        
        if (result.success) {
            showAlert('EMERGENCY STOP ACTIVATED', 'warning');
            await fetchStatus();
        }
    } catch (error) {
        showAlert(`Emergency stop failed: ${error.message}`, 'error');
    } finally {
        enableButtons();
    }
}

// ============================================================================
// Polling & Auto-Refresh
// ============================================================================

/**
 * Start periodic status polling
 */
function startPolling() {
    if (pollTimer) {
        clearInterval(pollTimer);
    }
    
    pollTimer = setInterval(async () => {
        await fetchStatus();
        await fetchHealth();
    }, POLL_INTERVAL);
}

/**
 * Stop polling
 */
function stopPolling() {
    if (pollTimer) {
        clearInterval(pollTimer);
        pollTimer = null;
    }
}

// ============================================================================
// UI Helper Functions
// ============================================================================

/**
 * Show alert banner
 */
function showAlert(message, type = 'error') {
    const banner = document.getElementById('alertBanner');
    banner.textContent = message;
    banner.className = `alert-banner ${type}`;
    
    setTimeout(() => {
        banner.classList.add('hidden');
    }, ALERT_TIMEOUT);
}

/**
 * Disable all interactive buttons
 */
function disableButtons() {
    const buttons = document.querySelectorAll('button');
    buttons.forEach(btn => btn.disabled = true);
}

/**
 * Enable all interactive buttons
 */
function enableButtons() {
    const buttons = document.querySelectorAll('button');
    buttons.forEach(btn => btn.disabled = false);
}

// ============================================================================
// Formatting Functions
// ============================================================================

/**
 * Format condition value for display
 */
function formatConditionLabel(condition) {
    const labels = {
        'day': 'Day',
        'hours_of_darkness': 'Hours of Darkness',
        'restricted_visibility': 'Restricted Visibility'
    };
    return labels[condition] || condition;
}

/**
 * Format boat state value for display
 */
function formatStateLabel(state) {
    const labels = {
        'moored': 'Moored',
        'underway_making_way': 'Underway - Making Way',
        'underway_no_way': 'Underway - Making No Way',
        'anchorage': 'Anchorage',
        'nuc_making_way': 'NUC - Making Way',
        'nuc_no_way': 'NUC - Making No Way',
        'towing': 'Towing'
    };
    return labels[state] || state;
}

/**
 * Format signal value for display
 */
function formatSignalLabel(signal) {
    const labels = {
        'turn_starboard': 'Turn Starboard',
        'turn_port': 'Turn Port',
        'astern_propulsion': 'Astern Propulsion',
        'danger_confusion': 'What?',
        'pay_attention': 'Pay Attention',
        'overtake_starboard': 'Overtake Starboard',
        'overtake_port': 'Overtake Port',
        'agreement_overtaken': 'Overtake OK'
    };
    return labels[signal] || signal;
}

/**
 * Format uptime seconds to human-readable format
 */
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m ${secs}s`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

/**
 * Format bytes to human-readable format
 */
function formatBytes(bytes) {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / 1048576).toFixed(1)} MB`;
}

// ============================================================================
// Visibility Change Handling
// ============================================================================

document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        stopPolling();
    } else {
        startPolling();
        fetchStatus();
    }
});

// ============================================================================
// Error Handling
// ============================================================================

window.addEventListener('error', (event) => {
    console.error('Global error:', event.error);
    showAlert('An unexpected error occurred', 'error');
});

window.addEventListener('unhandledrejection', (event) => {
    console.error('Unhandled promise rejection:', event.reason);
    showAlert('An unexpected error occurred', 'error');
});
