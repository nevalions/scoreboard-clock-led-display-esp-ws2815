# ESP32 Play Clock

A wireless play clock display for sports scoreboard systems using ESP32 and WS2815 LED strips.

## Overview

This project implements a play clock that receives time data wirelessly from a controller and displays it on large LED digit displays. It's designed for sports timing applications where a visible play clock is needed.

## Features

- **Wireless Communication**: Receives data via nRF24L01+ radio module
- **Large LED Display**: Uses WS2815 LED strips for 7-segment digit display
- **Real-time Updates**: Displays current play time with second precision
- **Link Monitoring**: Shows connection status with visual feedback
- **Hardware Testing**: Built-in test patterns for verification
- **Multiple Display Modes**: Stop, Run, Reset, and Error states

## Hardware Requirements

### Components
- ESP32 development board
- WS2815 LED strips (12V, approximately 165 LEDs per digit)
- nRF24L01+ wireless module
- 12V DC power supply (3-6A capacity)
- Button for testing (optional, uses GPIO0/BOOT button)

### Wiring
- LED data line to ESP32 GPIO pin (configurable)
- nRF24L01+ SPI interface (MOSI, MISO, SCK, CSN, CE)
- Power: 5V/3.3V to ESP32, 12V to LED strips
- Common ground connection required

## Software Architecture

### Core Components
- **Display Driver**: Manages WS2815 LED strips and 7-segment mapping
- **Radio Communication**: Handles nRF24L01+ data reception
- **System State**: Tracks time, sequence, and link status
- **Button Handler**: Debounces input for testing modes

### Data Protocol
Receives 3-byte packets:
- Byte 0-1: Time value (0-65535 seconds)
- Byte 2: Sequence number (0-255)

### Display Modes
- **Stop Mode**: Shows current time, static display
- **Run Mode**: Shows current time, ready for updates
- **Reset Mode**: Shows "00" or error pattern
- **Error Mode**: Shows warning pattern for communication issues

## Installation and Setup

### Prerequisites
- ESP-IDF development environment (v6.1-dev)
- Compatible toolchain for ESP32

### Build Instructions
```bash
# Activate ESP-IDF environment
idf

# Build the project
idf.py build

# Clean build if needed
idf.py fullclean

# Configure project
idf.py menuconfig
```

**Important**: This project should only be built, never flashed to hardware.

### Configuration
- LED strip pin in `sdkconfig`
- Radio settings in source code
- Display brightness and colors configurable

## Usage

### Normal Operation
1. Power on the device
2. LED test pattern runs on startup
3. Device listens for radio data
4. Display shows received time data
5. Status LED indicates link quality

### Testing Mode
- Press and hold the BOOT button (GPIO0) during startup
- Release to enter number cycling test (00-99)
- Useful for verifying LED segment mapping

### Troubleshooting
- Check radio link status LED
- Verify power and ground connections
- Monitor serial output for debug information
- Use built-in test patterns to verify hardware

## Development

### Project Structure
```
├── main/
│   ├── main.c              # Main application logic
│   ├── display_driver.c    # LED strip management
│   ├── led_strip_encoder.c # WS2815 protocol handling
│   └── radio_comm.c        # Radio communication
├── include/
│   ├── display_driver.h    # Display driver interface
│   ├── led_strip_encoder.h # LED strip encoder interface
│   └── radio_comm.h        # Radio communication interface
└── CMakeLists.txt          # Build configuration
```

### Key Features
- **Pure C Implementation**: Native ESP-IDF C code (converted from C++)
- **Real-time Operation**: Efficient main loop design
- **Hardware Abstraction**: Clean separation of concerns
- **Error Handling**: Robust error detection and recovery
- **Thread Safety**: Mutex protection for shared resources
- **radio_common Integration**: Uses shared nRF24L01+ driver library

## Technical Specifications

- **Microcontroller**: ESP32 (240 MHz, dual-core)
- **Display**: WS2815 LED strips, 12V powered
- **Wireless**: nRF24L01+ (2.4 GHz, 1 Mbps)
- **Update Rate**: Up to 50 Hz display updates
- **Power Consumption**: ~500mA (12V) + ESP32 power
- **Operating Temperature**: -20°C to +70°C

## License

This project is licensed under the MIT License - see the LICENSE file for details.