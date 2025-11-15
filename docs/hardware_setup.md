# Hardware Setup Guide

## Overview

This guide provides step-by-step instructions for assembling the Play Clock Module hardware components.

## Tools Required

### Essential Tools
- Soldering iron (25-30W)
- Solder (60/40 rosin core, 0.8mm)
- Wire strippers
- Multimeter
- Small Phillips screwdriver
- Needle-nose pliers
- Wire cutters

### Optional Tools
- Logic analyzer
- Oscilloscope
- Helping hands tool
- Heat gun (for heat shrink)

## Component Preparation

### 1. ESP32 Development Board
- Remove from packaging
- Verify all pins are straight
- Check for any visible damage
- Note pin layout orientation

### 2. nRF24L01+ Radio Module
- Inspect antenna connector
- Verify all pins are present
- Check module orientation marking
- Optional: Add pin headers if not pre-soldered

### 3. WS2815 LED Strips
- Measure and cut to required lengths:
  - Digit 1: 100cm (tens place)
  - Digit 2: 100cm (units place)
- Strip ~3mm insulation from wire ends
- Tin wire ends with solder
- Test each strip with 12V power

## Assembly Steps

### Step 1: Power Preparation

#### 12V Power Supply Setup
```bash
# Safety first - unplug all power supplies
# Verify 12V output with multimeter
# Prepare power injection points for LED strips
```

1. **Prepare 12V Power Connections**
   - Cut 18 AWG red wire for positive (+12V)
   - Cut 18 AWG black wire for ground (GND)
   - Strip 5mm insulation from ends
   - Tin wire ends with solder

2. **Install Power Capacitor**
   - Solder 1000µF capacitor across 12V and GND
   - Observe polarity: positive stripe to +12V
   - Place capacitor close to LED strip connection point

#### 5V Power Supply Setup
1. **Connect ESP32 Power**
   - Use USB cable or barrel jack connector
   - Verify 5V output with multimeter
   - Ensure stable power before proceeding

### Step 2: Radio Module Installation

#### nRF24L01+ Module Wiring
```
ESP32 Pin    →    nRF24L01+ Pin
GPIO22 (CE)  →    CE
GPIO21 (CSN) →    CSN
GPIO18 (SCK) →    SCK
GPIO23 (MOSI)→    MOSI
GPIO19 (MISO)→    MISO
3V3          →    VCC
GND          →    GND
```

1. **SPI Connection Preparation**
   - Cut 22 AWG wires for SPI connections
   - Use different colors for each signal:
     - Yellow: SCK (clock)
     - Green: MOSI (data out)
     - Blue: MISO (data in)
     - White: CSN (chip select)
     - Red: CE (chip enable)
     - Black: GND
     - Orange: 3V3 power

2. **Solder Radio Module**
   - If using breadboard: Insert radio module with pin headers
   - If direct solder: Solder wires directly to module pads
   - Double-check connections against pin diagram
   - Secure module with double-sided tape

### Step 3: Logic Level Shifter Setup

#### 3.3V to 5V Conversion
```
ESP32 Side (3.3V)    →    LED Side (5V)
GPIO5 (Data)         →    LED Data Input
3V3                  →    LV (Low Voltage)
5V (from LED supply) →    HV (High Voltage)
GND                  →    GND (Common)
```

1. **Install Logic Level Shifter**
   - Place on breadboard or PCB
   - Connect 3V3 to LV pin
   - Connect 5V to HV pin
   - Connect ground to GND pin
   - Connect GPIO5 to LV channel 1
   - Connect HV channel 1 to LED data line

### Step 4: LED Strip Installation

#### 7-Segment Layout Configuration

For each digit (100cm LED strip):

```
Segment Layout (approximately 30 LEDs per vertical segment):
    ┌───── A (15 LEDs) ─────┐
    │                       │
 F (25 LEDs)           B (25 LEDs)
    │                       │
    ├───── G (15 LEDs) ─────┤
    │                       │
 E (25 LEDs)           C (25 LEDs)
    │                       │
    └───── D (15 LEDs) ─────┘
```

1. **Prepare LED Strips**
   - Test each strip with 12V power before installation
   - Verify all LEDs light up
   - Note data input direction (arrow on strip)

2. **Create 7-Segment Pattern**
   - Mark segment boundaries on strip backing
   - Plan wire routing for each segment
   - Leave extra wire for connections

3. **Power Injection Setup**
   - Solder 12V positive to both ends of each strip
   - Solder ground to both ends of each strip
   - Use 18 AWG wire for power connections
   - Add solder blobs for secure connections

4. **Data Line Connection**
   - Connect logic level shifter output to first LED data input
   - Connect backup data (BI) to same point for reliability
   - Use 22 AWG wire for data connections
   - Keep data wire as short as possible

### Step 5: ESP32 Board Setup

#### Final ESP32 Connections

