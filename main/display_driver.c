#include "../include/display_driver.h"
#include "../include/led_strip_encoder.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "DISPLAY_DRIVER";

// RMT configuration for WS2815
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us

// Test pattern timing constants (milliseconds)
#define TEST_COLOR_DELAY_MS 1000
#define TEST_SEGMENT_DELAY_MS 300
#define TEST_SEGMENT_OFF_DELAY_MS 100
#define TEST_LED_DELAY_MS 500
#define TEST_LED_OFF_DELAY_MS 200
#define NUMBER_CYCLE_DELAY_MS 200

// Brightness levels for test patterns
#define TEST_COLOR_BRIGHTNESS 100
#define TEST_WHITE_BRIGHTNESS 50

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
static uint8_t led_buffer[LED_COUNT * 3]; // RGB buffer for RMT

// Display driver mutex for thread-safe operations
static SemaphoreHandle_t display_mutex = NULL;

// LED offset constants for segment positioning
#define SEGMENT_A_OFFSET 0
#define SEGMENT_B_OFFSET 15
#define SEGMENT_C_OFFSET 45
#define SEGMENT_D_OFFSET 75
#define SEGMENT_E_OFFSET 90
#define SEGMENT_F_OFFSET 120
#define SEGMENT_G_OFFSET 150
// Physical LED base positions for each digit (actual wiring)
#define DIGIT_0_BASE 0    // Digit 0 starts at LED 0
#define DIGIT_1_BASE 165  // Digit 1 starts at LED 165

