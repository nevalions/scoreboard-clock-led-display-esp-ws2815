# Play Clock Module

ESP32-based wireless play clock display for scoreboard systems. Shows seconds (SS) on large 7-segment LED displays and receives timing data wirelessly from a central controller via nRF24L01+ radio.

## Overview

The Play Clock module is a pure display unit that:
- Displays seconds (00-99) on large LED digits using WS2815 LED strips
- Receives timing data wirelessly via nRF24L01+ radio communication
- Shows only controller data without local timing logic
- Operates as a simple display peripheral with status monitoring
- Includes built-in test functionality via boot button
- Features refactored codebase with improved maintainability

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
| LED Strip Data | GPIO13 | WS2815 LED strip data line |
| Test Button | GPIO0 | Boot button (active low) |

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
- **Built-in Testing**: Number cycling test via boot button (GPIO0)
- **LED Test Pattern**: Hardware verification pattern on startup
- **Display Modes**: Stop, Run, Reset, and Error modes with visual indicators

### Communication Protocol
- **Radio Module**: nRF24L01+ (2.4 GHz) wireless communication
- **Data Format**: 3-byte payload [seconds_high, seconds_low, sequence]
- **Channel**: 76 (2.476 GHz)
- **Data Rate**: 1 Mbps
- **Payload Size**: 32 bytes (controller sends 3 bytes)
- **Auto-ACK**: Enabled for reliable transmission
- **Pure Display**: Shows received seconds without local processing
- **Radio Library**: Uses radio_common library for nRF24L01+ operations

### Status LED Behavior
- **Connected**: Slow blink (2 second period - 1s on, 1s off)
- **Disconnected**: Fast blink (200ms period - 100ms on, 100ms off)
- **Link Timeout**: 10 seconds without received data triggers disconnection
- **Link Recovery**: Automatic when data reception resumes
- **Hardware Error**: Fast blink (250ms period) if radio initialization fails
- **Display Error**: Shows error pattern on LED strips if display fails

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
- **Radio Library**: radio_common (shared nRF24L01+ driver)
- **LED Driver**: ESP-IDF LED Strip driver for WS2815
- **Architecture**: Struct-based design with pure display logic
- **Code Quality**: Refactored with named constants, helper functions, and improved maintainability

## Code Architecture

### Recent Improvements
The codebase has been refactored for improved maintainability:
- **Named Constants**: All magic numbers extracted to descriptive constants
- **Helper Functions**: Repeated operations consolidated into reusable functions
- **Variable Naming**: Consistent and descriptive naming conventions
- **Code Duplication**: Eliminated repetitive patterns in test functions
- **Type Safety**: Added const correctness where appropriate

### Key Constants
- **WS2815 Timing**: `WS2815_BIT_ONE_HIGH_US`, `WS2815_RESET_PULSE_US`
- **Test Patterns**: `TEST_COLOR_DELAY_MS`, `TEST_SEGMENT_DELAY_MS`
- **LED Offsets**: `SEGMENT_A_OFFSET`, `LEDS_PER_DIGIT`
- **Brightness Levels**: `TEST_COLOR_BRIGHTNESS`, `TEST_WHITE_BRIGHTNESS`

### Helper Functions
- `fill_all_leds()` - Efficient LED buffer filling
- `send_ws2815_bit()` - Encapsulated timing-critical bit sending
- `test_single_led_color()` - Reusable color testing
- `test_all_leds_color()` - Simplified color pattern testing

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
- **Radio Debug**: Radio register dump available in serial logs
- **Button Test**: Press boot button (GPIO0) to run number cycling test
- **LED Test**: Automatic test pattern runs on startup for verification

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