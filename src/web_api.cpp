/**
 * @file web_api.cpp
 * @brief HTTP REST API implementation for ESP-IDF HTTP server
 * 
 * Provides RESTful JSON API for web UI control using SensESP 3.x HTTP server.
 */

#include "web_api.h"
#include "signalk_integration.h"  // For string conversion functions
#include "sensesp_app.h"  // For sensesp_app global
#include "sensesp/ui/ui_controls.h"  // For CheckboxConfig
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_http_server.h>

using namespace sensesp;

// Global ECU pointer (set during setupWebAPI)
static NavigationLightsECU* g_ecu = nullptr;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Send JSON response
 */
esp_err_t sendJsonResponse(httpd_req_t* req, int status_code, const String& json) {
    const char* status_str;
    switch (status_code) {
        case 200: status_str = "200 OK"; break;
        case 400: status_str = "400 Bad Request"; break;
        case 500: status_str = "500 Internal Server Error"; break;
        default: status_str = "200 OK"; break;
    }
    
    httpd_resp_set_status(req, status_str);
    httpd_resp_set_type(req, "application/json");
    
    // Set Content-Length explicitly
    char content_length[16];
    snprintf(content_length, sizeof(content_length), "%d", json.length());
    httpd_resp_set_hdr(req, "Content-Length", content_length);
    
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

/**
 * @brief Send JSON error response
 */
esp_err_t sendJsonError(httpd_req_t* req, int code, const char* message) {
    StaticJsonDocument<128> doc;
    doc["success"] = false;
    doc["error"] = message;
    doc["code"] = code;
    
    String response;
    serializeJson(doc, response);
    
    return sendJsonResponse(req, code, response);
}

/**
 * @brief Check if SignalK is connected
 */
bool isSignalKConnected() {
    if (!sensesp_app) {
        return false;
    }
    auto ws_client = sensesp_app->get_ws_client();
    if (!ws_client) {
        return false;
    }
    return ws_client->is_connected();
}

/**
 * @brief Read and parse JSON request body
 */
bool readJsonBody(httpd_req_t* req, JsonDocument& doc) {
    // Get content length
    size_t content_len = req->content_len;
    if (content_len == 0 || content_len > 512) {
        return false;
    }
    
    // Read body
    char* buf = new char[content_len + 1];
    int ret = httpd_req_recv(req, buf, content_len);
    if (ret <= 0) {
        delete[] buf;
        return false;
    }
    buf[ret] = '\0';
    
    // Parse JSON
    DeserializationError error = deserializeJson(doc, buf, ret);
    delete[] buf;
    
    return !error;
}

// ============================================================================
// GET Endpoint Handlers
// ============================================================================

/**
 * @brief GET /api/status - Return complete ECU state
 */
esp_err_t handleGetStatus(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    StaticJsonDocument<768> doc;
    
    // Current state
    doc["condition"] = sk_conditionToString(g_ecu->getCondition());
    doc["boatState"] = sk_boatStateToString(g_ecu->getBoatState());
    doc["periodicMuted"] = g_ecu->isPeriodicMuted();
    doc["periodicCountdown"] = g_ecu->getPeriodicCountdownSeconds();
    
    // Light status
    LightConfiguration lights = g_ecu->getCurrentLights();
    JsonObject lightsObj = doc.createNestedObject("lights");
    lightsObj["masthead"] = lights.masthead_light;
    lightsObj["portSidelight"] = lights.port_sidelight;
    lightsObj["starboardSidelight"] = lights.starboard_sidelight;
    lightsObj["sternlight"] = lights.sternlight;
    lightsObj["allroundWhite"] = lights.allround_white;
    lightsObj["allroundRedUpper"] = lights.allround_red_upper;
    lightsObj["allroundRedLower"] = lights.allround_red_lower;
    lightsObj["yellowTowingLight"] = lights.yellow_towing_light;
    
    // Horn status
    JsonObject hornObj = doc.createNestedObject("horn");
    hornObj["active"] = g_ecu->isHornActive();
    
    // SOS signal status
    doc["sosActive"] = g_ecu->isSosActive();
    
    // System status
    doc["signalkConnected"] = isSignalKConnected();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Hardware capabilities (runtime configuration from SensESP web UI)
    // Read current values from config objects (they update when user changes via web UI)
    JsonObject capsObj = doc.createNestedObject("capabilities");
    extern std::shared_ptr<CheckboxConfig> g_config_has_nuc;
    extern std::shared_ptr<CheckboxConfig> g_config_has_towing;
    capsObj["hasNucLights"] = g_config_has_nuc ? g_config_has_nuc->get_value() : true;
    capsObj["hasTowingLights"] = g_config_has_towing ? g_config_has_towing->get_value() : true;
    
    String response;
    serializeJson(doc, response);
    return sendJsonResponse(req, 200, response);
}

/**
 * @brief GET /api/health - Health check endpoint
 */
esp_err_t handleGetHealth(httpd_req_t* req) {
    StaticJsonDocument<256> doc;
    
    doc["healthy"] = true;
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["wifiRSSI"] = WiFi.RSSI();
    doc["signalkConnected"] = isSignalKConnected();
    
    String response;
    serializeJson(doc, response);
    return sendJsonResponse(req, 200, response);
}

// ============================================================================
// POST Endpoint Handlers
// ============================================================================

/**
 * @brief POST /api/condition - Set sailing condition
 */
esp_err_t handlePostCondition(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    StaticJsonDocument<128> doc;
    if (!readJsonBody(req, doc)) {
        return sendJsonError(req, 400, "Invalid JSON");
    }
    
    if (!doc.containsKey("value")) {
        return sendJsonError(req, 400, "Missing 'value' field");
    }
    
    const char* valueStr = doc["value"];
    Condition condition = sk_stringToCondition(valueStr);
    
    // Validate
    if (strcmp(valueStr, "day") != 0 && 
        strcmp(valueStr, "hours_of_darkness") != 0 && 
        strcmp(valueStr, "restricted_visibility") != 0) {
        return sendJsonError(req, 400, "Invalid condition value");
    }
    
    g_ecu->setCondition(condition);
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["condition"] = sk_conditionToString(condition);
    
    String responseStr;
    serializeJson(response, responseStr);
    return sendJsonResponse(req, 200, responseStr);
}

/**
 * @brief POST /api/state - Set boat state
 */
esp_err_t handlePostState(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    StaticJsonDocument<128> doc;
    if (!readJsonBody(req, doc)) {
        return sendJsonError(req, 400, "Invalid JSON");
    }
    
    if (!doc.containsKey("value")) {
        return sendJsonError(req, 400, "Missing 'value' field");
    }
    
    const char* valueStr = doc["value"];
    BoatState state = sk_stringToBoatState(valueStr);
    
    // Validate
    if (strcmp(valueStr, "moored") != 0 && 
        strcmp(valueStr, "underway_making_way") != 0 && 
        strcmp(valueStr, "underway_no_way") != 0 && 
        strcmp(valueStr, "anchorage") != 0 && 
        strcmp(valueStr, "nuc_making_way") != 0 && 
        strcmp(valueStr, "nuc_no_way") != 0 && 
        strcmp(valueStr, "towing") != 0) {
        return sendJsonError(req, 400, "Invalid boat state value");
    }
    
    g_ecu->setBoatState(state);
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["boatState"] = sk_boatStateToString(state);
    
    String responseStr;
    serializeJson(response, responseStr);
    return sendJsonResponse(req, 200, responseStr);
}

/**
 * @brief POST /api/mute - Toggle periodic signal mute
 */
esp_err_t handlePostMute(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    StaticJsonDocument<128> doc;
    if (!readJsonBody(req, doc)) {
        return sendJsonError(req, 400, "Invalid JSON");
    }
    
    if (!doc.containsKey("muted")) {
        return sendJsonError(req, 400, "Missing 'muted' field");
    }
    
    bool muted = doc["muted"];
    
    if (muted) {
        g_ecu->mutePeriodicSignals();
    } else {
        g_ecu->unmutePeriodicSignals();
    }
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["periodicMuted"] = g_ecu->isPeriodicMuted();
    
    String responseStr;
    serializeJson(response, responseStr);
    return sendJsonResponse(req, 200, responseStr);
}

/**
 * @brief POST /api/signal - Trigger ad-hoc signal
 */
esp_err_t handlePostSignal(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    StaticJsonDocument<128> doc;
    if (!readJsonBody(req, doc)) {
        return sendJsonError(req, 400, "Invalid JSON");
    }
    
    if (!doc.containsKey("signal")) {
        return sendJsonError(req, 400, "Missing 'signal' field");
    }
    
    const char* signalStr = doc["signal"];
    AdHocSignal signal = sk_stringToAdHocSignal(signalStr);
    
    // Validate: sk_stringToAdHocSignal returns TURN_STARBOARD on invalid input
    // Check if conversion matches original string
    String converted = sk_adHocSignalToString(signal);
    if (converted != String(signalStr)) {
        return sendJsonError(req, 400, "Invalid signal value");
    }
    
    g_ecu->triggerAdHocSignal(signal);
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["triggered"] = signalStr;
    
    String responseStr;
    serializeJson(response, responseStr);
    return sendJsonResponse(req, 200, responseStr);
}

/**
 * @brief POST /api/emergency - Emergency stop all outputs
 */
esp_err_t handlePostEmergency(httpd_req_t* req) {
    if (!g_ecu) {
        return sendJsonError(req, 500, "ECU not initialized");
    }
    
    g_ecu->emergencyStop();
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["stopped"] = true;
    
    String responseStr;
    serializeJson(response, responseStr);
    return sendJsonResponse(req, 200, responseStr);
}

// ============================================================================
// Setup Function
// ============================================================================

/**
 * @brief Initialize web API endpoints
 */
void setupWebAPI(HTTPServer* server, NavigationLightsECU* ecu) {
    // Store ECU reference
    g_ecu = ecu;
    
    Serial.println("Setting up Web API endpoints...");
    
    // Register GET endpoints
    auto status_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_GET, "/api/status", handleGetStatus);
    server->add_handler(status_handler);
    
    auto health_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_GET, "/api/health", handleGetHealth);
    server->add_handler(health_handler);
    
    // Register POST endpoints
    auto condition_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_POST, "/api/condition", handlePostCondition);
    server->add_handler(condition_handler);
    
    auto state_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_POST, "/api/state", handlePostState);
    server->add_handler(state_handler);
    
    auto mute_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_POST, "/api/mute", handlePostMute);
    server->add_handler(mute_handler);
    
    auto signal_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_POST, "/api/signal", handlePostSignal);
    server->add_handler(signal_handler);
    
    auto emergency_handler = std::make_shared<HTTPRequestHandler>(
        1 << HTTP_POST, "/api/emergency", handlePostEmergency);
    server->add_handler(emergency_handler);
    
    Serial.println("Web API endpoints registered:");
    Serial.println("  GET  /api/status");
    Serial.println("  GET  /api/health");
    Serial.println("  POST /api/condition");
    Serial.println("  POST /api/state");
    Serial.println("  POST /api/mute");
    Serial.println("  POST /api/signal");
    Serial.println("  POST /api/emergency");
    Serial.println();
    Serial.println("Frontend UI available at:");
    Serial.println("  http://nav-lights-ecu.local/lights");
    Serial.println("  http://nav-lights-ecu.local/lights.html");
}

