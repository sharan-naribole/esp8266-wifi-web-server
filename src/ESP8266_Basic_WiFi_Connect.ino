/**
 ******************************************************************************
 * @file           : ESP8266_Basic_WiFi_Connect.ino
 * @brief          : ESP8266 WiFi Connection and HTTP Server Test
 ******************************************************************************
 * @description
 * This is a simple test sketch for the NodeMCU ESP8266 that demonstrates:
 * - WiFi connection to your home network
 * - Basic HTTP web server on port 80
 * - Simple HTML interface with interactive buttons
 * - LED control on the built-in LED (GPIO2/D4)
 *
 * Purpose:
 * This is a standalone test to verify your ESP8266 WiFi setup before
 * integrating it with STM32 or other complex projects.
 *
 * Hardware Required:
 * - NodeMCU ESP8266 board
 * - USB cable for power and programming
 * - Built-in LED on GPIO2 (D4) - already on the board!
 *
 * Web Interface:
 * - Homepage:     http://esp8266-wifi.local/  (or http://ESP8266_IP/)
 * - LED ON:       http://esp8266-wifi.local/led?state=on
 * - LED OFF:      http://esp8266-wifi.local/led?state=off
 * - LED Toggle:   http://esp8266-wifi.local/led?state=toggle
 *
 * @author  Sharan Naribole
 * @date    December 2025
 ******************************************************************************
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "index.h"  // HTML web interface

// ========================================
// Configuration Section - CHANGE THESE!
// ========================================

/**
 * @brief WiFi network credentials
 * @note Replace with your home WiFi details
 * @important ESP8266 only supports 2.4GHz WiFi (NOT 5GHz)
 */
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi network name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password

/**
 * @brief Web server configuration
 */
const int HTTP_PORT = 80;                        // Standard HTTP port

/**
 * @brief Built-in LED configuration
 * @note NodeMCU has built-in LED on GPIO2 (D4)
 * @note LED is ACTIVE LOW (LOW = ON, HIGH = OFF)
 */
const int BUILTIN_LED_PIN = 2;  // GPIO2 = D4 on NodeMCU
const bool LED_ACTIVE_LOW = true;

// ========================================
// Client Request Tracking Structure
// ========================================

/**
 * @struct ClientRequest
 * @brief Structure to store information about HTTP client requests
 *
 * This structure captures metadata about each HTTP request received by the
 * web server, enabling multi-device tracking and request history logging.
 */
struct ClientRequest {
  /**
   * @brief IP address of the client device
   * @details IPv4 address in dotted decimal notation (e.g., "192.168.1.100")
   */
  String ip;

  /**
   * @brief User-Agent string from HTTP header
   * @details Contains browser and OS information (e.g., "Mozilla/5.0...")
   *          Helps identify the device type (iPhone, laptop, etc.)
   */
  String userAgent;

  /**
   * @brief HTTP endpoint/URI that was accessed
   * @details The requested path (e.g., "/led?state=on", "/status", "/")
   */
  String endpoint;

  /**
   * @brief Timestamp when request was received
   * @details Value from millis() function, represents milliseconds since boot
   */
  unsigned long timestamp;
};

// ========================================
// Global Variables
// ========================================

ESP8266WebServer server(HTTP_PORT);
bool ledState = false;  // Track current LED state

/**
 * @brief Maximum number of recent requests to track
 * @details Circular buffer size for client request history
 */
#define MAX_REQUESTS 10

/**
 * @brief Circular buffer storing recent client requests
 * @details Stores up to MAX_REQUESTS most recent HTTP requests with metadata
 */
ClientRequest recentRequests[MAX_REQUESTS];

/**
 * @brief Current index in the circular buffer
 * @details Points to where the next request will be stored (0 to MAX_REQUESTS-1)
 */
int requestIndex = 0;

/**
 * @brief Total number of requests received since boot
 * @details Counter increments on every HTTP request, never resets
 */
unsigned long totalRequests = 0;

// ========================================
// Function Declarations
// ========================================

void setupWiFi();
void setupWebServer();
void handleRoot();
void handleLED();
void handleStatus();
void handleClients();
void handleNotFound();
void setLED(bool state);
void toggleLED();
void logRequest(String endpoint);

// ========================================
// Setup Function (Runs Once)
// ========================================

