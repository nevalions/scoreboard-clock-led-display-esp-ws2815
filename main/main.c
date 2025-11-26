#include "../include/display_driver.h"
#include "../include/radio_comm.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PLAY_CLOCK";

#define STATUS_LED_PIN GPIO_NUM_2
#define TEST_BUTTON_PIN GPIO_NUM_0  // Boot button on ESP32
#define LINK_TIMEOUT_MS 10000
#define BUTTON_DEBOUNCE_MS 50
#define NUMBER_CYCLE_DELAY_MS 200
#define LONG_HOLD_MS 2000  // 2 seconds for long hold detection

static PlayClockDisplay play_clock_display;
static RadioComm nrf24_radio;
static SystemState system_state;

// Button state tracking
static uint32_t last_button_press_time_ms = 0;
static bool button_pressed_state = false;
static uint32_t button_hold_start_time_ms = 0;
static bool long_hold_triggered = false;

// Button debouncing and press detection
static bool is_button_pressed(void) {
  uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
  bool current_state = gpio_get_level(TEST_BUTTON_PIN) == 0; // Boot button is active low
  
  if (current_state && !button_pressed_state && 
      (current_time_ms - last_button_press_time_ms > BUTTON_DEBOUNCE_MS)) {
    button_pressed_state = true;
    button_hold_start_time_ms = current_time_ms;
    long_hold_triggered = false; // Reset long hold flag for new press
    last_button_press_time_ms = current_time_ms;
    ESP_LOGI(TAG, "Button pressed detected, long_hold_triggered reset to false");
    return true;
  }
  
  return false;
}

// Check for long hold (2+ seconds) - only triggers once per press
static bool is_button_long_held(void) {
  if (!button_pressed_state || long_hold_triggered) {
    return false;
  }
  
  uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t hold_time = current_time_ms - button_hold_start_time_ms;
  if (hold_time >= LONG_HOLD_MS) {
    ESP_LOGI(TAG, "Long hold detected, hold_time=%d ms", hold_time);
    long_hold_triggered = true;
    return true;
  }
  
  return false;
}

// Check for button release (to trigger short press action if no long hold)
static bool is_button_released(void) {
  bool current_state = gpio_get_level(TEST_BUTTON_PIN) == 0;
  
  // Button was pressed and now released
  if (button_pressed_state && !current_state) {
    ESP_LOGI(TAG, "Button release detected, long_hold_triggered=%d", long_hold_triggered);
    button_pressed_state = false;
    return !long_hold_triggered; // Only return true if long hold wasn't triggered
  }
  
  return false;
}

// Number cycling test - displays 00-99 on both digits
static void run_number_cycling_test(void) {
  ESP_LOGI(TAG, "Starting number cycling test (00-99)");
  
  for (int i = 0; i <= 99; i++) {
    display_set_time(&play_clock_display, i);
    display_update(&play_clock_display);
    
    ESP_LOGD(TAG, "Displaying: %02d", i);
    vTaskDelay(pdMS_TO_TICKS(NUMBER_CYCLE_DELAY_MS));
  }
  
  // Clear display after test
  display_clear(&play_clock_display);
  display_update(&play_clock_display);
  ESP_LOGI(TAG, "Number cycling test completed");
}