1. **Power Connections**
   - Ensure 5V power supply connected
   - Verify 3.3V regulator output (if used)
   - Connect all ground points together

2. **GPIO Pin Verification**
   - GPIO4: LED power control (optional)
   - GPIO5: LED data line
   - GPIO18: SPI SCK
   - GPIO19: SPI MISO
   - GPIO21: SPI CSN
   - GPIO22: Radio CE
   - GPIO23: SPI MOSI

3. **Mount ESP32 Board**
   - Secure in enclosure with standoffs
   - Ensure good ventilation
   - Keep away from high-power components

## Wiring Organization

### Cable Management

1. **Group Similar Wires**
   - Power wires together (red/black)
   - SPI wires together (color-coded)
   - Data wires separate from power

2. **Secure Connections**
   - Use cable ties or wire loom
   - Leave slack for thermal expansion
   - Avoid sharp bends in wires

3. **Label Critical Connections**
   - Power supplies
   - Data lines
   - Radio connections

### Testing Points

Create accessible test points for:
- 12V power (before and after capacitor)
- 5V power at ESP32
- 3.3V power at radio module
- SPI signals (for debugging)
- LED data signal

## Safety Considerations

### Electrical Safety

1. **Power Isolation**
   - Keep AC and DC circuits separate
   - Use insulated tools
   - Work on one circuit at a time

2. **Current Protection**
   - Consider inline fuses for power supplies
   - Use appropriate wire gauges
   - Monitor for overheating

3. **Polarity Verification**
   - Double-check all power connections
   - Use multimeter to verify polarity
   - Mark positive connections clearly

### Physical Safety

1. **Secure Mounting**
   - Mount all components securely
   - Use appropriate fasteners
   - Consider vibration resistance

2. **Ventilation**
   - Provide adequate airflow
   - Keep power supplies away from heat-sensitive components
   - Monitor operating temperatures

## Initial Power-On

### Pre-Power Checklist

1. **Visual Inspection**
   - Check for loose wires
   - Verify no shorts between power and ground
   - Ensure all connections match diagram

2. **Continuity Testing**
   - Test power connections with multimeter
   - Verify ground continuity
   - Check for unintended connections

3. **Resistance Check**
   - Measure resistance between power and ground
   - Should be high (no direct short)
   - Investigate any low resistance readings

### Power-On Sequence

1. **Apply 5V Power First**
   - Connect ESP32 power supply
   - Look for status LED on ESP32
   - Check for normal boot sequence

2. **Test Radio Module**
   - Verify 3.3V at radio module
   - Check for SPI communication
   - Monitor for radio activity

3. **Apply 12V Power**
   - Connect LED strip power
   - Verify LED strips light up
   - Check for proper data reception

## Troubleshooting Guide

### Common Issues

**No Power to ESP32:**
- Check 5V power supply output
- Verify USB/Barrel jack connection
- Check fuse (if installed)
- Test power supply with multimeter

**Radio Module Not Responding:**
- Verify 3.3V power at module
- Check SPI connections
- Ensure common ground
- Test with known working radio module

**LED Strips Not Working:**
- Verify 12V power at strips
- Check data signal at logic level shifter
- Test individual strips with direct power
- Verify ground connections

**Flickering or Random LED Behavior:**
- Check power supply capacity
- Add larger capacitor
- Verify all ground connections
- Reduce data line length

**System Resets:**
- Check 5V power supply capacity
- Monitor current consumption
- Check for short circuits
- Verify stable power connections

## Performance Optimization

### Signal Integrity

1. **SPI Signal Quality**
   - Keep SPI wires short (<15cm)
   - Use twisted pairs for long runs
   - Add 100Ω series resistors if needed
   - Shield cables if interference present

2. **LED Data Signal**
   - Use appropriate wire gauge (22-24 AWG)
   - Keep data line away from power cables
   - Add 33Ω series resistor at source
   - Use shielded cable for long runs

### Power Optimization

1. **Reduce Power Consumption**
   - Lower LED brightness in software
   - Use efficient power supplies
   - Enable power saving modes in ESP32
   - Monitor current draw

2. **Improve Power Stability**
   - Use larger capacitors
   - Add voltage regulators if needed
   - Implement soft-start circuits
   - Monitor voltage levels

## Maintenance

### Regular Checks

1. **Visual Inspection**
   - Check for loose connections
   - Look for heat damage
   - Verify secure mounting

2. **Performance Monitoring**
   - Monitor radio signal strength
   - Check LED brightness consistency
   - Verify power consumption

3. **Cleaning**
   - Clean dust from components
   - Check for corrosion
   - Ensure good ventilation

### Firmware Updates

1. **Update Procedure**
   - Backup current configuration
   - Upload new firmware via USB
   - Verify functionality after update
   - Document changes

2. **Testing After Updates**
   - Test all display modes
   - Verify radio communication
   - Check power consumption
   - Validate timing accuracy