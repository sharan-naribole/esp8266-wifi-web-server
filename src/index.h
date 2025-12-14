/**
 ******************************************************************************
 * @file           : index.h
 * @brief          : HTML Web Interface for ESP8266 Test
 ******************************************************************************
 * @description
 * This header file contains the HTML, CSS, and JavaScript for the web-based
 * LED control interface. The HTML is stored as a C string constant and served
 * by the ESP8266 web server.
 *
 * Features:
 * - Responsive design (mobile-friendly)
 * - Modern gradient UI
 * - Real-time LED control
 * - System status display
 * - AJAX requests for smooth interaction
 *
 * @note The PROGMEM keyword stores the HTML in flash memory instead of RAM,
 *       saving precious SRAM on the ESP8266.
 ******************************************************************************
 */

#ifndef INDEX_H
#define INDEX_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 WiFi Test</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }

    .container {
      background: white;
      border-radius: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      max-width: 500px;
      width: 100%;
      padding: 40px 30px;
    }

    h1 {
      text-align: center;
      color: #333;
      margin-bottom: 10px;
      font-size: 28px;
      font-weight: 700;
    }

    .subtitle {
      text-align: center;
      color: #666;
      margin-bottom: 30px;
      font-size: 14px;
    }

    .status-card {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-radius: 15px;
      padding: 20px;
      margin-bottom: 30px;
      color: white;
      box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
    }

    .status-card h2 {
      font-size: 16px;
      font-weight: 600;
      margin-bottom: 15px;
      opacity: 0.9;
    }

    .status-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.2);
    }

    .status-item:last-child {
      border-bottom: none;
    }

    .status-label {
      font-size: 14px;
      opacity: 0.9;
    }

    .status-value {
      font-weight: 600;
      font-size: 14px;
      font-family: 'Courier New', monospace;
    }

    .led-indicator {
      display: inline-block;
      width: 12px;
      height: 12px;
      border-radius: 50%;
      margin-left: 8px;
      box-shadow: 0 0 10px currentColor;
    }

    .led-on {
      background: #4ade80;
      animation: pulse 1.5s infinite;
    }

    .led-off {
      background: #64748b;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }

    .button-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 15px;
      margin-bottom: 20px;
    }

    button {
      padding: 20px;
      font-size: 16px;
      font-weight: 600;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      transition: all 0.3s ease;
      color: white;
      box-shadow: 0 4px 15px rgba(0,0,0,0.2);
    }

    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 20px rgba(0,0,0,0.3);
    }

    button:active {
      transform: translateY(0);
    }

    button:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none !important;
    }

    .btn-on {
      background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
    }

    .btn-off {
      background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
    }

    .btn-toggle {
      background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
      grid-column: 1 / -1;
    }

    .btn-refresh {
      background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
      grid-column: 1 / -1;
      padding: 15px;
      font-size: 14px;
    }

    #message {
      text-align: center;
      margin-top: 20px;
      padding: 15px;
      border-radius: 10px;
      font-weight: 600;
      display: none;
      animation: fadeIn 0.3s ease;
    }

    @keyframes fadeIn {
      from {
        opacity: 0;
        transform: translateY(-10px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }

    .success {
      background: #d4edda;
      color: #155724;
      border: 2px solid #c3e6cb;
    }

    .error {
      background: #f8d7da;
      color: #721c24;
      border: 2px solid #f5c6cb;
    }

    .info-box {
      background: #e7f3ff;
      border-left: 4px solid #2196F3;
      padding: 15px;
      margin-top: 20px;
      border-radius: 8px;
      font-size: 13px;
      color: #0c5460;
      line-height: 1.6;
    }

    .info-box strong {
      display: block;
      margin-bottom: 5px;
      color: #084298;
    }

    .icon {
      margin-right: 8px;
    }

    .clients-card {
      background: #f8fafc;
      border-radius: 15px;
      padding: 20px;
      margin-bottom: 20px;
      border: 2px solid #e2e8f0;
    }

    .clients-card h2 {
      font-size: 16px;
      font-weight: 600;
      margin-bottom: 15px;
      color: #1e293b;
    }

    .client-item {
      background: white;
      border-radius: 10px;
      padding: 12px;
      margin-bottom: 10px;
      border: 1px solid #e2e8f0;
      font-size: 12px;
    }

    .client-item:last-child {
      margin-bottom: 0;
    }

    .client-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 8px;
    }

    .client-ip {
      font-weight: 600;
      color: #667eea;
      font-family: 'Courier New', monospace;
    }

    .client-time {
      font-size: 11px;
      color: #64748b;
    }

    .client-detail {
      color: #475569;
      margin: 4px 0;
      line-height: 1.4;
    }

    .client-detail strong {
      color: #1e293b;
    }

    .no-clients {
      text-align: center;
      color: #64748b;
      padding: 20px;
      font-style: italic;
    }

    .total-requests {
      text-align: center;
      font-size: 13px;
      color: #64748b;
      margin-top: 10px;
      padding-top: 10px;
      border-top: 1px solid #e2e8f0;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP8266 WiFi Test</h1>
    <p class="subtitle">NodeMCU Web Server Demo</p>

    <div class="status-card">
      <h2>System Status</h2>
      <div class="status-item">
        <span class="status-label">LED Status</span>
        <span class="status-value">
          <span id="ledStatus">Unknown</span>
          <span class="led-indicator led-off" id="ledIndicator"></span>
        </span>
      </div>
      <div class="status-item">
        <span class="status-label">WiFi Signal</span>
        <span class="status-value" id="wifiSignal">Loading...</span>
      </div>
      <div class="status-item">
        <span class="status-label">Uptime</span>
        <span class="status-value" id="uptime">Loading...</span>
      </div>
    </div>

    <div class="clients-card">
      <h2>Recent Client Requests</h2>
      <div id="clientsList">
        <div class="no-clients">Loading client data...</div>
      </div>
      <div class="total-requests" id="totalRequests"></div>
    </div>

    <div class="button-grid">
      <button class="btn-on" onclick="controlLED('on')">
        <span class="icon">💡</span>LED ON
      </button>
      <button class="btn-off" onclick="controlLED('off')">
        <span class="icon">🌙</span>LED OFF
      </button>
      <button class="btn-toggle" onclick="controlLED('toggle')">
        <span class="icon">🔄</span>Toggle LED
      </button>
      <button class="btn-refresh" onclick="updateStatus()">
        <span class="icon">🔄</span>Refresh Status
      </button>
    </div>

    <div id="message"></div>

    <div class="info-box">
      <strong>Test Successful!</strong>
      Your ESP8266 is connected to WiFi and serving this page.
      Use the buttons above to control the built-in LED (GPIO2/D4).
      The page tracks all client requests and displays device information from multiple clients.
      Status updates automatically every 5 seconds.
    </div>
  </div>

  <script>
    let autoRefreshTimer = null;

    // Control LED
    function controlLED(state) {
      const messageDiv = document.getElementById('message');
      const buttons = document.querySelectorAll('button');

      // Disable all buttons during request
      buttons.forEach(btn => btn.disabled = true);

      // Send HTTP request
      fetch('/led?state=' + state)
        .then(response => {
          if (!response.ok) throw new Error('Request failed');
          return response.text();
        })
        .then(data => {
          // Show success message
          showMessage('success', '✓ ' + data);

          // Update status immediately
          setTimeout(updateStatus, 100);
        })
        .catch(error => {
          // Show error message
          showMessage('error', '✗ Failed to control LED');
          console.error('Error:', error);
        })
        .finally(() => {
          // Re-enable buttons
          buttons.forEach(btn => btn.disabled = false);
        });
    }

    // Update system status
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          // Update LED status
          const ledState = data.led.state;
          document.getElementById('ledStatus').textContent = ledState;

          // Update LED indicator
          const indicator = document.getElementById('ledIndicator');
          if (ledState === 'ON') {
            indicator.className = 'led-indicator led-on';
          } else {
            indicator.className = 'led-indicator led-off';
          }

          // Update WiFi signal
          const rssi = data.wifi.rssi;
          let signalQuality = 'Excellent';
          if (rssi < -80) signalQuality = 'Poor';
          else if (rssi < -70) signalQuality = 'Fair';
          else if (rssi < -60) signalQuality = 'Good';
          document.getElementById('wifiSignal').textContent = rssi + ' dBm (' + signalQuality + ')';

          // Update uptime
          const uptime = data.system.uptime;
          const hours = Math.floor(uptime / 3600);
          const minutes = Math.floor((uptime % 3600) / 60);
          const seconds = uptime % 60;
          document.getElementById('uptime').textContent =
            hours + 'h ' + minutes + 'm ' + seconds + 's';
        })
        .catch(error => {
          console.error('Status update failed:', error);
        });
    }

    // Update clients list
    function updateClients() {
      fetch('/clients')
        .then(response => response.json())
        .then(data => {
          const clientsList = document.getElementById('clientsList');
          const totalRequests = document.getElementById('totalRequests');

          // Update total requests
          totalRequests.textContent = 'Total Requests: ' + data.totalRequests;

          // Clear current list
          clientsList.innerHTML = '';

          // Check if there are any requests
          if (data.recentRequests.length === 0) {
            clientsList.innerHTML = '<div class="no-clients">No requests yet</div>';
            return;
          }

          // Display each client request
          data.recentRequests.forEach(request => {
            const clientDiv = document.createElement('div');
            clientDiv.className = 'client-item';

            // Parse device type from User-Agent
            let deviceType = 'Unknown';
            let deviceIcon = '🖥️';
            const ua = request.userAgent;

            if (ua.includes('iPhone')) {
              deviceType = 'iPhone';
              deviceIcon = '📱';
            } else if (ua.includes('iPad')) {
              deviceType = 'iPad';
              deviceIcon = '📱';
            } else if (ua.includes('Android')) {
              deviceType = 'Android';
              deviceIcon = '📱';
            } else if (ua.includes('Macintosh')) {
              deviceType = 'Mac';
              deviceIcon = '💻';
            } else if (ua.includes('Windows')) {
              deviceType = 'Windows PC';
              deviceIcon = '💻';
            } else if (ua.includes('Linux')) {
              deviceType = 'Linux';
              deviceIcon = '💻';
            }

            // Parse browser
            let browser = 'Unknown';
            if (ua.includes('Chrome') && !ua.includes('Edg')) {
              browser = 'Chrome';
            } else if (ua.includes('Safari') && !ua.includes('Chrome')) {
              browser = 'Safari';
            } else if (ua.includes('Firefox')) {
              browser = 'Firefox';
            } else if (ua.includes('Edg')) {
              browser = 'Edge';
            }

            clientDiv.innerHTML = `
              <div class="client-header">
                <span class="client-ip">${deviceIcon} ${request.ip}</span>
                <span class="client-time">${request.uptime}</span>
              </div>
              <div class="client-detail"><strong>Device:</strong> ${deviceType} (${browser})</div>
              <div class="client-detail"><strong>Endpoint:</strong> ${request.endpoint}</div>
            `;

            clientsList.appendChild(clientDiv);
          });
        })
        .catch(error => {
          console.error('Clients update failed:', error);
          document.getElementById('clientsList').innerHTML =
            '<div class="no-clients">Failed to load client data</div>';
        });
    }

    // Show message
    function showMessage(type, text) {
      const messageDiv = document.getElementById('message');
      messageDiv.className = type;
      messageDiv.textContent = text;
      messageDiv.style.display = 'block';

      // Hide after 3 seconds
      setTimeout(() => {
        messageDiv.style.display = 'none';
      }, 3000);
    }

    // Auto-update status and clients every 5 seconds
    function startAutoRefresh() {
      updateStatus();
      updateClients();
      autoRefreshTimer = setInterval(() => {
        updateStatus();
        updateClients();
      }, 5000);
    }

    // Initialize on page load
    window.addEventListener('load', () => {
      startAutoRefresh();
    });

    // Clean up on page unload
    window.addEventListener('beforeunload', () => {
      if (autoRefreshTimer) {
        clearInterval(autoRefreshTimer);
      }
    });
  </script>
</body>
</html>
)rawliteral";

#endif // INDEX_H
