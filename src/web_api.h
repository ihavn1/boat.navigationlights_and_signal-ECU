/**
 * @file web_api.h
 * @brief HTTP REST API for web-based fallback UI
 * 
 * Provides JSON REST endpoints for browser-based control when SignalK unavailable.
 * Runs on SensESP's HTTP server at /api/* paths.
 * 
 * Endpoints:
 * - GET  /api/status     - Complete ECU state
 * - GET  /api/health     - Health check
 * - POST /api/condition  - Set sailing condition
 * - POST /api/state      - Set boat state
 * - POST /api/mute       - Toggle periodic signal mute
 * - POST /api/signal     - Trigger ad-hoc signal
 * - POST /api/emergency  - Emergency stop all outputs
 */

#ifndef WEB_API_H
#define WEB_API_H

#include "sensesp/net/http_server.h"
#include "NavigationLightsECU.h"

/**
 * @brief Initialize and register all web API endpoints
 * @param server SensESP's HTTPServer instance
 * @param ecu Pointer to NavigationLightsECU for state/control
 */
void setupWebAPI(sensesp::HTTPServer* server, NavigationLightsECU* ecu);

#endif // WEB_API_H
