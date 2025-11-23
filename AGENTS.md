# AGENTS.md - ESP32 Play Clock Development Guide

## Project Overview
- **Implementation**: Native C (converted from C++)
- **Framework**: ESP-IDF v6.1-dev
- **Target**: ESP32 microcontroller
- **Status**: Controller-driven display implementation
- **Data Source**: Receives seconds from controller via nRF24L01+

## Development Workflow

### ESP-IDF Environment Setup

Before running any ESP-IDF commands, you must activate the ESP-IDF environment:

```bash
# Activate ESP-IDF environment (configured in ~/.zshrc)
idf

# Then run build commands
idf.py build
```

### Environment Variables
The following are automatically configured in your `.zshrc`:
- `IDF_PATH=/home/linroot/esp-idf`
- `PATH="$IDF_PATH/tools:$PATH"`
- Custom `idf()` function that sources `$IDF_PATH/export.sh`

### Build Commands
```bash
# Build project
idf.py build

# Flash to device (DO NOT USE - build only)
# idf.py flash

# Monitor serial output (DO NOT USE - build only)
# idf.py monitor

# Build and flash in one command (DO NOT USE - build only)
# idf.py build flash monitor

# Clean build
idf.py fullclean

# Configure project
idf.py menuconfig
```

**IMPORTANT**: This project should only be built, never flashed to hardware. Use `idf.py build` only.

### Testing
```bash
# Build all tests
idf.py build

# Run specific test (edit test file app_main() to select test)
# Edit test/test_*.c files to run individual tests
# Flash test firmware:
# idf.py flash monitor
```

### Git Workflow (Recursive Commits)

Since this is a submodule of the parent scoreboard_clock repository, commits must be handled recursively:

```bash
# Step 1: Commit and push changes in the submodule
git add .
git commit -m "your commit message"
git push

# Step 2: Update parent repository to point to new commit
cd ..  # Go to parent repository
git add play_clock
git commit -m "Update play_clock submodule"
git push

# OR use the automated recursive workflow:
git submodule update --remote --merge
git add play_clock
git commit -m "Update submodule"
git push
```

**Important**: Always commit submodule changes first, then update the parent repository to reference the new commit. The parent repo only tracks which commit the submodule points to, not the submodule content directly.

For project overview, hardware specifications, and user documentation, see [README.md](README.md).

## Communication Protocol

### Data Format
The play clock receives 3-byte packets from the controller:
- **Byte 0**: Seconds high byte
- **Byte 1**: Seconds low byte  
- **Byte 2**: Sequence number

### SystemState Structure
```c
typedef struct {
  uint16_t seconds;        // 0-65535, combines bytes 0-1
  uint8_t sequence;        // 0-255, from byte 2
  uint32_t last_status_time; // Timestamp of last received message
  bool link_alive;         // Current connection status
} SystemState;
```

### Radio Configuration
- **Module**: nRF24L01+ (2.4 GHz)
- **Channel**: 76 (2.476 GHz)
- **Data Rate**: 1 Mbps
- **Payload Size**: 32 bytes (controller sends 3 bytes)
- **Auto-ACK**: Enabled for reliable transmission



## Code Style Guidelines

### Formatting
- 4-space indentation, no tabs
- Braces on same line for functions/methods
- Maximum line length: 120 characters

### Naming Conventions
- Constants: `UPPER_SNAKE_CASE` (e.g., `LED_STRIP_PIN`, `STATUS_TIMEOUT_MS`)
- Variables: `lower_snake_case` (e.g., `system_state`, `last_status_time`)
- Structs: `PascalCase` (e.g., `PlayClockDisplay`, `RadioComm`)
- Functions: `lower_snake_case()` (e.g., `display_begin()`, `radio_begin()`)
- Tags: `UPPER_SNAKE_CASE` (e.g., `TAG = "PLAY_CLOCK"`)

### Types & Headers
- Use standard C types: `uint8_t`, `uint16_t`, `bool`, `uint32_t`
- Header guards: `#pragma once`
- C headers: `<stdint.h>`, `<stdbool.h>`
- ESP-IDF headers: `#include "esp_log.h"`, `#include "freertos/FreeRTOS.h"`
- Project headers: `#include "display_driver.h"`, `#include "radio_comm.h"`

### Error Handling
- ESP-IDF logging: `ESP_LOGI(TAG, "message")`, `ESP_LOGE(TAG, "error")`
- Return `bool` for success/failure in initialization functions
- Test assertions: `TEST_ASSERT_TRUE(condition)`, `TEST_ASSERT_EQUAL(expected, actual)`
- Handle hardware failures gracefully with infinite loops or error states

### ESP-IDF Specific
- Entry point: `void app_main(void)`
- FreeRTOS delays: `vTaskDelay(pdMS_TO_TICKS(ms))`
- GPIO control: `gpio_set_direction(pin, GPIO_MODE_OUTPUT)`, `gpio_set_level(pin, level)`
- Memory: Prefer stack allocation, use `malloc()` only when necessary

### C Programming Patterns (Current Implementation)
- **Struct-based design** instead of classes (fully implemented)
- **Pass struct pointer** as first parameter to functions
- **Use `typedef struct`** for type definitions
- **Initialize structs with `memset()`** where appropriate
- **Native ESP-IDF C APIs** for all hardware interactions
- **Pure display logic** - only shows controller data, no local timing logic
- **Controller-driven** operation with nRF24L01+ radio communication
- **Link monitoring** - tracks connection status with timeout detection
- **Status LED feedback** - visual indication of connection state