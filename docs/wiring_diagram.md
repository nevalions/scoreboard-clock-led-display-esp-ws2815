# Play Clock Module - Wiring Diagram

## Overview

This document provides complete wiring instructions for the Play Clock Module, which displays seconds (00-99) using two large 7-segment displays powered by WS2815 LED strips.

## System Components

### Hardware Required
- ESP32 Development Board
- nRF24L01+ or SX1278 Radio Module (868 MHz)
- WS2815 12V LED Strips (60 LEDs/m, 2 meters total)
- 5V to 3.3V Logic Level Shifter
- 12V 5A Power Supply (for LED strips)
- 5V 2A Power Supply (for ESP32)
- 1000µF 16V Capacitor (LED power stabilization)
- 10µF 16V Capacitor (radio power stabilization)
- Breadboard or PCB for connections

## Pin Configuration

### ESP32 GPIO Pin Assignments
```cpp
// From radio_comm.h
#define RADIO_CE_PIN    GPIO_NUM_22   // Radio Chip Enable
#define RADIO_CSN_PIN   GPIO_NUM_21   // Radio Chip Select (SPI CS)
#define RADIO_MOSI_PIN  GPIO_NUM_23   // SPI Master Out Slave In
#define RADIO_MISO_PIN  GPIO_NUM_19   // SPI Master In Slave Out
#define RADIO_SCK_PIN   GPIO_NUM_18   // SPI Clock

// From display_driver.h
#define LED_DATA_PIN    GPIO_NUM_5    // WS2815 Data Line
#define LED_POWER_PIN   GPIO_NUM_4    // LED Strip Power Control
```

## Complete Wiring Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        12V POWER SUPPLY (5A)                  │
└─────────────────────┬───────────────────────────────────────────┘
                      │
            ┌─────────┴─────────┐
            │    1000µF 16V     │  (Power stabilization)
            └─────────┬─────────┘
                      │
        ┌─────────────┴─────────────┐
        │                           │
┌───────┴───────┐           ┌───────┴───────┐
│   LED STRIP   │           │   LED STRIP   │
│   Digit 1     │           │   Digit 2     │
│   (00-99)     │           │   (seconds)   │
│   100cm       │           │   100cm       │
└───────┬───────┘           └───────┬───────┘
        │                           │
        │ 12V + GND to both ends    │
        └─────────────┬─────────────┘
                      │
              (Power injection at both ends)

┌─────────────────────────────────────────────────────────────────┐
│                         ESP32 BOARD                           │
│                                                                 │
│  GPIO4  ────────┐                                              │
│  (LED PWR)      │                                              │
│                 │                                              │
│  GPIO5  ────────┼─────────────────────────────────────────────┐ │
│  (LED Data)     │                                             │ │
│                 │                                             │ │
│  GPIO18 ────────┼─────────────────────────────────────────────┼─┐
│  (SPI SCK)      │                                             │ │
│                 │                                             │ │
│  GPIO19 ────────┼─────────────────────────────────────────────┼─┤
│  (SPI MISO)     │                                             │ │
│                 │                                ┌─────────────┴─┐
│  GPIO21 ────────┼─────────────────────────────────┤  LOGIC LEVEL  │
│  (SPI CSN)      │                                 │  SHIFTER      │
│                 │                                 │  (5V↔3.3V)    │
│  GPIO22 ────────┼─────────────────────────────────┤               │
│  (Radio CE)     │                                 │ HV   LV       │
│                 │                                 │ 5V   3.3V     │
│  3V3     ───────┼─────────────────────────────────┤               │
│  (Radio PWR)    │                                 └───────┬───────┘
│                 │                                         │
│  GND     ───────┼─────────────────────────────────────────┼─────┐
│                 │                                         │     │
└─────────────────┴─────────────────────────────────────────┘     │
                                                                      │
┌─────────────────────────────────────────────────────────────────┐ │
│                     5V POWER SUPPLY (2A)                        │ │
└─────────────────────┬───────────────────────────────────────────┘ │
                      │                                           │
            ┌─────────┴─────────┐                                 │
            │   ESP32 USB/Power │                                 │
            │      (5V IN)      │                                 │
            └───────────────────┘                                 │
                                                                      │
┌─────────────────────────────────────────────────────────────────┐ │
│                    nRF24L01+ RADIO MODULE                        │ │
│                                                                 │ │
│  VCC  ──────────────────────────────────────────────────────────┤ │
│  (3.3V from ESP32)                                             │ │
│                                                                 │ │
│  GND  ──────────────────────────────────────────────────────────┤ │
│  (Common ground)                                               │ │
│                                                                 │ │
│  CE   ──────────────────────────────────────────────────────────┤ │
│  (GPIO22 from ESP32)                                           │ │
│                                                                 │ │
│  CSN  ──────────────────────────────────────────────────────────┤ │
│  (GPIO21 from ESP32)                                           │ │
│                                                                 │ │
│  SCK  ──────────────────────────────────────────────────────────┤ │
│  (GPIO18 from ESP32)                                           │ │
│                                                                 │ │
│  MOSI ──────────────────────────────────────────────────────────┤ │
│  (GPIO23 from ESP32)                                           │ │
│                                                                 │ │
│  MISO ──────────────────────────────────────────────────────────┤ │
│  (GPIO19 from ESP32)                                           │ │
│                                                                 │ │
│  IRQ  ──────────────────────────────────────────────────────────┤ │
│  (Optional - not used in current implementation)               │ │
└─────────────────────────────────────────────────────────────────┘ │
                                                                      │