void setup() {
  // Initialize built-in LED
  pinMode(BUILTIN_LED_PIN, OUTPUT);
  setLED(false);  // LED OFF initially

  // Initialize Serial for debug output
  Serial.begin(115200);
  delay(100);

  // Print startup banner
  Serial.println("\r\n\r\n");
  Serial.println("========================================");
  Serial.println("  ESP8266 WiFi Connection Test");
  Serial.println("  NodeMCU Web Server Demo");
  Serial.println("========================================");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.println("========================================");

  // Connect to WiFi network
  setupWiFi();

  // Configure and start web server
  setupWebServer();

  Serial.println("========================================");
  Serial.println("  System Ready!");
  Serial.println("  Open the IP address in your browser");
  Serial.println("========================================\n");
}

// ========================================
// Main Loop (Runs Forever)
// ========================================

void loop() {
  // Handle incoming HTTP requests
  server.handleClient();

  // Update mDNS
  MDNS.update();

  // Optional: WiFi connection monitoring
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) {  // Check every 30 seconds
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[WIFI] Connection lost! Reconnecting...");
      setupWiFi();
    } else {
      // Print status update
      Serial.print("[WIFI] Connected | IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(" | Signal: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    }
    lastCheck = millis();
  }
}

// ========================================
// WiFi Setup Function
// ========================================

void setupWiFi() {
  Serial.println("\n[WIFI] Starting WiFi connection...");
  Serial.print("[WIFI] SSID: ");
  Serial.println(WIFI_SSID);

  // Disconnect any previous connection
  WiFi.disconnect();
  delay(100);

  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);

  // Set hostname (optional - makes it easier to find on network)
  WiFi.hostname("ESP8266-Test");

  // Begin WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait for connection with timeout
  Serial.print("[WIFI] Connecting");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;

    // Blink LED while connecting
    toggleLED();
  }
  Serial.println();

  // Check connection status
  if (WiFi.status() == WL_CONNECTED) {
    setLED(false);  // Turn off LED when connected

    Serial.println("[WIFI] ✓ Connected successfully!");
    Serial.println("[WIFI] --------------------------------");
    Serial.print("[WIFI] IP Address:  ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("[WIFI] Gateway:     ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("[WIFI] Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("[WIFI] Signal:      ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("[WIFI] --------------------------------");

    // Start mDNS responder
    Serial.println("[mDNS] Starting mDNS responder...");
    if (MDNS.begin("esp8266-wifi")) {
      Serial.println("[mDNS] ✓ mDNS responder started");
      Serial.println("[mDNS] Access at: http://esp8266-wifi.local/");
      Serial.println("[mDNS] --------------------------------");
    } else {
      Serial.println("[mDNS] ✗ Error starting mDNS responder");
    }
  } else {
    Serial.println("[WIFI] ✗ Connection Failed!");
    Serial.println("[WIFI] Troubleshooting:");
    Serial.println("[WIFI] 1. Check WiFi credentials");
    Serial.println("[WIFI] 2. Ensure 2.4GHz WiFi (NOT 5GHz)");
    Serial.println("[WIFI] 3. Check if router is nearby");
    Serial.println("[WIFI] 4. Verify router allows new devices");

    // Blink LED rapidly on error
    for(int i = 0; i < 10; i++) {
      toggleLED();
      delay(100);
    }
  }
}

// ========================================
// Web Server Setup Function
// ========================================

void setupWebServer() {
  Serial.println("\n[SERVER] Configuring web server...");

  // Register URL handlers
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_GET, handleLED);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/clients", HTTP_GET, handleClients);
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();

  Serial.println("[SERVER] ✓ Web server started");
  Serial.println("[SERVER] --------------------------------");
  Serial.println("[SERVER] Available endpoints:");
  Serial.print("[SERVER] Homepage:    http://");
  Serial.println(WiFi.localIP());
  Serial.print("[SERVER] LED Control: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/led?state=on");
  Serial.print("[SERVER] Status:      http://");
  Serial.print(WiFi.localIP());
  Serial.println("/status");
  Serial.print("[SERVER] Clients:     http://");
  Serial.print(WiFi.localIP());
  Serial.println("/clients");
  Serial.println("[SERVER] --------------------------------");
}

// ========================================
// Handler: Serve Main HTML Page
// ========================================

void handleRoot() {
  Serial.println("\n[HTTP] GET / - Serving homepage");
  server.send_P(200, "text/html", INDEX_HTML);
}

// ========================================
// Handler: LED Control
// ========================================

