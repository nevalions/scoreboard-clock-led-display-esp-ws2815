# ESP32 Play Clock Complete Wiring Guide

## Overview
The ESP32 Play Clock consists of three main components:
- **ESP32 Development Board**: Main controller
- **nRF24L01+ Radio Module**: Receives timing data from controller
- **WS2815 LED Strip**: 7-segment display for showing seconds (00-99)

## Complete Wiring Diagram

```
                    ┌─────────────────┐
                    │   ESP32 Board   │
                    │                 │
                    │     3.3V ───────┼───┐
                    │     GND ────────┼───┼───┐
                    │    GPIO2 ───────┼───┼───┼───┐
                    │    GPIO0 ───────┼───┼───┼───┼───┐
                    │    GPIO5 ───────┼───┼───┼───┼───┼───┐
                    │    GPIO4 ───────┼───┼───┼───┼───┼───┼───┐
                    │   GPIO13 ───────┼───┼───┼───┼───┼───┼───┼───┐
                    │   GPIO18 ───────┼───┼───┼───┼───┼───┼───┼───┼───┐
                    │   GPIO19 ───────┼───┼───┼───┼───┼───┼───┼───┼───┼───┐
                    │   GPIO23 ───────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┐
                    └─────────────────┘   │   │   │   │   │   │   │   │   │
                                        │   │   │   │   │   │   │   │   │
                    ┌─────────────────┐ │   │   │   │   │   │   │   │   │
                    │   nRF24L01+     │ │   │   │   │   │   │   │   │   │
                    │                 │ │   │   │   │   │   │   │   │   │
                    │      VCC ───────┼─┘   │   │   │   │   │   │   │   │
                    │      GND ───────┼─────┘   │   │   │   │   │   │   │
                    │       CE ───────┼─────────┘   │   │   │   │   │   │
                    │      CSN ───────┼─────────────┘   │   │   │   │   │
                    │      SCK ───────┼─────────────────┘   │   │   │   │
                    │      MISO ──────┼─────────────────────┘   │   │   │
                    │      MOSI ──────┼─────────────────────────┘   │   │
                    │       IRQ ───────┼─────────────────────────────┘   │
                    └─────────────────┘                                 │
                                                                        │
                    ┌─────────────────┐                                 │
                    │   WS2815 Strip  │                                 │
                    │                 │                                 │
                    │      VCC ───────┼─────────────────────────────────┘
                    │      GND ───────┼───────────────────────────────────┘
                    │      DIN ───────┼────────────────────────────────────┘
                    └─────────────────┘

                    ┌─────────────────┐
                    │  Status LED     │
                    │                 │
                    │      VCC ───────┼────────────────────────────────────┐
                    │      GND ───────┼───────────────────────────────────┐│
                    │     (+) ───────┼──────────────────────────────────┐││
                    │     (-) ───────┼─────────────────────────────────┐│││
                    └─────────────────┘                                 ││││
                                                                      ││││
                    ┌─────────────────┐                               ││││
                    │  Test Button    │                               ││││
                    │  (Boot Button)  │                               ││││
                    │                 │                               ││││
                    │   GPIO0 ───────┼───────────────────────────────┘│││
                    │      GND ───────┼─────────────────────────────────┘││
                    └─────────────────┘                                   ││
                                                                        ││
                    ┌─────────────────┐                                 ││
                    │  Power Supply   │                                 ││
                    │     3.3V        │                                 ││
                    │                 │                                 ││
                    │     +3.3V ──────┼─────────────────────────────────┘│
                    │      GND ───────┼───────────────────────────────────┘
                    └─────────────────┘
```

## Component Connection Details

### 1. nRF24L01+ Radio Module
| nRF24L01+ Pin | ESP32 Pin | Function | Description |
|---------------|-----------|----------|-------------|
| VCC | 3.3V | Power | 3.3V power supply |
| GND | GND | Ground | Common ground |
| CE | GPIO5 | Chip Enable | Controls transmit/receive mode |
| CSN | GPIO4 | SPI Chip Select | SPI slave select |
| SCK | GPIO18 | SPI Clock | Serial clock (SPI2_HOST) |
| MOSI | GPIO23 | SPI Master Out | Data from ESP32 to nRF24L01+ |
| MISO | GPIO19 | SPI Master In | Data from nRF24L01+ to ESP32 |
| IRQ | (Not Connected) | Interrupt | Data available interrupt (optional) |