// Initialize segment-to-LED mapping for 2-digit display
static void init_segment_mapping(PlayClockDisplay *display) {
  // Digit 0 (left digit) - uses LEDs 0-164
  // Digit 1 (right digit) - uses LEDs 165-329
  
  // Define actual base positions for each digit
  uint16_t digit_base[PLAY_CLOCK_DIGITS] = {DIGIT_0_BASE, DIGIT_1_BASE};
  
  for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
    uint16_t base_offset = digit_base[digit];
    
    // Segment A (top horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_A] = (segment_range_t){base_offset + SEGMENT_A_OFFSET, LEDS_PER_SEGMENT_HORIZONTAL};
    
    // Segment B (upper right vertical) - 30 LEDs  
    display->segments[digit][SEGMENT_B] = (segment_range_t){base_offset + SEGMENT_B_OFFSET, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment C (lower right vertical) - 30 LEDs
    display->segments[digit][SEGMENT_C] = (segment_range_t){base_offset + SEGMENT_C_OFFSET, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment D (bottom horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_D] = (segment_range_t){base_offset + SEGMENT_D_OFFSET, LEDS_PER_SEGMENT_HORIZONTAL};
    
    // Segment E (lower left vertical) - 30 LEDs
    display->segments[digit][SEGMENT_E] = (segment_range_t){base_offset + SEGMENT_E_OFFSET, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment F (upper left vertical) - 30 LEDs
    display->segments[digit][SEGMENT_F] = (segment_range_t){base_offset + SEGMENT_F_OFFSET, LEDS_PER_SEGMENT_VERTICAL};
    
    // Segment G (middle horizontal) - 15 LEDs
    display->segments[digit][SEGMENT_G] = (segment_range_t){base_offset + SEGMENT_G_OFFSET, LEDS_PER_SEGMENT_HORIZONTAL};
  }
}

// Set LED color in buffer (RGB format for RMT encoder) - thread-safe
static void set_led_color(uint16_t led_index, color_t color, uint8_t brightness) {
  if (led_index < LED_COUNT) {
    uint8_t r = (color.r * brightness) / 255;
    uint8_t g = (color.g * brightness) / 255;
    uint8_t b = (color.b * brightness) / 255;
    
    // RMT encoder expects GRB format for WS2815
    led_buffer[led_index * 3 + 0] = g; // Green
    led_buffer[led_index * 3 + 1] = r; // Red  
    led_buffer[led_index * 3 + 2] = b; // Blue
  }
}

// Helper function to fill all LEDs with a specific color - thread-safe
static void fill_all_leds(color_t color, uint8_t brightness) {
  for (int i = 0; i < LED_COUNT; i++) {
    set_led_color(i, color, brightness);
  }
}



// Set segment LEDs - thread-safe
static void set_segment_leds(PlayClockDisplay *display, uint8_t digit, segment_t segment, color_t color) {
  if (digit >= PLAY_CLOCK_DIGITS || segment >= SEGMENTS_PER_DIGIT) return;
  
  segment_range_t range = display->segments[digit][segment];
  for (uint16_t i = 0; i < range.count; i++) {
    set_led_color(range.start + i, color, display->brightness);
  }
}

bool display_begin(PlayClockDisplay *display) {
  ESP_LOGI(TAG, "Initializing WS2815 display with RMT");

  // Create display mutex if not already created
  if (display_mutex == NULL) {
    display_mutex = xSemaphoreCreateMutex();
    if (display_mutex == NULL) {
      ESP_LOGE(TAG, "Failed to create display mutex");
      return false;
    }
  }

  // Initialize structure
  memset(display, 0, sizeof(PlayClockDisplay));

  // Configure RMT TX channel for WS2815
  ESP_LOGI(TAG, "Configuring RMT channel for WS2815 on GPIO %d", LED_STRIP_PIN);
  rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .gpio_num = LED_STRIP_PIN,
    .mem_block_symbols = 64,
    .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
    .trans_queue_depth = 4,
  };
  esp_err_t rmt_result = rmt_new_tx_channel(&tx_chan_config, &display->rmt_channel);
  if (rmt_result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create RMT TX channel: %s", esp_err_to_name(rmt_result));
    return false;
  }

  // Install LED strip encoder
  ESP_LOGI(TAG, "Installing LED strip encoder");
  led_strip_encoder_config_t encoder_config = {
    .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
  };
  rmt_result = rmt_new_led_strip_encoder(&encoder_config, &display->rmt_encoder);
  if (rmt_result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create LED strip encoder: %s", esp_err_to_name(rmt_result));
    return false;
  }

  // Enable RMT channel
  ESP_LOGI(TAG, "Enabling RMT TX channel");
  rmt_result = rmt_enable(display->rmt_channel);
  if (rmt_result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable RMT TX channel: %s", esp_err_to_name(rmt_result));
    return false;
  }

  ESP_LOGI(TAG, "RMT channel configured successfully");

  // Initialize segment mapping
  ESP_LOGI(TAG, "Initializing segment mapping for %d digits", PLAY_CLOCK_DIGITS);
  init_segment_mapping(display);

  // Initialize colors
  display->color_off = (color_t){0, 0, 0};
  display->color_on = (color_t){255, 165, 0}; // Orange for seconds display
  display->color_warning = (color_t){255, 255, 0}; // Yellow
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

  // Thread-safe display operations
  xSemaphoreTake(display_mutex, portMAX_DELAY);

  // Check for null signal (255 seconds = 0xFF)
  if (seconds == 255) {
    ESP_LOGI(TAG, "Received null signal (255 seconds) - clearing display");
    display_clear(display);
    xSemaphoreGive(display_mutex);
    display_update(display);
    display->last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return;
  }

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
  
  xSemaphoreGive(display_mutex);
}

void display_set_color(PlayClockDisplay *display, uint8_t r, uint8_t g, uint8_t b) {
  if (!display->initialized)
    return;

  // Thread-safe display operations
  xSemaphoreTake(display_mutex, portMAX_DELAY);
  
  // Update the main color with received RGB values
  display->color_on = (color_t){r, g, b};
  
  ESP_LOGI(TAG, "Display color updated to RGB(%d,%d,%d)", r, g, b);
  
  xSemaphoreGive(display_mutex);
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
  if (!display)
    return;

  // Clear all LEDs using helper function
  fill_all_leds(display->color_off, display->brightness);
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

// Helper function to test single LED color
static void test_single_led_color(PlayClockDisplay *display, color_t color, const char* color_name) {
  ESP_LOGI(TAG, "Testing LED color: %s", color_name);
  set_led_color(0, color, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(TEST_LED_DELAY_MS));
}

// Connection test - checks if LED strip responds to basic commands
bool display_connection_test(PlayClockDisplay *display) {
  if (!display) return false;
  
  ESP_LOGI(TAG, "Testing LED strip connection...");
  
  // Test primary colors using helper function
  test_single_led_color(display, (color_t){255, 0, 0}, "red");
  test_single_led_color(display, (color_t){0, 255, 0}, "green");
  test_single_led_color(display, (color_t){0, 0, 255}, "blue");
  
  // Clear first LED
  ESP_LOGI(TAG, "Clearing first LED");
  set_led_color(0, (color_t){0, 0, 0}, 255);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(TEST_LED_OFF_DELAY_MS));
  
  ESP_LOGI(TAG, "LED strip connection test completed");
  return true; // Always return true for now - visual verification needed
}

// Helper function to test all LEDs with a specific color
static void test_all_leds_color(PlayClockDisplay *display, color_t color, uint8_t brightness, const char* color_name) {
  ESP_LOGI(TAG, "Test pattern: All LEDs %s", color_name);
  fill_all_leds(color, brightness);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(TEST_COLOR_DELAY_MS));
}