void handleLED() {
  // Log the request
  String endpoint = "/led";
  if (server.hasArg("state")) {
    endpoint += "?state=" + server.arg("state");
  }
  logRequest(endpoint);

  // Check if state parameter exists
  if (!server.hasArg("state")) {
    Serial.println("[HTTP] GET /led - ERROR: Missing parameter");
    server.send(400, "text/plain", "ERROR: Missing 'state' parameter\nUsage: /led?state=on|off|toggle");
    return;
  }

  // Get state parameter
  String state = server.arg("state");
  state.toLowerCase();  // Convert to lowercase

  Serial.print("[HTTP] GET /led?state=");
  Serial.println(state);

  // Process command
  String response;
  if (state == "on") {
    setLED(true);
    response = "LED turned ON";
  }
  else if (state == "off") {
    setLED(false);
    response = "LED turned OFF";
  }
  else if (state == "toggle") {
    toggleLED();
    response = ledState ? "LED toggled ON" : "LED toggled OFF";
  }
  else {
    Serial.println("[HTTP] ERROR: Invalid state");
    server.send(400, "text/plain", "ERROR: Invalid state\nMust be: on, off, or toggle");
    return;
  }

  // Send success response
  Serial.print("[HTTP] Response: ");
  Serial.println(response);
  server.send(200, "text/plain", response);
}

// ========================================
// Handler: System Status (JSON)
// ========================================

void handleStatus() {
  // Build JSON response
  String json = "{\n";
  json += "  \"wifi\": {\n";
  json += "    \"connected\": " + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",\n";
  json += "    \"ssid\": \"" + String(WIFI_SSID) + "\",\n";
  json += "    \"ip\": \"" + WiFi.localIP().toString() + "\",\n";
  json += "    \"mac\": \"" + WiFi.macAddress() + "\",\n";
  json += "    \"rssi\": " + String(WiFi.RSSI()) + "\n";
  json += "  },\n";
  json += "  \"system\": {\n";
  json += "    \"chipId\": \"0x" + String(ESP.getChipId(), HEX) + "\",\n";
  json += "    \"freeHeap\": " + String(ESP.getFreeHeap()) + ",\n";
  json += "    \"uptime\": " + String(millis() / 1000) + "\n";
  json += "  },\n";
  json += "  \"led\": {\n";
  json += "    \"state\": \"" + String(ledState ? "ON" : "OFF") + "\"\n";
  json += "  }\n";
  json += "}\n";

  server.send(200, "application/json", json);
}

// ========================================
// Handler: Client Request History (JSON)
// ========================================

/**
 * @brief Handle /clients endpoint - Returns recent client request history
 * @details Returns JSON array of recent HTTP requests with client metadata
 */
void handleClients() {
  // Build JSON response
  String json = "{\n";
  json += "  \"totalRequests\": " + String(totalRequests) + ",\n";
  json += "  \"recentRequests\": [\n";

  // Add recent requests (newest first)
  int count = 0;
  for (int i = 0; i < MAX_REQUESTS; i++) {
    // Calculate index to read from newest to oldest
    int idx = (requestIndex - 1 - i + MAX_REQUESTS) % MAX_REQUESTS;

    // Skip empty entries
    if (recentRequests[idx].ip.length() == 0) continue;

    if (count > 0) json += ",\n";

    json += "    {\n";
    json += "      \"ip\": \"" + recentRequests[idx].ip + "\",\n";
    json += "      \"userAgent\": \"" + recentRequests[idx].userAgent + "\",\n";
    json += "      \"endpoint\": \"" + recentRequests[idx].endpoint + "\",\n";
    json += "      \"timestamp\": " + String(recentRequests[idx].timestamp) + ",\n";
    json += "      \"uptime\": \"" + String(recentRequests[idx].timestamp / 1000) + "s\"\n";
    json += "    }";

    count++;
  }

  json += "\n  ]\n";
  json += "}\n";

  server.send(200, "application/json", json);
}

// ========================================
// Handler: 404 Not Found
// ========================================

void handleNotFound() {
  String message = "404: Page Not Found\n\n";
  message += "URI: " + server.uri() + "\n";
  message += "Method: " + String((server.method() == HTTP_GET) ? "GET" : "POST") + "\n";
  message += "\nAvailable endpoints:\n";
  message += "  /          - Homepage\n";
  message += "  /led       - LED control (state=on|off|toggle)\n";
  message += "  /status    - System status (JSON)\n";
  message += "  /clients   - Client request history (JSON)\n";

  Serial.println("[HTTP] 404 - " + server.uri());
  server.send(404, "text/plain", message);
}