### 2. WS2815 LED Strip
| WS2815 Pin | ESP32 Pin | Function | Description |
|------------|-----------|----------|-------------|
| VCC | 3.3V | Power | 3.3V power for LEDs |
| GND | GND | Ground | Common ground |
| DIN | GPIO13 | Data In | WS2815 data signal |

### 3. Status LED
| LED Pin | ESP32 Pin | Function | Description |
|---------|-----------|----------|-------------|
| Anode (+) | GPIO5 | LED Positive | Status indicator (via 220Ω resistor) |
| Cathode (-) | GND | LED Negative | Common ground |

### 4. Test Button (Boot Button)
| Button Pin | ESP32 Pin | Function | Description |
|------------|-----------|----------|-------------|
| One Side | GPIO0 | Button Input | Boot button (active-low, internal pullup) |
| Other Side | GND | Ground | Common ground |

## Power Requirements

### Main Power Supply
- **Voltage**: 3.3V regulated
- **Current**: Minimum 2A for full LED strip brightness
- **Recommendation**: Use dedicated 3.3V power supply for LED strip

### Component Power Consumption
- **ESP32**: ~200mA (peak)
- **nRF24L01+**: ~15mA (transmit), ~12mA (receive)
- **WS2815 Strip**: ~60mA per LED at full brightness (900 LEDs = ~54A max!)
- **Status LED**: ~20mA

**Important**: The WS2815 strip can draw significant current. Use appropriate power supply and consider current limiting for full brightness operation.

## Configuration Details

### Radio Configuration
- **SPI Host**: SPI2_HOST
- **Clock Speed**: 1 MHz
- **SPI Mode**: 0 (CPOL=0, CPHA=0)
- **Channel**: 76 (2.476 GHz)
- **Data Rate**: 1 Mbps
- **Power**: 0 dBm
- **CRC**: Enabled
- **Payload Size**: 32 bytes
- **RX Address**: 0xE7E7E7E7E7

### LED Strip Configuration
- **LED Type**: WS2815 (12V compatible, but using 3.3V in this design)
- **Total LEDs**: 900 (450 per digit × 2 digits)
- **Data Format**: GRB (Green-Red-Blue)
- **Control**: GPIO bit-banging (basic implementation)
- **Brightness**: 0-255 (adjustable)

### Display Configuration
- **Digits**: 2 (00-99 seconds display)
- **Segments per Digit**: 7 (standard 7-segment)
- **LEDs per Segment**: 15 (horizontal) or 30 (vertical)
- **Colors**: Orange (normal), Yellow (warning), Red (error)

## Hardware Features

### Button Testing
- **Boot Button (GPIO0)**: Active-low with internal pullup
- **Debounce**: 50ms software debounce
- **Function**: Triggers number cycling test (00-99)

### Status Indicators
- **Status LED (GPIO2)**: Visual link status indicator
  - Slow blink (2s period): Link alive
  - Fast blink (200ms period): Link lost
- **Display Modes**: Stop, Run, Reset, Error with different colors

### Built-in Tests
- **LED Test Pattern**: Runs on startup for hardware verification
- **Number Cycling**: Button-activated test (00-99 display)
- **Connection Test**: Basic LED strip communication test

## Troubleshooting

### Radio Issues
1. **No Communication**: Check VCC is 3.3V, not 5V
2. **Intermittent Connection**: Add 10µF decoupling capacitor near nRF24L01+
3. **Short Range**: Check antenna placement and power supply stability
4. **SPI Errors**: Verify all SPI connections and ensure proper grounding

### LED Strip Issues
1. **No LEDs On**: Check 3.3V power supply and data connection
2. **Wrong Colors**: Verify GRB format and color definitions
3. **Flickering**: Check power supply capacity and connections
4. **First LED Only**: Check data signal integrity and timing

### General Issues
1. **Boot Loop**: Check power supply stability
2. **Button Not Working**: Verify GPIO0 connection and pullup
3. **Status LED Not Blinking**: Check GPIO2 connection and LED polarity

## Assembly Notes

1. **Power First**: Connect all power and ground connections before signal wires
2. **Decoupling**: Add 10µF capacitor near nRF24L01+ VCC/GND pins
3. **LED Strip**: Use appropriate gauge wire for high current LED strip
4. **Antenna**: Keep nRF24L01+ antenna away from metal objects and LED strip power lines
5. **Testing**: Use built-in test patterns to verify connections before full operation

## Safety Considerations

- **Voltage**: Never connect 5V to nRF24L01+ or ESP32 pins
- **Current**: LED strip can draw high current - use appropriate power supply
- **ESD**: Use proper ESD protection when handling components
- **Heat**: Monitor LED strip temperature at high brightness