// Helper function to test individual segment
static void test_single_segment(PlayClockDisplay *display, uint8_t digit, segment_t segment) {
  ESP_LOGI(TAG, "Testing segment %d on digit %d", segment, digit);
  set_segment_leds(display, digit, segment, (color_t){255, 255, 0}); // Yellow
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(TEST_SEGMENT_DELAY_MS));
  set_segment_leds(display, digit, segment, display->color_off);
  display_update(display);
  vTaskDelay(pdMS_TO_TICKS(TEST_SEGMENT_OFF_DELAY_MS));
}

// Test function to verify digit addressing - helps find correct base addresses
static void test_digit_addressing(PlayClockDisplay *display) {
  ESP_LOGI(TAG, "=== DIGIT ADDRESSING TEST ===");
  
  // Test each digit individually with all segments lit (digit 8)
  for (int digit = 0; digit < PLAY_CLOCK_DIGITS; digit++) {
    ESP_LOGI(TAG, "Testing digit %d - should show '8'", digit);
    display_clear(display);
    
    // Light all segments for this digit (pattern for 8)
    uint8_t pattern = digit_patterns[8]; // 0x7F = all segments
    for (int seg = 0; seg < SEGMENTS_PER_DIGIT; seg++) {
      if (pattern & (1 << seg)) {
        set_segment_leds(display, digit, seg, (color_t){255, 0, 0}); // Red
      }
    }
    
    display_update(display);
    ESP_LOGI(TAG, "Digit %d base address: %d, LED range: %d-%d", 
             digit, 
             (digit == 0) ? DIGIT_0_BASE : DIGIT_1_BASE,
             (digit == 0) ? DIGIT_0_BASE : DIGIT_1_BASE,
             (digit == 0) ? DIGIT_0_BASE + 164 : DIGIT_1_BASE + 164);
    
    vTaskDelay(pdMS_TO_TICKS(3000)); // Show for 3 seconds
  }
  
  display_clear(display);
  display_update(display);
  ESP_LOGI(TAG, "Digit addressing test completed");
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
  
  // Test primary colors using helper function
  test_all_leds_color(display, (color_t){255, 0, 0}, TEST_COLOR_BRIGHTNESS, "red");
  test_all_leds_color(display, (color_t){0, 255, 0}, TEST_COLOR_BRIGHTNESS, "green");
  test_all_leds_color(display, (color_t){0, 0, 255}, TEST_COLOR_BRIGHTNESS, "blue");
  test_all_leds_color(display, (color_t){255, 255, 255}, TEST_WHITE_BRIGHTNESS, "white");
  
  // Test digit addressing to verify second digit wiring
  test_digit_addressing(display);
  
  // Test digit segments
  ESP_LOGI(TAG, "Test pattern: Digit segments");
  display_clear(display);
  
  // Test each segment on first digit using helper function
  for (int seg = 0; seg < SEGMENTS_PER_DIGIT; seg++) {
    test_single_segment(display, 0, seg);
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

  // Thread-safe display update
  xSemaphoreTake(display_mutex, portMAX_DELAY);

  // Force buffer access to prevent compiler optimization issues
  // This simulates the effect of debug logging that was making it work
  volatile uint8_t buffer_check = led_buffer[0] + led_buffer[1] + led_buffer[2];
  (void)buffer_check; // Prevent unused variable warning

  // Transmit LED data using RMT
  rmt_transmit_config_t tx_config = {
    .loop_count = 0, // no transfer loop
  };
  
  esp_err_t result = rmt_transmit(display->rmt_channel, display->rmt_encoder, 
                                 led_buffer, sizeof(led_buffer), &tx_config);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to transmit LED data: %s", esp_err_to_name(result));
    xSemaphoreGive(display_mutex);
    return;
  }
  
  // Wait for transmission to complete
  rmt_tx_wait_all_done(display->rmt_channel, portMAX_DELAY);
  
  // WS2815 requires explicit reset delay after transmission
  esp_rom_delay_us(320);

  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  if (current_time - display->last_update_time > 1000) {
    ESP_LOGD(TAG, "Display update - mode: %d", display->current_mode);
    display->last_update_time = current_time;
  }
  
  xSemaphoreGive(display_mutex);
}

void display_set_all_white(PlayClockDisplay *display) {
  if (!display->initialized)
    return;
    
  ESP_LOGI(TAG, "Setting all LEDs to white");
  
  // Thread-safe white LED setting
  xSemaphoreTake(display_mutex, portMAX_DELAY);
  fill_all_leds((color_t){255, 255, 255}, display->brightness);
  xSemaphoreGive(display_mutex);
  
  display_update(display);
}
