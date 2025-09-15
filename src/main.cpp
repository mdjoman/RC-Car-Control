#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// Motor control pins - Using GPIO numbers instead of Dx labels
#define ENA 5     // D1 - GPIO5 - PWM for right motors
#define ENB 4     // D2 - GPIO4 - PWM for left motors
#define IN1 0     // D3 - GPIO0 - Right motor forward
#define IN2 2     // D4 - GPIO2 - Right motor backward
#define IN3 14    // D5 - GPIO14 - Left motor forward
#define IN4 12    // D6 - GPIO12 - Left motor backward

// Additional features
#define HEADLIGHT_PIN 13   // D7 - GPIO13 - Car headlights
#define BRAKELIGHT_PIN 15  // D8 - GPIO15 - Brake lights
#define INDICATOR_LEFT 16  // D0 - GPIO16 - Left indicator
#define INDICATOR_RIGHT 3  // RX - GPIO3 - Right indicator
#define HORN_PIN 1         // TX - GPIO1 - Buzzer for horn

// IR Receiver
#define IR_RECV_PIN 10     // SD3 - GPIO10
IRrecv irrecv(IR_RECV_PIN);
decode_results results;

// WiFi credentials - AP Mode
const char* ssid = "Nabi_Remot_control_car";
const char* password = "123456789";
IPAddress local_IP(192,168,10,1);   // ESP AP IP
IPAddress gateway(192,168,10,1);    // Gateway (same as ESP IP)
IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

    
// Movement variables
int speed = 800;  // PWM value (0-1023)
int turnSpeed = 600;
bool isMoving = false;
String currentDirection = "stop";

// Light states
bool headlightState = false;
bool brakelightState = false;
bool indicatorLeftState = false;
bool indicatorRightState = false;
bool hazardLightsState = false;
bool garageMode = false;
unsigned long lastIndicatorToggle = 0;
const unsigned long indicatorInterval = 500; // Blink interval

// Function prototypes
void handleWebSocketCommand(String command);
void handleIRCommand(unsigned long value);
void setSpeed(int speedPercent);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void toggleHeadlight();
void toggleBrakelight();
void toggleLeftIndicator();
void toggleRightIndicator();
void toggleHazardLights();
void soundHorn();
void toggleGarageMode();
void handleIndicators();

