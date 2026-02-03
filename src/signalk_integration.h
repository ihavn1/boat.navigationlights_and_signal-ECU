/**
 * @file signalk_integration.h
 * @brief SignalK integration for Navigation Lights ECU (SensESP v3 API)
 * 
 * Implements bidirectional communication:
 * - Inputs: Receive condition, boat state, signal commands from SignalK server via PUT requests
 * - Outputs: Publish current state, countdown, mute status to SignalK server
 * 
 * SignalK Paths (following official specification):
 * - electrical.switches.navigationLights.condition - String: day/hours_of_darkness/restricted_visibility
 * - electrical.switches.navigationLights.boatState - String: moored/underway_making_way/underway_no_way/anchorage/nuc_making_way/nuc_no_way
 * - electrical.switches.navigationLights.periodicMuted - bool
 * - electrical.switches.navigationLights.periodicCountdown - int (seconds)
 * - electrical.switches.navigationLights.lights.* - bool per light
 * - electrical.switches.navigationLights.horn.active - bool
 * - electrical.switches.navigationLights.adHocSignal - String for triggering signals
 * - electrical.switches.navigationLights.emergencyStop - bool (write-only trigger)
 */

#ifndef SIGNALK_INTEGRATION_H
#define SIGNALK_INTEGRATION_H

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "NavigationLightsECU.h"

// SignalK path prefix
#define SK_PATH_PREFIX "electrical.switches.navigationLights"

/**
 * @brief Setup SignalK inputs and outputs for NavigationLightsECU
 * Call this from main setup after SensESP app initialization
 * @param ecu Reference to the NavigationLightsECU instance
 */
void setupSignalK(NavigationLightsECU& ecu);

#endif // SIGNALK_INTEGRATION_H
