# AGENTS.md - ESP32 Play Clock Development Guide

## Project Overview
- **Implementation**: Native C (converted from C++)
- **Framework**: ESP-IDF v6.1-dev
- **Target**: ESP32 microcontroller
- **Status**: Controller-driven display implementation
- **Data Source**: Receives seconds from controller via nRF24L01+
- **Architecture**: Struct-based design with radio_common library integration
- **Display**: WS2815 LED strips with 7-segment mapping
- **Testing**: Built-in button-activated number cycling test

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

### PlayClockDisplay Structure
```c
typedef struct {
  bool initialized;
  display_mode_t current_mode;
  uint32_t last_update_time;
  led_strip_t *led_strip;
  uint8_t brightness;
  segment_range_t segments[PLAY_CLOCK_DIGITS][SEGMENTS_PER_DIGIT];
  color_t color_off, color_on, color_warning, color_error;
  uint8_t current_digits[PLAY_CLOCK_DIGITS];
} PlayClockDisplay;
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
- **Button debouncing** - proper debouncing for test button input
- **LED segment mapping** - 7-segment display with WS2815 LED strips
- **Display modes** - Stop, Run, Reset, Error modes with visual patterns
- **Hardware testing** - built-in test patterns and number cycling

### Code Quality and Refactoring Guidelines

#### Constants and Magic Numbers
- **Extract all magic numbers** to named constants with descriptive names
- **Use descriptive prefixes** for constant groups (e.g., `WS2815_`, `TEST_`, `SEGMENT_`)
- **Group related constants** together in logical sections
- **Use appropriate units** in constant names (e.g., `_MS`, `_US` for timing)

```c
// Good examples
#define WS2815_BIT_ONE_HIGH_US 1
#define TEST_COLOR_DELAY_MS 1000
#define SEGMENT_A_OFFSET 0
#define LEDS_PER_DIGIT 450
```

#### Helper Functions
- **Extract repeated operations** into reusable helper functions
- **Use descriptive function names** that clearly indicate purpose
- **Keep functions focused** on single responsibilities
- **Prefer static functions** for file-scope helpers

```c
// Good examples
static void fill_all_leds(color_t color, uint8_t brightness);
static void send_ws2815_bit(bool bit_value);
static void test_single_led_color(PlayClockDisplay *display, color_t color, const char* color_name);
```

#### Variable Naming
- **Use descriptive names** that clearly indicate purpose and type
- **Add suffixes for clarity** (e.g., `_ms` for milliseconds, `_state` for booleans)
- **Be consistent** with naming patterns across the codebase
- **Avoid abbreviations** unless widely understood

```c
// Good examples
PlayClockDisplay play_clock_display;  // Not just "display"
uint32_t last_button_press_time_ms;   // Not just "last_time"
bool button_pressed_state;             // Not just "pressed"
```

#### Code Duplication
- **Eliminate repetitive patterns** through helper functions
- **Consolidate similar operations** into parameterized functions
- **Use loops and iteration** instead of repeated code blocks
- **Create test helpers** for common testing patterns

#### Type Safety
- **Add const correctness** to read-only function parameters
- **Use appropriate types** for values (e.g., `uint32_t` for timestamps)
- **Prefer explicit types** over implicit conversions
- **Use enum types** for related constants

### Recent Refactoring Examples

#### Constants Extraction
```c
// Before: Magic numbers scattered throughout code
esp_rom_delay_us(1);  // What does this mean?
vTaskDelay(pdMS_TO_TICKS(200));  // Why 200ms?

// After: Named constants with clear meaning
esp_rom_delay_us(WS2815_BIT_ONE_HIGH_US);
vTaskDelay(pdMS_TO_TICKS(NUMBER_CYCLE_DELAY_MS));
```

#### Helper Function Creation
```c
// Before: Repeated LED filling code
for (int i = 0; i < LED_COUNT; i++) {
  led_buffer[i] = rgb_to_ws2815((color_t){255, 0, 0}, 100);
}

// After: Reusable helper function
fill_all_leds((color_t){255, 0, 0}, TEST_COLOR_BRIGHTNESS);
```

#### Variable Naming Improvement
```c
// Before: Generic names
static PlayClockDisplay display;
static RadioComm radio;
static uint32_t last_button_press_time;

// After: Descriptive names
static PlayClockDisplay play_clock_display;
static RadioComm nrf24_radio;
static uint32_t last_button_press_time_ms;
```

### Key Implementation Details

#### Main Application Structure
```c
void app_main(void) {
  setup();  // Initialize hardware and radio
  while (1) {
    loop(); // Main processing loop
  }
}
```

#### Display Driver Features
- **7-segment mapping**: Each digit uses 7 segments with specific LED ranges
- **LED strip buffer**: 32-bit color buffer for WS2815 communication
- **Brightness control**: Adjustable brightness (0-255)
- **Test patterns**: Hardware verification and number cycling tests
- **Error handling**: Visual error patterns for hardware failures

#### Radio Communication
- **radio_common integration**: Uses shared nRF24L01+ driver library
- **FIFO monitoring**: Checks RX FIFO for incoming data
- **Register debugging**: Radio register dump for troubleshooting
- **Auto-ACK**: Enabled for reliable data transmission

#### Button Testing
- **Boot button (GPIO0)**: Active-low input with pullup
- **Debouncing**: 50ms debounce time for reliable detection
- **Number cycling**: Displays 00-99 sequence for testing
- **Hardware verification**: LED test pattern on startup

### Memory Safety and C Programming Best Practices

#### Memory Management
- **Stack allocation preferred** over dynamic allocation for predictable timing
- **Fixed-size buffers** for LED operations to prevent overflow
- **Bounds checking** in all array access operations
- **Static allocation** for large data structures (LED buffer, display state)

#### Common C Issues to Avoid
- **Integer overflow** in timestamp calculations (use appropriate types)
- **Buffer overflow** in LED operations (validate array indices)
- **Race conditions** in interrupt handling (disable interrupts during critical sections)
- **Stack overflow** in FreeRTOS tasks (monitor stack usage)
- **Magic numbers** without clear meaning (use named constants)

#### Defensive Programming
- **Null pointer checks** in all public functions
- **Parameter validation** before dereferencing pointers
- **Error handling** with graceful degradation
- **Logging** for debugging and troubleshooting
- **State validation** before hardware operations

#### Performance Considerations
- **Timing-critical sections** for WS2815 communication
- **Interrupt management** during LED data transmission
- **Efficient algorithms** for LED buffer operations
- **Memory locality** for frequently accessed data
- **Minimize blocking operations** in main loop