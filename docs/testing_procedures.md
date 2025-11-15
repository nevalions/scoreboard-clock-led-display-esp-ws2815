# Testing Procedures Guide

## Overview

This guide provides comprehensive testing procedures for the Play Clock Module, from initial power-on to full system validation.

## Testing Equipment Required

### Essential Equipment
- Digital Multimeter
- Logic Analyzer (Saleae, PulseView, or similar)
- Power Supply with current monitoring
- Oscilloscope (optional but recommended)
- ESP32 Programmer/USB cable

### Software Tools
- ESP-IDF monitoring tools
- Serial terminal (minicom, PuTTY, etc.)
- Logic analyzer software
- RF spectrum analyzer app (for radio testing)

## Pre-Testing Checklist

### Safety Verification
- [ ] All power connections double-checked
- [ ] No exposed conductors
- [ ] Proper grounding verified
- [ ] Workspace clear and organized
- [ ] Emergency power cutoff accessible

### Component Verification
- [ ] ESP32 board powers on normally
- [ ] Radio module receives 3.3V power
- [ ] LED strips tested individually
- [ ] Logic level shifter functioning
- [ ] All connections match wiring diagram

## Phase 1: Power System Testing

### 1.1 ESP32 Power Test

```bash
# Test procedure:
1. Connect only 5V power to ESP32
2. Measure voltage at ESP32 VIN pin: should be 5.0V ±0.1V
3. Measure 3.3V regulator output: should be 3.3V ±0.05V
4. Check current draw: should be <100mA at idle
5. Verify power LED status
```

**Expected Results:**
- Voltage readings within specifications
- ESP32 boots normally (serial output visible)
- No overheating components
- Current draw within expected range

**Troubleshooting:**
- Low voltage: Check power supply output
- No 3.3V: Verify regulator functionality
- High current: Check for short circuits

### 1.2 Radio Module Power Test

```bash
# Test procedure:
1. With ESP32 powered, measure radio module VCC
2. Verify 3.3V at radio module power pin
3. Check ground continuity
4. Measure current draw: should be ~12-15mA at idle
```

**Expected Results:**
- Stable 3.3V at radio module
- Normal current consumption
- No excessive heating

### 1.3 LED Strip Power Test

```bash
# Test procedure:
1. Apply 12V power to LED strips only
2. Measure voltage at strip input: should be 12V ±0.5V
3. Check current draw with all LEDs white: ~3-4A per strip
4. Verify voltage drop across strip: <1V
5. Test power injection at both ends
```

**Expected Results:**
- Uniform brightness across entire strip
- No significant voltage drop
- Current within power supply capacity

## Phase 2: Communication Testing

### 2.1 SPI Communication Test

```cpp
// Test code for SPI communication
#include "driver/spi_master.h"
#include "driver/gpio.h"

void test_spi_communication() {
    // Initialize SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_NUM_23,
        .miso_io_num = GPIO_NUM_19,
        .sclk_io_num = GPIO_NUM_18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_bus_initialize(SPI2_HOST, &bus_cfg, 0);

    // Test SPI transaction
    spi_device_handle_t spi;
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = GPIO_NUM_21,
        .queue_size = 1
    };

    spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi);

    // Send test data
    uint8_t tx_data[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t rx_data[4];
    spi_transaction_t trans = {
        .length = 32,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };

    spi_device_transmit(spi, &trans);

    // Verify received data
    ESP_LOGI("TEST", "SPI Test: TX: %02X %02X %02X %02X, RX: %02X %02X %02X %02X",
             tx_data[0], tx_data[1], tx_data[2], tx_data[3],
             rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
}
```

### 2.2 Radio Module Test

```bash
# Manual radio test procedure:
1. Connect logic analyzer to SPI pins
2. Configure analyzer for SPI mode
3. Send radio initialization command
4. Observe SPI transactions
5. Verify radio registers are accessible
```

