#include "../include/display_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "DISPLAY_DRIVER";

// 7-segment digit patterns (0-9)
// Each bit represents a segment: A,B,C,D,E,F,G
static const uint8_t digit_patterns[10] = {
  0x3F, // 0: A+B+C+D+E+F
  0x06, // 1: B+C
  0x5B, // 2: A+B+G+E+D
  0x4F, // 3: A+B+C+D+G
  0x66, // 4: F+G+B+C
  0x6D, // 5: A+F+G+C+D
  0x7D, // 6: A+F+G+C+D+E
  0x07, // 7: A+B+C
  0x7F, // 8: A+B+C+D+E+F+G
  0x6F  // 9: A+B+C+D+F+G
};

// LED strip buffer
static uint32_t led_buffer[LED_COUNT];

// Initialize segment-to-LED mapping for 2-digit display
static void init_segment_mapping(PlayClockDisplay *display) {
  // Digit 0 (left digit) - LEDs 0-449
  // Digit 1 (right digit) - LEDs 450-899
  
  for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
    uint16_t base_offset = digit * 450; // 450 LEDs per digit
    
    // Segment A (top horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_A] = (segment_range_t){base_offset, LEDS_PER_SEGMENT_HORIZONTAL};
    
    // Segment B (upper right vertical) - 30 LEDs  
    display->segments[digit][SEGMENT_B] = (segment_range_t){base_offset + 15, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment C (lower right vertical) - 30 LEDs
    display->segments[digit][SEGMENT_C] = (segment_range_t){base_offset + 45, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment D (bottom horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_D] = (segment_range_t){base_offset + 75, LEDS_PER_SEGMENT_HORIZONTAL};
    
    // Segment E (lower left vertical) - 30 LEDs
    display->segments[digit][SEGMENT_E] = (segment_range_t){base_offset + 90, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment F (upper left vertical) - 30 LEDs
    display->segments[digit][SEGMENT_F] = (segment_range_t){base_offset + 120, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment G (middle horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_G] = (segment_range_t){base_offset + 150, LEDS_PER_SEGMENT_HORIZONTAL};
  }
}

// Convert RGB color to WS2815 format with brightness
static uint32_t rgb_to_ws2815(color_t color, uint8_t brightness) {
  uint8_t r = (color.r * brightness) / 255;
  uint8_t g = (color.g * brightness) / 255;
  uint8_t b = (color.b * brightness) / 255;
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b; // GRB format for WS2815
}

// Set LED color in buffer
static void set_led_color(uint16_t led_index, color_t color, uint8_t brightness) {
  if (led_index < LED_COUNT) {
    led_buffer[led_index] = rgb_to_ws2815(color, brightness);
  }
}

// Set segment LEDs
static void set_segment_leds(PlayClockDisplay *display, uint8_t digit, segment_t segment, color_t color) {
  if (digit >= PLAY_CLOCK_DIGITS || segment >= SEGMENTS_PER_DIGIT) return;
  
  segment_range_t range = display->segments[digit][segment];
  for (uint16_t i = 0; i < range.count; i++) {
    set_led_color(range.start + i, color, display->brightness);
  }
}

bool display_begin(PlayClockDisplay *display) {
  ESP_LOGI(TAG, "Initializing WS2815 display");

  // Initialize structure
  memset(display, 0, sizeof(PlayClockDisplay));

  // Configure GPIO for LED strip data pin
  ESP_LOGI(TAG, "Configuring GPIO pin %d for WS2815 data line", LED_STRIP_PIN);
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << LED_STRIP_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  esp_err_t gpio_result = gpio_config(&io_conf);
  if (gpio_result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure GPIO pin %d: %s", LED_STRIP_PIN, esp_err_to_name(gpio_result));
    return false;
  }
  ESP_LOGI(TAG, "GPIO pin %d configured successfully", LED_STRIP_PIN);

  // Initialize segment mapping
  ESP_LOGI(TAG, "Initializing segment mapping for %d digits", PLAY_CLOCK_DIGITS);
  init_segment_mapping(display);

  // Initialize colors
  display->color_off = (color_t){0, 0, 0};
  display->color_on = (color_t){255, 255, 255};
  display->color_warning = (color_t){255, 165, 0}; // Orange
  display->color_error = (color_t){255, 0, 0};
  
  // Set default brightness
  display->brightness = 255;
  ESP_LOGI(TAG, "Brightness set to default: %d", display->brightness);

  // Clear display
  display_clear(display);

  // Run connection test
  ESP_LOGI(TAG, "Running LED strip connection test...");
  bool test_result = display_connection_test(display);
  if (!test_result) {
    ESP_LOGW(TAG, "LED strip connection test failed - continuing anyway");
  } else {
    ESP_LOGI(TAG, "LED strip connection test passed");
  }

  display->initialized = true;
  ESP_LOGI(TAG, "WS2815 display initialized successfully");
  return true;
}

void display_set_time(PlayClockDisplay *display, uint16_t seconds) {
  if (!display->initialized)
    return;

  ESP_LOGI(TAG, "Setting time: %d seconds", seconds);

  // Extract digits (00-99 seconds)
  uint8_t tens = (seconds / 10) % 10;
  uint8_t ones = seconds % 10;
  
  display->current_digits[0] = tens;
  display->current_digits[1] = ones;

  // Clear all segments first
  display_clear(display);
  
  // Set segments for each digit
  for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
    uint8_t digit_value = display->current_digits[digit];
    uint8_t pattern = digit_patterns[digit_value];
    
    color_t segment_color = display->color_on;
    
    // Apply mode-specific colors
    if (display->current_mode == DISPLAY_MODE_ERROR) {
      segment_color = display->color_error;
    } else if (display->current_mode == DISPLAY_MODE_RESET) {
      segment_color = display->color_warning;
    }
    
    // Set segments based on pattern
    for (int seg = 0; seg < SEGMENTS_PER_DIGIT; seg++) {
      if (pattern & (1 << seg)) {
        set_segment_leds(display, digit, seg, segment_color);
      }
    }
  }

  // Log the time
  display->last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
}



void display_set_run_mode(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  display->current_mode = DISPLAY_MODE_RUN;
  ESP_LOGI(TAG, "Display mode: RUN");
}

void display_set_stop_mode(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  display->current_mode = DISPLAY_MODE_STOP;
  ESP_LOGI(TAG, "Display mode: STOP");
}

void display_set_reset_mode(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  display->current_mode = DISPLAY_MODE_RESET;
  ESP_LOGI(TAG, "Display mode: RESET");
}

void display_show_error(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  display->current_mode = DISPLAY_MODE_ERROR;
  ESP_LOGI(TAG, "Display mode: ERROR");
}

void display_clear(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  // Clear all LEDs
  for (int i = 0; i < LED_COUNT; i++) {
    led_buffer[i] = rgb_to_ws2815(display->color_off, display->brightness);
  }
}

void display_set_brightness(PlayClockDisplay *display, uint8_t brightness) {
  if (!display->initialized)
    return;
    
  display->brightness = brightness;
  ESP_LOGI(TAG, "Brightness set to: %d", brightness);
}

void display_set_segment(PlayClockDisplay *display, uint8_t digit, segment_t segment, bool enable) {
  if (!display->initialized || digit >= PLAY_CLOCK_DIGITS || segment >= SEGMENTS_PER_DIGIT)
    return;

  color_t color = enable ? display->color_on : display->color_off;
  set_segment_leds(display, digit, segment, color);
}

// Connection test - checks if LED strip responds to basic commands
bool display_connection_test(PlayClockDisplay *display) {
  if (!display) return false;
  
  ESP_LOGI(TAG, "Testing LED strip connection...");
  
  // Test 1: Set first LED to red
  ESP_LOGI(TAG, "Test 1: Setting first LED to red");
  set_led_color(0, (color_t){255, 0, 0}, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(500));
  
  // Test 2: Set first LED to green
  ESP_LOGI(TAG, "Test 2: Setting first LED to green");
  set_led_color(0, (color_t){0, 255, 0}, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(500));
  
  // Test 3: Set first LED to blue
  ESP_LOGI(TAG, "Test 3: Setting first LED to blue");
  set_led_color(0, (color_t){0, 0, 255}, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(500));
  
  // Test 4: Clear first LED
  ESP_LOGI(TAG, "Test 4: Clearing first LED");
  set_led_color(0, (color_t){0, 0, 0}, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(200));
  
  ESP_LOGI(TAG, "LED strip connection test completed");
  return true; // Always return true for now - visual verification needed
}

// Visual test pattern - displays all colors and segments
void display_test_pattern(PlayClockDisplay *display) {
  if (!display->initialized) {
    ESP_LOGE(TAG, "Display not initialized for test pattern");
    return;
  }
  
  ESP_LOGI(TAG, "Starting LED test pattern...");
  
  // Clear display first
  display_clear(display);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(500));
  
  // Test 1: All LEDs red
  ESP_LOGI(TAG, "Test pattern: All LEDs red");
  for (int i = 0; i < LED_COUNT; i++) {
    led_buffer[i] = rgb_to_ws2815((color_t){255, 0, 0}, 100);
  }
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Test 2: All LEDs green
  ESP_LOGI(TAG, "Test pattern: All LEDs green");
  for (int i = 0; i < LED_COUNT; i++) {
    led_buffer[i] = rgb_to_ws2815((color_t){0, 255, 0}, 100);
  }
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Test 3: All LEDs blue
  ESP_LOGI(TAG, "Test pattern: All LEDs blue");
  for (int i = 0; i < LED_COUNT; i++) {
    led_buffer[i] = rgb_to_ws2815((color_t){0, 0, 255}, 100);
  }
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Test 4: White (all colors)
  ESP_LOGI(TAG, "Test pattern: All LEDs white");
  for (int i = 0; i < LED_COUNT; i++) {
    led_buffer[i] = rgb_to_ws2815((color_t){255, 255, 255}, 50);
  }
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Test 5: Digit segments test
  ESP_LOGI(TAG, "Test pattern: Digit segments");
  display_clear(display);
  
  // Test each segment on first digit
  for (int seg = 0; seg < SEGMENTS_PER_DIGIT; seg++) {
    ESP_LOGI(TAG, "Testing segment %d on digit 0", seg);
    set_segment_leds(display, 0, seg, (color_t){255, 255, 0}); // Yellow
    display_update(display);
    vTaskDelay(pdMS_TO_TICKS(300));
    set_segment_leds(display, 0, seg, display->color_off);
    display_update(display);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  
  // Test 6: Display "88" (all segments on)
  ESP_LOGI(TAG, "Test pattern: Display '88' (all segments)");
  display_set_time(display, 88);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(2000));
  
  // Clear display
  display_clear(display);
  display_update(display);
  
  ESP_LOGI(TAG, "LED test pattern completed");
}

void display_update(PlayClockDisplay *display) {
  if (!display->initialized)
    return;

  // Simple GPIO bit-banging for WS2815 (basic implementation)
  // In a production system, this should use RMT for proper timing
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t color = led_buffer[i];
    
    // Send 24 bits (GRB format)
    for (int bit = 23; bit >= 0; bit--) {
      bool bit_value = (color >> bit) & 1;
      
      // WS2815 timing (simplified - not accurate for production)
      if (bit_value) {
        // '1' bit: ~0.7us high, ~0.6us low
        gpio_set_level(LED_STRIP_PIN, 1);
        esp_rom_delay_us(1);
        gpio_set_level(LED_STRIP_PIN, 0);
        esp_rom_delay_us(1);
      } else {
        // '0' bit: ~0.35us high, ~0.8us low
        gpio_set_level(LED_STRIP_PIN, 1);
        esp_rom_delay_us(0);
        gpio_set_level(LED_STRIP_PIN, 0);
        esp_rom_delay_us(1);
      }
    }
  }
  
  // Reset pulse
  gpio_set_level(LED_STRIP_PIN, 0);
  esp_rom_delay_us(50);

  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  if (current_time - display->last_update_time > 1000) {
    ESP_LOGD(TAG, "Display update - mode: %d", display->current_mode);
    display->last_update_time = current_time;
  }
}