/**
 * @brief Redirect handler for /lights -> /lights.html  
 */
esp_err_t handleLightsRedirect(httpd_req_t* req) {
    httpd_resp_set_status(req, "301 Moved Permanently");
    httpd_resp_set_hdr(req, "Location", "/lights.html");
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

/**
 * @brief Static file handler for SPIFFS
 */
esp_err_t handleStaticFile(httpd_req_t* req) {
    String path = String(req->uri);
    
    // Security: prevent directory traversal
    if (path.indexOf("..") >= 0) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }
    
    // Open file from SPIFFS (operates on filesystem directly, not through VFS mount point)
    File file = SPIFFS.open(path, "r");
    if (!file) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }
    
    // Set content type based on extension
    const char* content_type = "text/plain";
    if (path.endsWith(".html")) content_type = "text/html";
    else if (path.endsWith(".css")) content_type = "text/css";
    else if (path.endsWith(".js")) content_type = "application/javascript";
    else if (path.endsWith(".json")) content_type = "application/json";
    else if (path.endsWith(".svg")) content_type = "image/svg+xml";
    else if (path.endsWith(".png")) content_type = "image/png";
    else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) content_type = "image/jpeg";
    else if (path.endsWith(".gif")) content_type = "image/gif";
    
    httpd_resp_set_type(req, content_type);
    
    // Disable caching for development - always fetch fresh files
    httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    
    // Send file in chunks
    const size_t chunk_size = 1024;
    uint8_t buffer[chunk_size];
    while (file.available()) {
        size_t bytes_read = file.read(buffer, chunk_size);
        if (httpd_resp_send_chunk(req, (const char*)buffer, bytes_read) != ESP_OK) {
            file.close();
            return ESP_FAIL;
        }
    }
    
    file.close();
    httpd_resp_send_chunk(req, nullptr, 0); // Signal end
    return ESP_OK;
}