// ========================================
// LED Control Functions
// ========================================

/**
 * @brief Set LED to specific state
 * @param state true = ON, false = OFF
 */
void setLED(bool state) {
  ledState = state;

  // Account for active-low LED
  if (LED_ACTIVE_LOW) {
    digitalWrite(BUILTIN_LED_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(BUILTIN_LED_PIN, state ? HIGH : LOW);
  }

  Serial.print("[LED] State: ");
  Serial.println(state ? "ON" : "OFF");
}

/**
 * @brief Toggle LED state
 */
void toggleLED() {
  setLED(!ledState);
}

// ========================================
// Client Request Logging Function
// ========================================

/**
 * @brief Log client request metadata
 * @param endpoint The HTTP endpoint/URI that was accessed
 * @details Captures client IP, User-Agent, endpoint, and timestamp in circular buffer
 *          Also logs to Serial Monitor with formatted output
 */
void logRequest(String endpoint) {
  // Get client IP
  String clientIP = server.client().remoteIP().toString();

  // Get User-Agent header
  String userAgent = "Unknown";
  if (server.hasHeader("User-Agent")) {
    userAgent = server.header("User-Agent");
  }

  // Store in circular buffer
  recentRequests[requestIndex].ip = clientIP;
  recentRequests[requestIndex].userAgent = userAgent;
  recentRequests[requestIndex].endpoint = endpoint;
  recentRequests[requestIndex].timestamp = millis();

  // Increment counters
  requestIndex = (requestIndex + 1) % MAX_REQUESTS;
  totalRequests++;

  // Log to Serial Monitor
  Serial.println("[CLIENT] --------------------------------");
  Serial.print("[CLIENT] IP: ");
  Serial.println(clientIP);
  Serial.print("[CLIENT] User-Agent: ");
  Serial.println(userAgent);
  Serial.print("[CLIENT] Device: ");

  // Convert User-Agent to lowercase for case-insensitive matching
  String userAgentLower = userAgent;
  userAgentLower.toLowerCase();

  // Parse User-Agent to identify device type
  if (userAgentLower.indexOf("iphone") >= 0) {
    Serial.print("iPhone");
  } else if (userAgentLower.indexOf("ipad") >= 0) {
    Serial.print("iPad");
  } else if (userAgentLower.indexOf("android") >= 0) {
    Serial.print("Android Device");
  } else if (userAgentLower.indexOf("macintosh") >= 0 || userAgentLower.indexOf("mac os") >= 0) {
    Serial.print("Mac Computer");
  } else if (userAgentLower.indexOf("windows") >= 0 || userAgentLower.indexOf("win64") >= 0 || userAgentLower.indexOf("win32") >= 0) {
    Serial.print("Windows Computer");
  } else if (userAgentLower.indexOf("linux") >= 0 && userAgentLower.indexOf("android") < 0) {
    Serial.print("Linux Computer");
  } else if (userAgentLower.indexOf("curl") >= 0) {
    Serial.print("Command Line (curl)");
  } else if (userAgentLower.indexOf("postman") >= 0) {
    Serial.print("Postman API Client");
  } else {
    Serial.print("Unknown Device");
  }
  Serial.println();

  Serial.print("[CLIENT] Browser: ");
  if (userAgentLower.indexOf("edg") >= 0) {
    Serial.print("Edge");
  } else if (userAgentLower.indexOf("chrome") >= 0 || userAgentLower.indexOf("crios") >= 0) {
    Serial.print("Chrome");
  } else if (userAgentLower.indexOf("firefox") >= 0 || userAgentLower.indexOf("fxios") >= 0) {
    Serial.print("Firefox");
  } else if (userAgentLower.indexOf("safari") >= 0) {
    Serial.print("Safari");
  } else if (userAgentLower.indexOf("opera") >= 0 || userAgentLower.indexOf("opr") >= 0) {
    Serial.print("Opera");
  } else if (userAgentLower.indexOf("curl") >= 0) {
    Serial.print("curl");
  } else {
    Serial.print("Unknown");
  }
  Serial.println();

  Serial.print("[CLIENT] Endpoint: ");
  Serial.println(endpoint);
  Serial.print("[CLIENT] Total Requests: ");
  Serial.println(totalRequests);
  Serial.println("[CLIENT] --------------------------------");
}
