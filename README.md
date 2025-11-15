<<<<<<< HEAD
# Play Clock Module

This module implements the Play Clock display for the scoreboard system. It displays seconds (SS) on a 2-digit 7-segment LED display and receives timing data wirelessly from the controller.

## Hardware

### Components
- **MCU**: ESP32
- **Radio**: SX1278 (868 MHz) or nRF24L01+ (2.4 GHz)
- **LED Display**: WS2815 12V LED strips forming 2 × 100cm digits
- **Power**: 12V DC with power injection at both ends

### Pin Configuration
- **LED Data Pin**: GPIO5 (RMT channel)
- **LED Power Control**: GPIO4 (optional power switching)
- **Radio CE**: GPIO22
- **Radio CSN**: GPIO21
- **SPI (for nRF24L01+)**:
  - MOSI: GPIO23
  - MISO: GPIO19
  - SCK: GPIO18

### Display Specifications
- **Digits**: 2 (for seconds display - SS format)
- **Segment Height**: ~50cm (~30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (~15 LEDs)
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m
- **Total LEDs**: ~900 (estimated for complete display)

## Functionality

### Operation Mode
- **Receive-only**: Listens for status broadcasts from controller
- **No local timing**: Displays only data received from controller
- **Passive display**: No user interaction required

### Communication Protocol
- **Frame Type**: 0xA1 (Status frame)
- **Data Rate**: 250kbps
- **Channel**: 100 (2.500 GHz)
- **Update Rate**: Every 100-200ms from controller

### Display Features
- **Time Display**: Shows seconds (00-99)
- **Status Indication**: Different colors for RUN/STOP/RESET states
- **Link Status**: Visual indication when controller connection is lost
- **Error Display**: Shows error pattern on hardware failure

## Build and Flash

### Using PlatformIO
```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Using ESP-IDF
```bash
# Build the project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor
```

## Network Configuration

### Radio Settings
- **Node ID**: 1 (Play Clock node)
- **Controller ID**: 0 (Master node)
- **Auto-ACK**: Enabled
- **Retries**: 15 attempts, 1500µs delay
- **CRC**: 16-bit validation
- **Address Width**: 5 bytes

### Link Monitoring
- **Timeout Detection**: 800ms without status triggers warning
- **Visual Indicator**: Middle segment blinks on link loss
- **Auto-Recovery**: Normal operation resumes when connection restored

## Dependencies

### ESP-IDF Components
- `driver/gpio` - GPIO control
- `driver/rmt` - LED strip control
- `driver/spi_master` - SPI communication for radio
- `freertos` - Task management
- `esp_log` - Logging system

### No External Libraries
All functionality is implemented using ESP-IDF built-in components for maximum reliability and performance.

## Troubleshooting

### Display Issues
- Check 12V power supply and connections
- Verify LED strip wiring and data polarity
- Confirm power injection at both ends of each digit

### Radio Communication
- Verify radio module connections (CE, CSN, SPI)
- Check radio module power supply
- Confirm frequency and channel match with controller

### System Errors
- Check serial monitor for error messages
- Verify ESP32 power and boot sequence
- Check for hardware conflicts (pin assignments)

## Documentation

- [Wiring Diagram](docs/wiring_diagram.md) - Complete wiring instructions and diagrams
- [Connection Flow Diagrams](docs/connection_flow.md) - Visual system architecture and signal flow
- [Hardware Setup Guide](docs/hardware_setup.md) - Step-by-step assembly instructions
- [Testing Procedures](docs/testing_procedures.md) - Comprehensive testing and validation guide

## Development Notes

### Memory Usage
- **LED Buffer**: ~2.7KB (900 LEDs × 3 bytes)
- **Stack Size**: Configured for FreeRTOS tasks
- **Heap Usage**: Minimal, mainly for radio buffers

### Timing Constraints
- **Main Loop**: 10ms cycle time
- **LED Updates**: Async via RMT hardware
- **Radio Reception**: Event-driven, non-blocking

### Power Consumption
- **ESP32**: ~160-260mA (depending on radio activity)
- **LED Strip**: 3-6A at 12V (depending on display content)
- **Radio**: ~15mA (receive mode)
=======
# ScoreBoard Timer System

A modular wireless scoreboard system based on ESP32 microcontrollers and WS2815 LED strips, designed for sports timing applications.

## 🏗️ System Architecture

The system consists of multiple networked nodes that communicate wirelessly to provide synchronized timing displays:

### Node Types

| Node | Function | Radio | Display | Role |
|------|----------|-------|---------|------|
| **Controller** | Master timing control | SX1278 (868 MHz) | OLED/LCD status | Broadcasts timing data |
| **Play Clock** | Seconds display (SS) | SX1278/nRF24L01+ | 2×100cm digits | Receive-only display |
| **Game Clock** | Minutes:Seconds (MM:SS) | SX1278/nRF24L01+ | 4×60cm digits | Receive-only display |
| **Referee Watch** | Remote control | SX1278 | LCD + buttons | Sends commands |
| **Repeaters** | Network extension | SX1278 | None | Mesh routing |

## 📁 Project Structure

```
scoreboard_clock/
├── README.md                 # This file
├── CLAUDE.md                 # Development guidance for Claude
├── play_clock/              # ✅ Play Clock module
│   ├── src/
│   │   ├── main.cpp         # Main application
│   │   ├── display_driver.cpp
│   │   └── radio_comm.cpp
│   ├── include/
│   │   ├── display_driver.h
│   │   └── radio_comm.h
│   ├── platformio.ini
│   ├── CMakeLists.txt
│   └── README.md            # Module-specific docs
├── game_clock/              # 🚧 Game Clock module (planned)
├── controller/              # 🚧 Controller module (planned)
├── referee_watch/           # 🚧 Referee Watch module (planned)
└── repeater/                # 🚧 Repeater module (planned)
```

## 📡 Radio Communication

### Common Settings (nRF24L01+)
- **Data Rate**: 250 kbps (best sensitivity/range)
- **Channel**: 100 (≈2.500 GHz, above Wi-Fi)
- **Power Level**: MAX on Controller/Repeaters, medium/low on Watch
- **Auto-ACK**: Enabled with 15 retries, 1500µs delay
- **Dynamic Payloads**: ON (≤32 bytes)
- **CRC**: 16-bit validation
- **Address Width**: 5 bytes

### Protocol Structure

**Status Broadcast Frame (≤32 bytes)**
```
type=0xA1  // status frame
state: 1B  // 0=STOP, 1=RUN, 2=RESET
seconds: 2B
ms_lowres: 2B  // 0..999/10 for tenths
seq: 1B
crc8: 1B
```

**Command Frame (Watch → Controller)**
```
type=0xB1  // command frame
cmd: 1B    // 1=START, 2=STOP, 3=RESET
nonce: 2B
```

## 🔧 Module Details

### ✅ Play Clock Module (`/play_clock/`)

**Status**: Implemented and ready for deployment
**Function**: Displays seconds (SS) on 2 × 100cm digits
**Hardware**: ESP32 + WS2815 LED strips + Radio module

#### Key Features
- **Receive-only operation** - No local timing logic
- **2-digit 7-segment display** for seconds (00-99)
- **Link monitoring** with visual warnings on connection loss
- **Automatic recovery** when controller reconnects

#### Hardware Specifications
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m (16.6mm spacing)
- **Segment Height**: ~50cm (≈30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (≈15 LEDs)
- **Power**: 12V DC injected at both ends of each digit

#### 7-Segment Layout
```
 —A—
 | |
 F B
 | |
 —G—
 | |
 E C
 | |
 —D—
```

#### Documentation
- See [`/play_clock/README.md`](./play_clock/README.md) for detailed documentation
- Includes pin configurations, build instructions, and troubleshooting

---

### 🚧 Game Clock Module (`/game_clock/`)

**Status**: Planned - Not yet implemented
**Function**: Displays minutes and seconds (MM:SS) on 4 × 60cm digits
**Hardware**: ESP32 + WS2815 LED strips + Radio module

#### Planned Features
- **4-digit 7-segment display** (MM:SS format)
- **Receive-only operation** similar to Play Clock
- **Synchronized timing** with Play Clock and Controller

#### Hardware Specifications
- **LED Type**: WS2815 12V
- **Segment Height**: ~30cm (≈20 LEDs per vertical segment)
- **Horizontal Segments**: 15cm (≈10 LEDs)
- **Power**: 12V DC injected at both ends

---

### 🚧 Controller Module (`/controller/`)

**Status**: Planned - Not yet implemented
**Function**: Master timing control and system coordinator
**Hardware**: ESP32 + SX1278 (868 MHz) + Buttons + OLED/LCD

#### Planned Responsibilities
- **Maintain official time/state** for entire system
- **Broadcast status** every 100-200ms to all display modules
- **Process commands** from Referee Watch
- **Mesh address assignment** (if using RF24Mesh)
- **System synchronization** and error handling

#### User Interface
- Start/Stop/Reset buttons
- Status display (OLED/LCD)
- System health indicators

---

### 🚧 Referee Watch Module (`/referee_watch/`)

**Status**: Planned - Not yet implemented
**Function**: Remote control for referees
**Hardware**: ESP32 + SX1278 + LCD + Buttons

#### Planned Features
- **Compact handheld design**
- **Button controls** for Start/Stop/Reset
- **LCD display** showing current time and status
- **Haptic feedback** on command confirmation
- **Low power operation** for battery use

#### Command Protocol
- Sends command packets to Controller
- Expects ACK confirmation (80-120ms timeout)
- Automatic retry (up to 3 attempts)

---

### 🚧 Repeater Module (`/repeater/`)

**Status**: Planned - Not yet implemented
**Function**: Network range extension
**Hardware**: ESP32 + SX1278 (mains powered)

#### Planned Features
- **Passive mesh routing** - No application logic
- **High placement** (2.5-4m) for optimal coverage
- **Automatic failover** and route discovery
- **Mains powered** with battery backup option

## 🚀 Quick Start

### Prerequisites
- ESP32 development board
- PlatformIO or ESP-IDF development environment
- WS2815 LED strips and appropriate power supplies
- Radio modules (nRF24L01+ or SX1278)

### Building the Play Clock Module

```bash
# Navigate to the play clock module
cd play_clock/

# Using PlatformIO
pio run                    # Build the project
pio run --target upload    # Flash to ESP32
pio device monitor         # View serial output

# Using ESP-IDF
idf.py build              # Build the project
idf.py flash              # Flash to device
idf.py monitor            # View serial output
```

### Development Workflow
1. **Set up development environment** (PlatformIO recommended)
2. **Configure hardware pins** in module-specific configuration
3. **Build and flash** individual modules
4. **Test radio communication** between modules
5. **Deploy in network topology** with repeaters as needed

## 📋 Development Status

| Module | Status | Next Steps |
|--------|--------|------------|
| **Play Clock** | ✅ Complete | Field testing, optimization |
| **Game Clock** | 🚧 Planned | LED driver adaptation |
| **Controller** | 🚧 Planned | State machine, UI |
| **Referee Watch** | 🚧 Planned | UI design, power management |
| **Repeater** | 🚧 Planned | Mesh networking setup |

## 🔗 Technical Documentation

- **Development Guidelines**: See [`CLAUDE.md`](./CLAUDE.md) for AI development assistance
- **Module Documentation**: Each module has its own README with detailed specifications
- **Radio Protocol**: Detailed in the Radio Communication section above
>>>>>>> 2827031d59a8c4cdf3119a0edaf19754d07fb3ac
