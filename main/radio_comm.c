#include "../include/radio_comm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

static const char *TAG = "RADIO_COMM";

bool radio_begin(RadioComm *radio, gpio_num_t ce, gpio_num_t csn) {
  ESP_LOGI(TAG, "Initializing nRF24L01+ radio using radio_common");

  // Use radio_common initialization
  if (!radio_common_init(radio, ce, csn)) {
    ESP_LOGE(TAG, "Radio common initialization failed");
    return false;
  }

  // Use radio_common configuration
  if (!radio_common_configure(radio)) {
    ESP_LOGE(TAG, "Radio common configuration failed");
    return false;
  }

  ESP_LOGI(TAG, "nRF24L01+ initialized successfully using radio_common");
  return true;
}



bool radio_receive_message(RadioComm *radio, SystemState *state) {
  if (!radio->initialized) {
    ESP_LOGE(TAG, "Radio not initialized");
    return false;
  }

  uint8_t status = nrf24_get_status(radio);
  ESP_LOGD(TAG, "Radio status: 0x%02X", status);
  
  // Check if RX FIFO has data (even if RX_DR flag isn't set)
  uint8_t fifo_status = nrf24_read_register(radio, NRF24_REG_FIFO_STATUS);
  bool rx_fifo_empty = (fifo_status & 0x01) != 0;
  ESP_LOGD(TAG, "FIFO status: 0x%02X, RX empty: %s", fifo_status, rx_fifo_empty ? "yes" : "no");
  
  if (status & NRF24_STATUS_RX_DR) {
    uint8_t payload[RADIO_PAYLOAD_SIZE];
    nrf24_read_payload(radio, payload, RADIO_PAYLOAD_SIZE);
    
    // Clear RX_DR flag
    nrf24_write_register(radio, NRF24_REG_STATUS, NRF24_STATUS_RX_DR);
    
    // Parse payload (format: seconds(2), sequence(1))
    state->seconds = (payload[0] << 8) | payload[1];
    state->sequence = payload[2];
    state->last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    state->link_alive = true;
    
    ESP_LOGI(TAG, "Message received: seconds=%d, seq=%d",
             state->seconds, state->sequence);
    return true;
  }

  return false;
}

void radio_start_listening(RadioComm *radio) {
  if (!radio->initialized)
    return;

  // Ensure we're in RX mode
  gpio_set_level(radio->ce_pin, 0);
  nrf24_write_register(radio, NRF24_REG_CONFIG, RADIO_CONFIG_RX_MODE);
  vTaskDelay(pdMS_TO_TICKS(2)); // Small delay to ensure mode switch
  
  // Clear any pending RX flags
  nrf24_write_register(radio, NRF24_REG_STATUS, RADIO_STATUS_CLEAR_ALL);
  
  // Flush RX FIFO to start fresh
  nrf24_flush_rx(radio);
  
  // Start listening
  gpio_set_level(radio->ce_pin, 1);
  
  // Log current configuration for debugging
  uint8_t config = nrf24_read_register(radio, NRF24_REG_CONFIG);
  uint8_t rf_ch = nrf24_read_register(radio, NRF24_REG_RF_CH);
  uint8_t rf_setup = nrf24_read_register(radio, NRF24_REG_RF_SETUP);
  uint8_t en_aa = nrf24_read_register(radio, NRF24_REG_EN_AA);
  uint8_t setup_retr = nrf24_read_register(radio, NRF24_REG_SETUP_RETR);
  
  ESP_LOGI(TAG, "Starting radio listening");
  ESP_LOGI(TAG, "Config: 0x%02X, Ch: %d, RF: 0x%02X, EN_AA: 0x%02X, RETR: 0x%02X", 
           config, rf_ch, rf_setup, en_aa, setup_retr);
}

void radio_stop_listening(RadioComm *radio) {
  if (!radio->initialized)
    return;

  gpio_set_level(radio->ce_pin, 0);
  ESP_LOGI(TAG, "Stopping radio listening");
}

bool radio_is_data_available(RadioComm *radio) {
  uint8_t status = nrf24_get_status(radio);
  return (status & NRF24_STATUS_RX_DR);
}

void radio_flush_rx(RadioComm *radio) {
  nrf24_flush_rx(radio);
  ESP_LOGD(TAG, "Flushing RX buffer");
}

void radio_dump_registers(RadioComm* radio) {
  radio_common_dump_registers(radio);
}
