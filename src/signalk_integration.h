/**
 * @file signalk_integration.h
 * @brief SignalK integration for Navigation Lights ECU
 * 
 * Implements bidirectional communication:
 * - Inputs: Receive condition, boat state, signal commands from SignalK server
 * - Outputs: Publish current state, countdown, mute status to SignalK server
 * 
 * SignalK Paths (following official specification):
 * - electrical.switches.navigationLights.condition - enum: day/hours_of_darkness/restricted_visibility
 * - electrical.switches.navigationLights.boatState - enum: moored/underway_making_way/underway_no_way/anchorage/nuc_making_way/nuc_no_way
 * - electrical.switches.navigationLights.periodicMuted - boolean
 * - electrical.switches.navigationLights.periodicCountdown - number (seconds)
 * - electrical.switches.navigationLights.lights.* - boolean per light
 * - electrical.switches.navigationLights.horn.active - boolean
 * - electrical.switches.navigationLights.adHocSignal - enum for triggering signals
 * - electrical.switches.navigationLights.emergencyStop - boolean (write-only trigger)
 */

#ifndef SIGNALK_INTEGRATION_H
#define SIGNALK_INTEGRATION_H

#include "sensesp_app.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "NavigationLightsECU.h"

// SignalK path prefix
const char* SK_PATH_PREFIX = "electrical.switches.navigationLights";

/**
 * @brief Setup SignalK inputs and outputs for NavigationLightsECU
 * Call this from main setup after SensESP app initialization
 */
void setupSignalK(NavigationLightsECU& ecu);

/**
 * @brief Helper: Convert Condition enum to SignalK string
 */
const char* conditionToString(Condition condition);

/**
 * @brief Helper: Convert SignalK string to Condition enum
 */
Condition stringToCondition(const char* str);

/**
 * @brief Helper: Convert BoatState enum to SignalK string
 */
const char* boatStateToString(BoatState state);

/**
 * @brief Helper: Convert SignalK string to BoatState enum
 */
BoatState stringToBoatState(const char* str);

/**
 * @brief Helper: Convert AdHocSignal enum to SignalK string
 */
const char* adHocSignalToString(AdHocSignal signal);

/**
 * @brief Helper: Convert SignalK string to AdHocSignal enum
 */
AdHocSignal stringToAdHocSignal(const char* str);

#endif // SIGNALK_INTEGRATION_H