**Expected SPI Signals:**
- Clock frequency: ~1-2 MHz
- Proper chip select timing
- Data transmitted on MOSI
- Data received on MISO
- Clean signal transitions

### 2.3 RF Reception Test

```cpp
// Radio reception test
void test_radio_reception() {
    RadioComm radio;
    SystemState state;

    // Initialize radio
    if (!radio.begin(GPIO_NUM_22, GPIO_NUM_21)) {
        ESP_LOGE("TEST", "Radio initialization failed");
        return;
    }

    // Listen for messages (30 second test)
    for (int i = 0; i < 300; i++) {
        if (radio.receiveMessage(state)) {
            ESP_LOGI("TEST", "Received: state=%d, seconds=%d, seq=%d",
                     state.display_state, state.seconds, state.sequence);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

## Phase 3: Display Testing

### 3.1 LED Strip Control Test

```cpp
// Basic LED control test
void test_led_control() {
    PlayClockDisplay display;

    // Initialize display
    if (!display.begin()) {
        ESP_LOGE("TEST", "Display initialization failed");
        return;
    }

    // Test individual segments
    display.testSegments();

    // Test number display
    for (int i = 0; i < 100; i++) {
        display.setTime(i);
        display.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Test modes
    display.setStopMode();
    display.update();
    vTaskDelay(pdMS_TO_TICKS(2000));

    display.setRunMode();
    display.update();
    vTaskDelay(pdMS_TO_TICKS(2000));

    display.setResetMode();
    display.update();
    vTaskDelay(pdMS_TO_TICKS(2000));
}
```

### 3.2 Display Pattern Test

```bash
# Visual display test procedure:
1. Power on LED strips
2. Run display test pattern
3. Verify all segments light up
4. Check for dead or dim LEDs
5. Verify segment mapping is correct
6. Test brightness levels
7. Test color modes
```

**Expected Display Behavior:**
- All segments illuminate correctly
- Numbers display correctly (0-99)
- No flickering or flicker patterns
- Smooth transitions between states
- Appropriate brightness levels

## Phase 4: Integration Testing

### 4.1 End-to-End System Test

```cpp
// Complete system integration test
void test_system_integration() {
    ESP_LOGI("TEST", "Starting system integration test");

    // Initialize components
    PlayClockDisplay display;
    RadioComm radio;
    SystemState state;

    // Test initialization
    TEST_ASSERT_TRUE(display.begin());
    TEST_ASSERT_TRUE(radio.begin(GPIO_NUM_22, GPIO_NUM_21));

    // Test message processing
    int messages_received = 0;
    uint32_t test_start = xTaskGetTickCount() * portTICK_PERIOD_MS;

    while (messages_received < 10) {
        if (radio.receiveMessage(state)) {
            display.setTime(state.seconds);
            display.setLinkStatus(true);

            switch (state.display_state) {
                case 0: display.setStopMode(); break;
                case 1: display.setRunMode(); break;
                case 2: display.setResetMode(); break;
            }

            display.update();
            messages_received++;

            ESP_LOGI("TEST", "Message %d: %d seconds, state %d",
                     messages_received, state.seconds, state.display_state);
        }

        // Timeout after 30 seconds
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS - test_start) > 30000) {
            ESP_LOGW("TEST", "Test timeout after 30 seconds");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    ESP_LOGI("TEST", "Integration test completed. Messages received: %d", messages_received);
}
```

### 4.2 Performance Testing

```cpp
// Performance benchmark test
void test_system_performance() {
    PlayClockDisplay display;

    uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Test display update speed
    for (int i = 0; i < 1000; i++) {
        display.setTime(i % 100);
        display.update();
    }

    uint32_t end_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t elapsed_ms = end_time - start_time;

    ESP_LOGI("TEST", "1000 display updates in %d ms (avg: %0.2f ms/update)",
             elapsed_ms, (float)elapsed_ms / 1000.0);

    // Test should complete in <5 seconds
    TEST_ASSERT_LESS_THAN(5000, elapsed_ms);
}
```

## Phase 5: Stress Testing

### 5.1 Continuous Operation Test

```bash
# 24-hour stress test procedure:
1. Start continuous display cycling
2. Monitor system temperature
3. Log any errors or restarts
4. Verify radio link stability
5. Check for memory leaks
6. Monitor power consumption
```

### 5.2 Radio Range Test

```bash
# Range testing procedure:
1. Place module at known distance from controller
2. Measure received signal strength
3. Test message reliability
4. Increase distance gradually
5. Note maximum reliable range
6. Test with obstacles
```

## Test Results Documentation

### Test Report Template

```
Play Clock Module Test Report
============================

Date: [Date]
Tester: [Name]
Module ID: [Serial/ID]
Firmware Version: [Version]

Component Tests:
- ESP32 Power: PASS/FAIL - [Notes]
- Radio Module: PASS/FAIL - [Notes]
- LED Display: PASS/FAIL - [Notes]
- Logic Level Shifter: PASS/FAIL - [Notes]

Integration Tests:
- SPI Communication: PASS/FAIL - [Notes]
- Radio Reception: PASS/FAIL - [Notes]
- Display Control: PASS/FAIL - [Notes]
- System Integration: PASS/FAIL - [Notes]

Performance Tests:
- Display Update Speed: [ms/update]
- Radio Message Rate: [messages/second]
- Power Consumption: [mA] idle, [mA] active
- Operating Temperature: [°C]

Stress Tests:
- Continuous Operation: [hours] - [Notes]
- Radio Range: [meters] - [Notes]

Issues Found:
- [List any issues discovered]

Recommendations:
- [Any recommendations for improvement]

Overall Status: PASS/FAIL
```

## Automated Testing Script

### pytest Configuration

```python
# test_play_clock_hardware.py
import pytest
import serial
import time

class TestPlayClockHardware:
    def setup_method(self):
        self.serial_port = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)

    def teardown_method(self):
        self.serial_port.close()

    def test_esp32_boot(self):
        """Test ESP32 boots successfully"""
        response = self.send_command("BOOT_TEST")
        assert "OK" in response

    def test_radio_init(self):
        """Test radio module initialization"""
        response = self.send_command("RADIO_TEST")
        assert "RADIO_OK" in response

    def test_display_init(self):
        """Test LED display initialization"""
        response = self.send_command("DISPLAY_TEST")
        assert "DISPLAY_OK" in response

    def test_system_integration(self):
        """Test complete system integration"""
        response = self.send_command("INTEGRATION_TEST")
        assert "INTEGRATION_PASS" in response

    def send_command(self, command):
        self.serial_port.write((command + "\n").encode())
        time.sleep(0.1)
        return self.serial_port.read_all().decode()
```

## Troubleshooting Guide

### Common Test Failures

**Radio Module Not Responding:**
1. Check 3.3V power at module
2. Verify SPI connections
3. Test with known working module
4. Check for radio interference

**LED Display Issues:**
1. Verify 12V power supply
2. Check data signal with logic analyzer
3. Test individual LED strips
4. Verify logic level shifter operation

**System Instability:**
1. Monitor power supply voltage under load
2. Check for overheating components
3. Verify ground connections
4. Test with reduced current draw

**Performance Issues:**
1. Profile code execution time
2. Optimize display refresh rate
3. Check for memory fragmentation
4. Verify radio communication timing

## Quality Control

### Acceptance Criteria

- ✅ All individual component tests pass
- ✅ System integration test completes successfully
- ✅ Performance meets specifications
- ✅ No hardware errors during 24-hour stress test
- ✅ Radio range meets requirements (>50m line of sight)
- ✅ Power consumption within limits (<2A total)
- ✅ Operating temperature within specifications (-20°C to +60°C)

### Final Inspection

Before deployment, verify:
- All connections secure and insulated
- Component mounting is secure
- Labels and documentation complete
- Test report filled out
- Firmware version logged
- Module ID recorded