/**
 * @brief Setup static file serving from SPIFFS
 */
void setupStaticFiles(sensesp::HTTPServer* server) {
    Serial.println("Registering static file handlers...");
    
    // Register redirect from /lights to /lights.html
    auto lights_redirect_handler = std::make_shared<sensesp::HTTPRequestHandler>(
        1 << HTTP_GET, "/lights", handleLightsRedirect);
    server->add_handler(lights_redirect_handler);
    
    // Register handlers for web UI files
    auto lights_html_handler = std::make_shared<sensesp::HTTPRequestHandler>(
        1 << HTTP_GET, "/lights.html", handleStaticFile);
    server->add_handler(lights_html_handler);
    
    auto lights_css_handler = std::make_shared<sensesp::HTTPRequestHandler>(
        1 << HTTP_GET, "/lights.css", handleStaticFile);
    server->add_handler(lights_css_handler);
    
    auto lights_js_handler = std::make_shared<sensesp::HTTPRequestHandler>(
        1 << HTTP_GET, "/lights.js", handleStaticFile);
    server->add_handler(lights_js_handler);
    
    auto logo_handler = std::make_shared<sensesp::HTTPRequestHandler>(
        1 << HTTP_GET, "/iha-logo.png", handleStaticFile);
    server->add_handler(logo_handler);
    
    Serial.println("Static file handlers registered:");
    Serial.println("  GET  /lights (redirect)");
    Serial.println("  GET  /lights.html");
    Serial.println("  GET  /lights.css");
    Serial.println("  GET  /lights.js");
    Serial.println("  GET  /iha-logo.png");
}