// White LED mode - all LEDs white until button released
static void run_white_led_mode(void) {
  ESP_LOGI(TAG, "Starting white LED mode (hold button)");
  
  display_set_all_white(&play_clock_display);
  
  // Wait until button is released
  while (gpio_get_level(TEST_BUTTON_PIN) == 0) {
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  
  // Clear display after mode
  display_clear(&play_clock_display);
  display_update(&play_clock_display);
  ESP_LOGI(TAG, "White LED mode completed");
}

static void setup(void) {
  ESP_LOGI(TAG, "Starting Play Clock Application");

  // Configure status LED
  gpio_reset_pin(STATUS_LED_PIN);
  gpio_set_direction(STATUS_LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(STATUS_LED_PIN, 1);
  
  // Configure test button (boot button)
  ESP_LOGI(TAG, "Configuring test button on GPIO %d", TEST_BUTTON_PIN);
  gpio_reset_pin(TEST_BUTTON_PIN);
  gpio_set_direction(TEST_BUTTON_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(TEST_BUTTON_PIN, GPIO_PULLUP_ONLY);

  memset(&system_state, 0, sizeof(system_state));
  system_state.last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  if (!display_begin(&play_clock_display)) {
    ESP_LOGE(TAG, "Failed to initialize display");
    while (1) {
      gpio_set_level(STATUS_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(100));
      gpio_set_level(STATUS_LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  // Run comprehensive display tests BEFORE radio initialization
  ESP_LOGI(TAG, "=== DISPLAY TESTING PHASE ===");
  display_set_stop_mode(&play_clock_display);
  
  ESP_LOGI(TAG, "Running LED test pattern for hardware verification...");
  display_test_pattern(&play_clock_display);
  
  ESP_LOGI(TAG, "Display testing completed - ready for operation");
  display_clear(&play_clock_display);
  display_update(&play_clock_display);
  vTaskDelay(pdMS_TO_TICKS(500)); // Brief pause before radio
  
  ESP_LOGI(TAG, "=== RADIO INITIALIZATION PHASE ===");

  if (!radio_begin(&nrf24_radio, RADIO_CE_PIN, RADIO_CSN_PIN)) {
    ESP_LOGE(TAG, "Failed to initialize radio");
    display_show_error(&play_clock_display);
    while (1) {
      gpio_set_level(STATUS_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(250));
      gpio_set_level(STATUS_LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(250));
    }
  }

  radio_start_listening(&nrf24_radio);
  
  // Dump radio registers for debugging
  vTaskDelay(pdMS_TO_TICKS(100)); // Let radio settle
  radio_dump_registers(&nrf24_radio);

  ESP_LOGI(TAG, "Play Clock initialized successfully");
}

static void loop(void) {
  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  // Check for button press (for debouncing and state tracking)
  if (is_button_pressed()) {
    ESP_LOGI(TAG, "Button press detected");
  }
  
  // Debug: Show button state every 5 seconds
  static uint32_t last_debug_time = 0;
  if (current_time - last_debug_time > 5000) {
    bool button_state = gpio_get_level(TEST_BUTTON_PIN) == 0;
    ESP_LOGI(TAG, "Debug: button_state=%d, button_pressed_state=%d", button_state, button_pressed_state);
    last_debug_time = current_time;
  }
  
  // Check for button long hold
  if (is_button_long_held()) {
    ESP_LOGI(TAG, "Button long hold detected - running white LED mode");
    run_white_led_mode();
  }
  
  // Check for button release (short press)
  if (is_button_released()) {
    ESP_LOGI(TAG, "Test button released - running number cycling test");
    run_number_cycling_test();
  }

  // Enable debug logging temporarily
  esp_log_level_set("RADIO_COMM", ESP_LOG_DEBUG);

  if (radio_receive_message(&nrf24_radio, &system_state)) {
    // Update display with controller data
    display_set_time(&play_clock_display, system_state.seconds);
    ESP_LOGI(TAG, "Time update: seconds=%d, seq=%d",
             system_state.seconds, system_state.sequence);
    // Update current_time after receiving message
    current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  }

  // Check for link timeout
  if (current_time - system_state.last_status_time > LINK_TIMEOUT_MS) {
    if (system_state.link_alive) {
      ESP_LOGW(TAG, "Link timeout detected");
      system_state.link_alive = false;
    }
  } else if (!system_state.link_alive && (current_time - system_state.last_status_time < LINK_TIMEOUT_MS)) {
    // Link recovered
    ESP_LOGI(TAG, "Link restored");
    system_state.link_alive = true;
  }

  display_update(&play_clock_display);

  // Status LED based on link status
  static bool led_state = false;
  if (system_state.link_alive) {
    // Slow blink when link is alive
    if (current_time % 2000 < 1000) {
      led_state = true;
    } else {
      led_state = false;
    }
  } else {
    // Very fast blink when link is lost
    led_state = (current_time % 200) < 100;
  }
  gpio_set_level(STATUS_LED_PIN, led_state ? 1 : 0);

  vTaskDelay(pdMS_TO_TICKS(50));
}

void app_main(void) {
  setup();

  while (1) {
    loop();
  }
}
