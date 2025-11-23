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
- **Radio**: nRF24L01+ (2.4 GHz)
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

### Status LED Behavior
- **Connected**: Slow blink (2 second period - 1s on, 1s off)
- **Disconnected**: Fast blink (200ms period - 100ms on, 100ms off)
- **Link Timeout**: 10 seconds without received data triggers disconnection
- **Link Recovery**: Automatic when data reception resumes

## Development

For detailed development setup, build commands, code style guidelines, and workflow, see [AGENTS.md](AGENTS.md).

### Quick Start
```bash
# Activate ESP-IDF environment
idf

# Build project
idf.py build
```

**Important**: This project should only be built, never flashed to hardware.

### Technology Stack
- **Language**: Native C (not C++)
- **Framework**: ESP-IDF v6.1-dev
- **Target**: ESP32 microcontroller
- **Build System**: CMake with ESP-IDF

## Troubleshooting

### Display Issues
- Check 12V power supply and connections
- Verify LED strip wiring and data polarity
- Confirm power injection at both ends of each digit

### Radio Communication
- Verify nRF24L01+ connections (CE, CSN, SPI)
- Check radio module power supply
- Confirm controller is transmitting data
- Check status LED behavior for connection issues
- Monitor serial logs for timeout/recovery messages

### System Errors
- Check serial monitor for error messages
- Verify ESP32 power and boot sequence
- Check for hardware conflicts (pin assignments)

## Technical Specifications

### Power Consumption
- **ESP32**: ~160-200mA (base operation)
- **LED Strip**: 3-6A at 12V (when implemented)
- **Radio**: ~15mA (receive mode)

### Timing Constraints
- **Radio Reception**: Continuous listening for controller data
- **Display Updates**: Immediate when new data received
- **Link Timeout**: 10-second timeout detection
- **Status LED**: 2s blink (connected), 200ms blink (disconnected)