┌─────────────────────────────────────────────────────────────────┐ │
│                   WS2815 LED STRIP CONNECTIONS                  │ │
│                                                                 │ │
│  Power Input:                                                   │ │
│  • 12V DC positive to LED strip +12V                           │ │
│  • Ground to LED strip GND (both ends)                         │ │
│                                                                 │ │
│  Data Input:                                                    │ │
│  • GPIO5 → Logic Level Shifter (3.3V→5V)                       │ │
│  • Logic Level Shifter → LED strip Data Input (DI)             │ │
│  • Backup Data (BI) connected to DI for reliability            │ │
│                                                                 │ │
│  Power Control:                                                 │ │
│  • GPIO4 controls LED strip power via MOSFET or relay           │ │
│  • Allows software power management                            │ │
└─────────────────────────────────────────────────────────────────┘ │
                                                                      │
└───────────────────────────────────────────────────────────────────┘
```

## Power Distribution Details

### 12V LED Strip Power
```
12V 5A Power Supply
    │
    ├─── 1000µF 16V Capacitor (close to LED strips)
    │
    ├─── LED Strip Digit 1 (100cm) ──── Power injection ────┐
    │                                                     │
    └─── LED Strip Digit 2 (100cm) ──── Power injection ────┘
    │
    └─── Common ground connected to ESP32 ground
```

### 5V ESP32 Power
```
5V 2A Power Supply
    │
    └─── ESP32 Development Board
          │
          ├─── 3.3V regulator → Radio module
          ├─── GPIO pins → Logic level shifter
          └─── Ground → Common ground system
```

## Signal Flow

### Radio Communication
```
Controller (868 MHz) → nRF24L01+ → SPI → ESP32 → System State Update
```

### Display Update
```
System State → ESP32 GPIO5 → Logic Level Shifter → WS2815 → LED Display
```

## Grounding Scheme

**Critical:** All grounds must be connected together:
- ESP32 ground
- Radio module ground
- Logic level shifter ground
- LED strip ground (both ends)
- Power supply grounds

```
Common Ground Bus:
┌─────────────────────────────────────────────────┐
│ ESP32 GND ──┐                                   │
│             ├─── Radio Module GND               │
│ Logic Level ├─── Logic Shifter GND              │
│ Shifter GND ──── LED Strip GND (both ends)      │
│             └─── Power Supplies GND             │
└─────────────────────────────────────────────────┘
```

## Safety Considerations

### Electrical Safety
1. **Power Isolation:** Keep 12V and 5V circuits separate except for ground
2. **Current Capacity:** Use 18-20 AWG wire for 12V LED power connections
3. **Fuse Protection:** Consider 3A fuse on 12V LED power line
4. **Polarity:** Double-check all power connections before applying power

### Signal Integrity
1. **Capacitors:** Place 1000µF capacitor close to LED strip power input
2. **Short Wires:** Keep SPI wires short (<15cm) for reliable communication
3. **Shielding:** Consider twisted pair for long SPI runs
4. **Level Shifting:** Always use level shifter for 3.3V→5V signals

## Testing Procedure

### Power-On Test
1. Verify all connections with multimeter
2. Connect 5V power to ESP32 only - check for normal boot
3. Connect 12V power to LED strips - check for proper illumination
4. Test radio communication with controller

### Signal Test
1. Use logic analyzer or oscilloscope to verify SPI signals
2. Check LED data signal at logic level shifter output
3. Verify radio module reception with known controller

## Troubleshooting

### Common Issues

**LED Strips Not Working:**
- Check 12V power supply and connections
- Verify data signal at logic level shifter
- Check ground connections
- Test with single LED strip first

**Radio Communication Issues:**
- Verify 3.3V power to radio module
- Check SPI connections (MOSI, MISO, SCK, CSN, CE)
- Ensure common ground
- Test with different radio channel

**Flickering LEDs:**
- Add larger capacitor (2000µF) at LED power input
- Check for loose connections
- Verify 12V power supply capacity
- Reduce LED brightness in software

**System Resets:**
- Check 5V power supply capacity
- Ensure proper grounding
- Monitor power consumption
- Check for short circuits

## Bill of Materials

| Component | Quantity | Specifications |
|-----------|----------|----------------|
| ESP32 Dev Board | 1 | 30-pin version recommended |
| nRF24L01+ Module | 1 | With PA/LNA for range |
| WS2815 LED Strip | 2m | 60 LEDs/m, 12V DC |
| Logic Level Shifter | 1 | 4-channel, bidirectional |
| 12V Power Supply | 1 | 5A minimum, DC barrel jack |
| 5V Power Supply | 1 | 2A minimum, USB or barrel jack |
| 1000µF Capacitor | 1 | 16V electrolytic |
| 10µF Capacitor | 1 | 16V electrolytic |
| Wire | Various | 18-20 AWG for power, 22-24 AWG for signals |
| Breadboard/PCB | 1 | For mounting components |

## Assembly Notes

1. **Mounting:** Consider mounting ESP32 and radio module away from LED strips to reduce RF interference
2. **Cable Management:** Use cable ties or wire loom to organize wiring
3. **Ventilation:** Ensure adequate ventilation for power supplies and ESP32
4. **Weather Protection:** If outdoor use, consider waterproof enclosure for electronics