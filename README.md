# Play Clock Module

ESP32-based wireless play clock display for scoreboard systems. Shows seconds (SS) on large 7-segment LED displays and receives timing data wirelessly from a central controller.

## Overview

The Play Clock module is a pure display unit that:
- Displays seconds (00-99) on large LED digits
- Receives timing data wirelessly from controller
- Shows only controller data without local logic
- Operates as a simple display peripheral

## Hardware

### Components
- **MCU**: ESP32
- **Radio**: SX1278 (868 MHz) or nRF24L01+ (2.4 GHz)
- **LED Display**: WS2815 12V LED strips forming 2 × 100cm digits
- **Power**: 12V DC with power injection at both ends

### Pin Configuration
| Function | Pin | Description |
|----------|-----|-------------|
| Radio CE | GPIO5 | Radio chip enable |
| Radio CSN | GPIO4 | Radio chip select |
| Status LED | GPIO2 | System status indicator |

### Display Specifications
- **Digits**: 2 (seconds display - SS format)
- **Segment Height**: ~50cm (~30 LEDs per vertical segment)
- **Horizontal Segments**: 25cm (~15 LEDs)
- **LED Type**: WS2815 12V (dual data lines for reliability)
- **LED Density**: 60 LEDs/m
- **Total LEDs**: ~900 (estimated for complete display)

## Operation

### Display Features
- **Time Display**: Shows seconds (00-99) as received from controller
- **Connection Monitoring**: Status LED indicates controller link status
- **Smart Blinking**: Different blink rates for connected/disconnected states
- **Error Display**: Shows error pattern on hardware failure
- **Timeout Detection**: 10-second timeout with visual warning

### Communication Protocol
- **Radio Module**: nRF24L01+ (2.4 GHz) wireless communication
- **Data Format**: 3-byte payload [seconds_high, seconds_low, sequence]
- **Update Rate**: Receives data every 250ms from controller
- **Pure Display**: Shows received seconds without local processing

### Data Reception
- **Controller-Driven**: All timing data comes from controller module
- **Link Monitoring**: Tracks connection status with timeout detection
- **Smart Status LED**: Visual feedback for connection state
- **Simple Parsing**: Extracts seconds and sequence from received packets
- **Real-time Display**: Updates immediately when new data arrives

## Build and Flash

See [AGENTS.md](AGENTS.md) for detailed development commands and workflow.

### Quick Start
```bash
# Activate ESP-IDF environment
idf

# Build and flash
idf.py build flash monitor

# Or build only
idf.py build
```

### Development Environment
- **Language**: Native C (not C++)
- **ESP-IDF Version**: v6.1-dev
- **Target Platform**: ESP32
- **Build System**: CMake with ESP-IDF

## Dependencies

### ESP-IDF Components
- `driver/gpio` - GPIO control
- `freertos` - Task management
- `esp_log` - Logging system

### Implementation Language
- **Native C**: Project implemented in pure C (not C++)
- **Struct-based Design**: Uses C structs instead of classes
- **ESP-IDF Framework**: Built on ESP-IDF v6.1-dev

All functionality is implemented using ESP-IDF built-in components for maximum reliability and performance.

## Troubleshooting

### Display Issues
- Check 12V power supply and connections
- Verify LED strip wiring and data polarity
- Confirm power injection at both ends of each digit

### Radio Communication
- Verify nRF24L01+ connections (CE, CSN, SPI)
- Check radio module power supply
- Confirm controller is transmitting data
- Verify payload format matches controller (seconds+sequence)
- Check status LED behavior for connection issues
- Monitor serial logs for timeout/recovery messages

### System Errors
- Check serial monitor for error messages
- Verify ESP32 power and boot sequence
- Check for hardware conflicts (pin assignments)

## Technical Specifications

### Memory Usage
- **LED Buffer**: ~2.7KB (900 LEDs × 3 bytes, mock allocation)
- **Stack Size**: Configured for FreeRTOS tasks
- **Heap Usage**: Minimal, mainly for struct allocation
- **Code Size**: Optimized C implementation

### Timing Constraints
- **Main Loop**: 50ms cycle time
- **Radio Reception**: Continuous listening for controller data
- **Display Updates**: Immediate when new data received
- **Link Timeout**: 10-second timeout detection
- **Status LED**: 2s blink (connected), 200ms blink (disconnected)

### Power Consumption
- **ESP32**: ~160-200mA (base operation)
- **Future LED Strip**: 3-6A at 12V (when implemented)
- **Future Radio**: ~15mA (receive mode)