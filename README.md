# 🚗 Nabil RC Car Project

## 📋 Project Overview
An ESP8266-based remote control car with web interface and IR remote control capabilities. Features motor control, lighting systems, and real-time status updates.

## 🎮 Web Interface Preview

![RC Car Web Interface](https://via.placeholder.com/400x700/2a2a40/ffffff?text=RC+Car+Interface)
*Web Interface for Mobile Control*

## ✨ Features

### 🎯 Core Functionality
- **Motor Control**: Forward, backward, left, right movements
- **Speed Control**: Adjustable PWM speed (0-100%)
- **Lighting System**: Headlights, brake lights, indicators
- **Horn**: Audible alert system
- **Dual Control**: Web interface + IR remote

### 🌐 Web Interface Features
- **Real-time Status**: Direction, speed, connection status
- **Touch Controls**: Mobile-optimized touch interface
- **Visual Feedback**: Toggle states and active indicators
- **Connection Management**: Auto-reconnect with visual indicators
- **Responsive Design**: Works on mobile and desktop

## 🛠️ Technical Specifications

### Hardware Components
| Component | Pin | Function |
|-----------|-----|----------|
| Motor Driver | ENA (D1), ENB (D2) | PWM Speed Control |
| Motor Control | IN1 (D3), IN2 (D4) | Motor A Direction |
| Motor Control | IN3 (D5), IN4 (D6) | Motor B Direction |
| Headlights | D7 | Front illumination |
| Brake Lights | D8 | Rear braking indicator |
| Indicators | D0, RX | Turn signals |
| Horn | TX | Audible alert |
| IR Receiver | SD3 | Remote control input |

### Software Features
- **Web Server**: AsyncWebServer on port 80
- **WebSocket**: Real-time communication on port 81
- **WiFi Mode**: Access Point (192.168.10.1)
- **Protocols**: HTTP, WebSocket, IR remote

## 📱 Interface Layout

### Status Panel
```
┌─────────────────────────────────┐
│           🚗 Nabil RC Car       │
├─────────────────────────────────┤
│ Direction: ⏹    Speed: ⚡ 80%   │
│ Connection: ● Connected          │
├─────────────────────────────────┤
│                                 │
│           ▲ [Forward]           │
│     ◀ [Left]   ■ [Stop]   ▶ [Right]
│           ▼ [Backward]          │
│                                 │
├─────────────────────────────────┤
│ [Speed Slider 0───────□──────100]│
├─────────────────────────────────┤
│ [🔆] [⏹] [◀] [▶] [🔊]           │
│ Head  Brake  L    R   Horn      │
│ light Light  Turn Turn          │
└─────────────────────────────────┘
```

# Required Arduino Libraries
ESP8266WiFi
ESPAsyncTCP
ESPAsyncWebServer
WebSocketsServer
IRremoteESP8266
```

### Wiring Diagram
```
ESP8266 → L298N Motor Driver
D1 (ENA) → Enable A
D2 (ENB) → Enable B  
D3 (IN1) → Input 1
D4 (IN2) → Input 2
D5 (IN3) → Input 3
D6 (IN4) → Input 4

Lights → ESP8266
Headlights → D7
Brake Lights → D8
Left Indicator → D0
Right Indicator → RX
Horn → TX
IR Receiver → SD3
```

### Flashing the ESP8266
1. Connect ESP8266 via USB
2. Select correct board in Arduino IDE
3. Upload the provided code
4. Monitor serial output at 115200 baud

## 📶 Network Configuration

### Access Point Settings
```cpp
SSID: "🚘 Nabil Remote Car 🚘"
Password: "123456789"
IP Address: 192.168.10.1
Subnet: 255.255.255.0
```

### Connection Steps
1. Power on the RC car
2. Connect to WiFi "Nabi_Remot_control_car"
3. Open browser to http://192.168.10.1
4. Start controlling!

## 🎯 Usage Instructions

### Basic Controls
1. **Movement**: Press and hold direction buttons
2. **Speed**: Use slider to adjust speed (0-100%)
3. **Lights**: Toggle switches for headlights/brake lights
4. **Indicators**: Left/right toggle buttons
5. **Horn**: Press horn button for sound

### IR Remote Control
| Button | Function |
|--------|----------|
| ⏹ | Stop |
| ▲ | Forward |
| ▼ | Backward |
| ◀ | Left turn |
| ▶ | Right turn |
| 🔆 | Headlights |
| ⏹ | Brake lights |
| ◀ | Left indicator |
| ▶ | Right indicator |
| ⚠ | Hazard lights |
| 📢 | Horn |
| 🏠 | Garage mode |

## 🔄 State Management

### Real-time Synchronization
- Web interface reflects actual car state
- Automatic state updates across multiple clients
- Persistent connection with heartbeat mechanism
- Auto-reconnect on connection loss

### State Indicators
- **Direction**: Current movement direction
- **Speed**: Current PWM percentage
- **Lights**: On/off status for all lights
- **Connection**: WebSocket connection status

## 🚀 Performance Optimizations

### Code Efficiency
- Minimal string operations
- Efficient WebSocket communication
- Debounced IR input
- Optimized pin handling
- Non-blocking delays

### Memory Management
- PROGMEM for HTML storage
- Efficient state structure
- Minimal dynamic allocation
- Optimized WebSocket payloads

## 🐛 Troubleshooting

### Common Issues
1. **Connection Problems**: Check WiFi connection to AP
2. **Motor Not Working**: Verify motor driver connections
3. **Lights Not Working**: Check LED wiring and resistors
4. **IR Not Responding**: Verify IR receiver wiring

### Serial Debug Output
Enable `DEBUG_MODE` for detailed logging:
- WebSocket connections
- IR command reception
- State changes
- Error messages

## 📊 Technical Details

### WebSocket Protocol
- **Messages**: JSON-like state strings
- **Heartbeat**: 30-second intervals
- **Reconnect**: 2-second retry interval
- **State Updates**: Broadcast to all clients

### PWM Configuration
- **Base Speed**: 800 (78% duty cycle)
- **Turn Speed**: 600 (59% duty cycle)
- **Range**: 0-1023 (10-bit resolution)

## 🔮 Future Enhancements

### Planned Features
- [ ] Battery level monitoring
- [ ] Camera streaming integration
- [ ] GPS tracking
- [ ] Autonomous driving modes
- [ ] Mobile app development
- [ ] Voice control integration

### Possible Improvements
- OTA (Over-The-Air) updates
- SSL encryption for WebSocket
- Multi-language support
- Theme customization
- Preset speed profiles

## 📝 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

**Note**: Replace the placeholder image with actual screenshots of your web interface for the final documentation.