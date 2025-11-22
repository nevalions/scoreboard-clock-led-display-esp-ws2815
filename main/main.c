#include "../include/display_driver.h"
#include "../include/radio_comm.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PLAY_CLOCK";

#define STATUS_LED_PIN GPIO_NUM_2
#define LINK_TIMEOUT_MS 10000

static PlayClockDisplay display;
static RadioComm radio;
static SystemState system_state;

static void setup(void) {
  ESP_LOGI(TAG, "Starting Play Clock Application");

  gpio_reset_pin(STATUS_LED_PIN);
  gpio_set_direction(STATUS_LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(STATUS_LED_PIN, 1);

  memset(&system_state, 0, sizeof(system_state));
  system_state.last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  if (!display_begin(&display)) {
    ESP_LOGE(TAG, "Failed to initialize display");
    while (1) {
      gpio_set_level(STATUS_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(100));
      gpio_set_level(STATUS_LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  if (!radio_begin(&radio, NRF24_CE_PIN, NRF24_CSN_PIN)) {
    ESP_LOGE(TAG, "Failed to initialize radio");
    display_show_error(&display);
    while (1) {
      gpio_set_level(STATUS_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(250));
      gpio_set_level(STATUS_LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(250));
    }
  }

  radio_start_listening(&radio);
  
  // Dump radio registers for debugging
  vTaskDelay(pdMS_TO_TICKS(100)); // Let radio settle
  radio_dump_registers(&radio);
  
  display_set_stop_mode(&display);

  ESP_LOGI(TAG, "Play Clock initialized successfully");
}

static void loop(void) {
  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  // Enable debug logging temporarily
  esp_log_level_set("RADIO_COMM", ESP_LOG_DEBUG);

  if (radio_receive_message(&radio, &system_state)) {
    // Update display with controller data
    display_set_time(&display, system_state.seconds);
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

  display_update(&display);

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
    // Fast blink when link is lost
    led_state = (current_time % 500) < 250;
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
