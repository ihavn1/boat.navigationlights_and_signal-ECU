/**
 * @file signalk_integration.h
 * @brief SignalK integration for Navigation Lights ECU (SensESP v3 API)
 * 
 * Implements bidirectional communication:
 * - Inputs: Subscribe to condition, boat state, signal commands from SignalK server (delta updates)
 * - Outputs: Publish current state, countdown, mute status to SignalK server
 * 
 * SignalK Paths (following official specification):
 * 
 * COMMAND PATHS (ECU subscribes to delta updates from SignalK server):
 * - electrical.switches.navigationLights.command.condition - String: day/hours_of_darkness/restricted_visibility
 * - electrical.switches.navigationLights.command.boatState - String: moored/underway_making_way/underway_no_way/anchorage/nuc_making_way/nuc_no_way
 * - electrical.switches.navigationLights.command.periodicMuted - bool
 * - electrical.switches.navigationLights.command.adHocSignal - String for triggering signals
 * - electrical.switches.navigationLights.command.emergencyStop - bool (write-only trigger)
 * 
 * STATUS PATHS (ECU publishes state):
 * - electrical.switches.navigationLights.condition - String: current condition
 * - electrical.switches.navigationLights.boatState - String: current boat state
 * - electrical.switches.navigationLights.periodicMuted - bool: current mute status
 * - electrical.switches.navigationLights.periodicCountdown - int (seconds)
 * - electrical.switches.navigationLights.lights.* - bool per light
 * - electrical.switches.navigationLights.horn.active - bool
 * - electrical.switches.navigationLights.sosActive - bool
 * - electrical.switches.navigationLights.heartbeat - int
 */

#ifndef SIGNALK_INTEGRATION_H
#define SIGNALK_INTEGRATION_H

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "NavigationLightsECU.h"

// SignalK path prefix
#define SK_PATH_PREFIX "electrical.switches.navigationLights"

/**
 * @brief Setup SignalK inputs and outputs for NavigationLightsECU
 * Call this from main setup after SensESP app initialization
 * @param ecu Reference to the NavigationLightsECU instance
 */
void setupSignalK(NavigationLightsECU& ecu);

// String conversion functions (for SignalK and Web API)
const char* sk_conditionToString(Condition condition);
const char* sk_boatStateToString(BoatState state);
const char* sk_adHocSignalToString(AdHocSignal signal);
Condition sk_stringToCondition(const String& str);
BoatState sk_stringToBoatState(const String& str);
AdHocSignal sk_stringToAdHocSignal(const String& str);

#endif // SIGNALK_INTEGRATION_H