// HTML web page with enhanced UI
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP8266 RC Car</title>
  <style>
    :root {
      --primary: #2c3e50;
      --secondary: #3498db;
      --accent: #e74c3c;
      --success: #2ecc71;
      --warning: #f39c12;
      --dark: #34495e;
      --light: #ecf0f1;
    }
    
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    
    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      text-align: center; 
      background: linear-gradient(135deg, var(--dark) 0%, var(--primary) 100%);
      color: var(--light);
      min-height: 100vh;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }
    
    .container { 
      max-width: 500px; 
      width: 100%;
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      padding: 25px; 
      border-radius: 20px; 
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
      margin: 20px 0;
    }
    
    h1 {
      font-size: 2.2rem;
      margin-bottom: 20px;
      color: white;
      text-shadow: 0 2px 5px rgba(0, 0, 0, 0.3);
    }
    
    .car-status {
      display: flex;
      justify-content: space-between;
      margin-bottom: 20px;
      background: rgba(0, 0, 0, 0.2);
      padding: 15px;
      border-radius: 15px;
    }
    
    .status-item {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    
    .status-label {
      font-size: 0.9rem;
      margin-bottom: 5px;
      opacity: 0.8;
    }
    
    .status-value {
      font-size: 1.1rem;
      font-weight: bold;
    }
    
    .control-panel { 
      margin-bottom: 25px; 
    }
    
    .direction-controls {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-rows: repeat(3, 1fr);
      gap: 10px;
      margin-bottom: 20px;
    }
    
    .btn { 
      padding: 18px; 
      border: none; 
      border-radius: 50%;
      font-size: 18px; 
      cursor: pointer; 
      background: var(--secondary);
      color: white;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      transition: all 0.2s ease;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    
    .btn:active { 
      transform: translateY(3px);
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
    }
    
    .btn-forward {
      grid-column: 2;
      grid-row: 1;
      background: var(--success);
    }
    
    .btn-backward {
      grid-column: 2;
      grid-row: 3;
      background: var(--accent);
    }
    
    .btn-left {
      grid-column: 1;
      grid-row: 2;
    }
    
    .btn-right {
      grid-column: 3;
      grid-row: 2;
    }
    
    .btn-stop {
      grid-column: 2;
      grid-row: 2;
      background: var(--warning);
    }
    
    .feature-controls {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 10px;
      margin-bottom: 20px;
    }
    
    .btn-feature {
      border-radius: 15px;
      padding: 15px;
      background: var(--dark);
    }
    
    .btn-active {
      background: var(--warning);
      box-shadow: 0 0 15px var(--warning);
    }
    
    .slider-container { 
      margin: 20px 0; 
      padding: 15px;
      background: rgba(0, 0, 0, 0.2);
      border-radius: 15px;
    }
    
    .slider {
      width: 100%;
      height: 20px;
      -webkit-appearance: none;
      appearance: none;
      background: linear-gradient(to right, #e74c3c, #f39c12, #2ecc71);
      outline: none;
      border-radius: 10px;
    }
    
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 30px;
      height: 30px;
      border-radius: 50%;
      background: white;
      cursor: pointer;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
    }
    
    .connection-status {
      display: flex;
      align-items: center;
      justify-content: center;
      margin-top: 15px;
      font-size: 0.9rem;
    }
    
    .status-dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      margin-right: 8px;
    }
    
    .connected {
      background: var(--success);
      box-shadow: 0 0 10px var(--success);
    }
    
    .disconnected {
      background: var(--accent);
    }
    
    @media (max-width: 500px) {
      .container {
        padding: 15px;
      }
      
      .btn {
        padding: 15px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🚗 RC Car Control</h1>
    
    <div class="car-status">
      <div class="status-item">
        <div class="status-label">Direction</div>
        <div class="status-value" id="direction">Stop</div>
      </div>
      <div class="status-item">
        <div class="status-label">Speed</div>
        <div class="status-value" id="speed-value">80%</div>
      </div>
      <div class="status-item">
        <div class="status-label">Connection</div>
        <div class="connection-status">
          <div class="status-dot disconnected" id="connection-dot"></div>
          <span id="connection-status">Disconnected</span>
        </div>
      </div>
    </div>
    
    <div class="control-panel">
      <div class="direction-controls">
        <button class="btn btn-forward" id="forward">↑</button>
        <button class="btn btn-backward" id="backward">↓</button>
        <button class="btn btn-left" id="left">←</button>
        <button class="btn btn-right" id="right">→</button>
        <button class="btn btn-stop" id="stop">●</button>
      </div>
    </div>

    <div class="slider-container">
      <label for="speed">Speed Control</label>
      <input type="range" min="0" max="100" value="80" class="slider" id="speed">
    </div>

    <div class="feature-controls">
      <button class="btn-feature" id="headlight">Headlights</button>
      <button class="btn-feature" id="brakelight">Brake Lights</button>
      <button class="btn-feature" id="indicator-left">Left Indicator</button>
      <button class="btn-feature" id="indicator-right">Right Indicator</button>
      <button class="btn-feature" id="hazard">Hazard Lights</button>
      <button class="btn-feature" id="horn">Horn</button>
      <button class="btn-feature" id="garage">Garage Mode</button>
    </div>
  </div>

  <script>
    var websocket;
    var isConnected = false;
    
    function initWebSocket() {
      websocket = new WebSocket('ws://' + window.location.hostname + ':81/');
      
      websocket.onopen = function(event) {
        console.log('WebSocket connected');
        isConnected = true;
        updateConnectionStatus(true);
      };
      
      websocket.onclose = function(event) {
        console.log('WebSocket disconnected');
        isConnected = false;
        updateConnectionStatus(false);
        // Try to reconnect after 2 seconds
        setTimeout(initWebSocket, 2000);
      };
      
      websocket.onmessage = function(event) {
        console.log('Received: ', event.data);
        // Handle incoming messages if needed
      };
    }
    
    function updateConnectionStatus(connected) {
      const dot = document.getElementById('connection-dot');
      const status = document.getElementById('connection-status');
      
      if (connected) {
        dot.className = 'status-dot connected';
        status.textContent = 'Connected';
      } else {
        dot.className = 'status-dot disconnected';
        status.textContent = 'Disconnected';
      }
    }
    
    function sendCommand(command) {
      if (isConnected) {
        websocket.send(command);
        console.log('Sent: ', command);
      } else {
        console.log('WebSocket is not connected');
      }
    }
    
    // Initialize when page loads
    window.onload = function() {
      initWebSocket();
      
      // Direction controls
      document.getElementById('forward').addEventListener('touchstart', function() {
        sendCommand('forward');
        document.getElementById('direction').textContent = 'Forward';
      });
      document.getElementById('forward').addEventListener('touchend', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('backward').addEventListener('touchstart', function() {
        sendCommand('backward');
        document.getElementById('direction').textContent = 'Backward';
      });
      document.getElementById('backward').addEventListener('touchend', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('left').addEventListener('touchstart', function() {
        sendCommand('left');
        document.getElementById('direction').textContent = 'Turning Left';
      });
      document.getElementById('left').addEventListener('touchend', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('right').addEventListener('touchstart', function() {
        sendCommand('right');
        document.getElementById('direction').textContent = 'Turning Right';
      });
      document.getElementById('right').addEventListener('touchend', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('stop').addEventListener('click', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      // Speed control
      document.getElementById('speed').addEventListener('input', function() {
        const speedValue = this.value;
        document.getElementById('speed-value').textContent = speedValue + '%';
        sendCommand('speed:' + speedValue);
      });
      
      // Feature controls
      const features = ['headlight', 'brakelight', 'indicator-left', 'indicator-right', 'hazard', 'horn', 'garage'];
      features.forEach(function(feature) {
        document.getElementById(feature).addEventListener('click', function() {
          sendCommand(feature);
          
          // Toggle active state for visual feedback
          if (feature !== 'horn') { // Horn is momentary, not toggle
            this.classList.toggle('btn-active');
          }
        });
      });
      
      // Mouse events for desktop compatibility
      document.getElementById('forward').addEventListener('mousedown', function() {
        sendCommand('forward');
        document.getElementById('direction').textContent = 'Forward';
      });
      document.getElementById('forward').addEventListener('mouseup', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('backward').addEventListener('mousedown', function() {
        sendCommand('backward');
        document.getElementById('direction').textContent = 'Backward';
      });
      document.getElementById('backward').addEventListener('mouseup', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('left').addEventListener('mousedown', function() {
        sendCommand('left');
        document.getElementById('direction').textContent = 'Turning Left';
      });
      document.getElementById('left').addEventListener('mouseup', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
      
      document.getElementById('right').addEventListener('mousedown', function() {
        sendCommand('right');
        document.getElementById('direction').textContent = 'Turning Right';
      });
      document.getElementById('right').addEventListener('mouseup', function() {
        sendCommand('stop');
        document.getElementById('direction').textContent = 'Stop';
      });
    };
  </script>
</body>
</html>
)rawliteral";

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Received: %s\n", num, payload);
      if(length > 0) {
        String message = String((char*)payload);
        handleWebSocketCommand(message);
      }
      break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void handleWebSocketCommand(String command) {
  if (command == "forward") {
    moveForward();
  } else if (command == "backward") {
    moveBackward();
  } else if (command == "left") {
    turnLeft();
  } else if (command == "right") {
    turnRight();
  } else if (command == "stop") {
    stopMotors();
  } else if (command.startsWith("speed:")) {
    int speedPercent = command.substring(6).toInt();
    setSpeed(speedPercent);
  } else if (command == "headlight") {
    toggleHeadlight();
  } else if (command == "brakelight") {
    toggleBrakelight();
  } else if (command == "indicator-left") {
    toggleLeftIndicator();
  } else if (command == "indicator-right") {
    toggleRightIndicator();
  } else if (command == "hazard") {
    toggleHazardLights();
  } else if (command == "horn") {
    soundHorn();
  } else if (command == "garage") {
    toggleGarageMode();
  }
}

void handleIRCommand(unsigned long value) {
  Serial.print("IR Command: ");
  Serial.println(value, HEX);
  
  // Map your IR remote buttons to these values
  // These are examples - you'll need to replace them with your remote's codes
  switch(value) {
    case 0xFFA25D:  // Example: Power button
      stopMotors();
      break;
    case 0xFF629D:  // Example: Vol+ button
      moveForward();
      break;
    case 0xFFA857:  // Example: Vol- button
      moveBackward();
      break;
    case 0xFF22DD:  // Example: Rewind button
      turnLeft();
      break;
    case 0xFFC23D:  // Example: Fast Forward button
      turnRight();
      break;
    case 0xFF02FD:  // Example: Play/Pause button
      toggleHeadlight();
      break;
    case 0xFFE01F:  // Example: Down button
      toggleBrakelight();
      break;
    case 0xFF906F:  // Example: Up button
      toggleLeftIndicator();
      break;
    case 0xFF6897:  // Example: 0 button
      toggleRightIndicator();
      break;
    case 0xFF9867:  // Example: EQ button
      toggleHazardLights();
      break;
    case 0xFFB04F:  // Example: ST/REPT button
      soundHorn();
      break;
    case 0xFF30CF:  // Example: 1 button
      toggleGarageMode();
      break;
    default:
      Serial.println("Unknown IR command");
  }
}

void setSpeed(int speedPercent) {
  speed = map(speedPercent, 0, 100, 0, 1023);
  turnSpeed = map(speedPercent, 0, 100, 0, 800); // Lower speed for turning
  
  // Apply speed to motors if moving
  if (currentDirection == "forward") moveForward();
  else if (currentDirection == "backward") moveBackward();
  else if (currentDirection == "left") turnLeft();
  else if (currentDirection == "right") turnRight();
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  currentDirection = "forward";
  isMoving = true;
  
  // Turn off brake lights when moving forward
  if (brakelightState) {
    toggleBrakelight();
  }
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  currentDirection = "backward";
  isMoving = true;
  
  // Turn on brake lights when moving backward
  if (!brakelightState) {
    toggleBrakelight();
  }
}

void turnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
  currentDirection = "left";
  isMoving = true;
  
  // Activate left indicator
  if (!indicatorLeftState && !hazardLightsState) {
    toggleLeftIndicator();
  }
}

void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
  currentDirection = "right";
  isMoving = true;
  
  // Activate right indicator
  if (!indicatorRightState && !hazardLightsState) {
    toggleRightIndicator();
  }
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  currentDirection = "stop";
  isMoving = false;
  
  // Turn on brake lights when stopping
  if (!brakelightState) {
    toggleBrakelight();
  }
  
  // Turn off indicators if they were activated by turning
  if (indicatorLeftState && !hazardLightsState) {
    toggleLeftIndicator();
  }
  if (indicatorRightState && !hazardLightsState) {
    toggleRightIndicator();
  }
}

void toggleHeadlight() {
  headlightState = !headlightState;
  digitalWrite(HEADLIGHT_PIN, headlightState);
}

void toggleBrakelight() {
  brakelightState = !brakelightState;
  digitalWrite(BRAKELIGHT_PIN, brakelightState);
}

void toggleLeftIndicator() {
  indicatorLeftState = !indicatorLeftState;
  if (hazardLightsState) {
    // If hazards are on, turn them off first
    toggleHazardLights();
  }
}

void toggleRightIndicator() {
  indicatorRightState = !indicatorRightState;
  if (hazardLightsState) {
    // If hazards are on, turn them off first
    toggleHazardLights();
  }
}

void toggleHazardLights() {
  hazardLightsState = !hazardLightsState;
  if (hazardLightsState) {
    // Turn off individual indicators if they were on
    if (indicatorLeftState) indicatorLeftState = false;
    if (indicatorRightState) indicatorRightState = false;
  }
}

void soundHorn() {
  digitalWrite(HORN_PIN, HIGH);
  delay(300);
  digitalWrite(HORN_PIN, LOW);
}

void toggleGarageMode() {
  garageMode = !garageMode;
  if (garageMode) {
    // Enter garage mode - slow speed, lights on
    setSpeed(30);
    if (!headlightState) toggleHeadlight();
  } else {
    // Exit garage mode - restore previous speed
    setSpeed(80);
  }
}

void handleIndicators() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastIndicatorToggle >= indicatorInterval) {
    lastIndicatorToggle = currentMillis;
    
    if (hazardLightsState) {
      // Toggle both indicators for hazard lights
      digitalWrite(INDICATOR_LEFT, !digitalRead(INDICATOR_LEFT));
      digitalWrite(INDICATOR_RIGHT, !digitalRead(INDICATOR_RIGHT));
    } else if (indicatorLeftState) {
      // Blink left indicator
      digitalWrite(INDICATOR_LEFT, !digitalRead(INDICATOR_LEFT));
      digitalWrite(INDICATOR_RIGHT, LOW);
    } else if (indicatorRightState) {
      // Blink right indicator
      digitalWrite(INDICATOR_RIGHT, !digitalRead(INDICATOR_RIGHT));
      digitalWrite(INDICATOR_LEFT, LOW);
    } else {
      // Turn off both indicators
      digitalWrite(INDICATOR_LEFT, LOW);
      digitalWrite(INDICATOR_RIGHT, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);  
  Serial.println("Hello ESP8266");
  // Initialize motor control pins
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Initialize additional features
  pinMode(HEADLIGHT_PIN, OUTPUT);
  pinMode(BRAKELIGHT_PIN, OUTPUT);
  pinMode(INDICATOR_LEFT, OUTPUT);
  pinMode(INDICATOR_RIGHT, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);
  
  // Stop motors initially
  stopMotors();
  
  // Initialize IR receiver
  irrecv.enableIRIn();
  Serial.println("IR Receiver Initialized");
  
  // Set up WiFi access point
 if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("AP Config Failed!");
  }

  // Start AP
  WiFi.softAP(ssid, password);

  // Print IP address
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  
  // Start server
  server.begin();
  
  // Initialize WebSocket with correct event handler
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("HTTP server started");
  Serial.println("WebSocket server started");
}

void loop() {
  webSocket.loop();
  
  // Handle IR remote commands
  if (irrecv.decode(&results)) {
    handleIRCommand(results.value);
    irrecv.resume();  // Receive the next value
  }
  
  // Handle indicator blinking
  handleIndicators();
  
  delay(10);
}