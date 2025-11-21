#include "../include/radio_comm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

static const char *TAG = "RADIO_COMM";

static uint8_t spi_transfer(RadioComm* radio, uint8_t data) {
  uint8_t rx_data;
  spi_transaction_t trans = {
    .length = 8,
    .tx_buffer = &data,
    .rx_buffer = &rx_data
  };
  spi_device_transmit(radio->spi, &trans);
  return rx_data;
}

uint8_t nrf24_read_register(RadioComm* radio, uint8_t reg) {
  gpio_set_level(radio->csn_pin, 0);
  spi_transfer(radio, NRF24_CMD_R_REGISTER | reg);
  uint8_t value = spi_transfer(radio, NRF24_CMD_NOP);
  gpio_set_level(radio->csn_pin, 1);
  return value;
}

bool nrf24_write_register(RadioComm* radio, uint8_t reg, uint8_t value) {
  gpio_set_level(radio->csn_pin, 0);
  spi_transfer(radio, NRF24_CMD_W_REGISTER | reg);
  spi_transfer(radio, value);
  gpio_set_level(radio->csn_pin, 1);
  return true;
}

uint8_t nrf24_get_status(RadioComm* radio) {
  gpio_set_level(radio->csn_pin, 0);
  uint8_t status = spi_transfer(radio, NRF24_CMD_NOP);
  gpio_set_level(radio->csn_pin, 1);
  return status;
}

bool radio_begin(RadioComm *radio, gpio_num_t ce, gpio_num_t csn) {
  ESP_LOGI(TAG, "Initializing nRF24L01+ radio");

  // Initialize structure
  memset(radio, 0, sizeof(RadioComm));

  radio->ce_pin = ce;
  radio->csn_pin = csn;

  // Initialize addresses
  uint8_t default_tx[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  uint8_t default_rx[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  memcpy(radio->tx_address, default_tx, 5);
  memcpy(radio->rx_address, default_rx, 5);

  // Configure GPIO pins
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << ce) | (1ULL << csn),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  gpio_config(&io_conf);

  gpio_set_level(ce, 0);
  gpio_set_level(csn, 1);

  // Configure SPI bus
  spi_bus_config_t bus_cfg = {
    .mosi_io_num = NRF24_MOSI_PIN,
    .miso_io_num = NRF24_MISO_PIN,
    .sclk_io_num = NRF24_SCK_PIN,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };

  spi_device_interface_config_t dev_cfg = {
    .clock_speed_hz = 1000000,
    .mode = 0,
    .spics_io_num = -1,
    .queue_size = 7
  };

  spi_bus_initialize(NRF24_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
  spi_bus_add_device(NRF24_SPI_HOST, &dev_cfg, &radio->spi);

  // Power up and configure nRF24L01+
  nrf24_write_register(radio, NRF24_REG_CONFIG, 0);
  vTaskDelay(pdMS_TO_TICKS(10));
  
  // Enable CRC, power up, primary receiver mode
  nrf24_write_register(radio, NRF24_REG_CONFIG, NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX);
  vTaskDelay(pdMS_TO_TICKS(5));

  // Set data rate to 1Mbps, 0dBm power
  nrf24_write_register(radio, NRF24_REG_RF_SETUP, 0x06);

  // Set channel (2.4GHz + channel)
  nrf24_write_register(radio, NRF24_REG_RF_CH, 76);

  // Set RX address width to 5 bytes
  nrf24_write_register(radio, NRF24_REG_SETUP_AW, 0x03);

  // Set RX address
  nrf24_write_register(radio, NRF24_REG_RX_ADDR_P0, radio->rx_address[0]);
  nrf24_write_register(radio, NRF24_REG_RX_ADDR_P0 + 1, radio->rx_address[1]);
  nrf24_write_register(radio, NRF24_REG_RX_ADDR_P0 + 2, radio->rx_address[2]);
  nrf24_write_register(radio, NRF24_REG_RX_ADDR_P0 + 3, radio->rx_address[3]);
  nrf24_write_register(radio, NRF24_REG_RX_ADDR_P0 + 4, radio->rx_address[4]);

  // Set payload size
  nrf24_write_register(radio, NRF24_REG_RX_PW_P0, NRF24_PAYLOAD_SIZE);

  // Enable pipe 0
  nrf24_write_register(radio, NRF24_REG_EN_RXADDR, 0x01);

  // Clear status flags
  nrf24_write_register(radio, NRF24_REG_STATUS, NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT);

  ESP_LOGI(TAG, "nRF24L01+ initialized successfully");
  radio->initialized = true;
  return true;
}

bool nrf24_read_payload(RadioComm* radio, uint8_t* data, uint8_t length) {
  gpio_set_level(radio->csn_pin, 0);
  spi_transfer(radio, NRF24_CMD_RX_PAYLOAD);
  for (uint8_t i = 0; i < length; i++) {
    data[i] = spi_transfer(radio, NRF24_CMD_NOP);
  }
  gpio_set_level(radio->csn_pin, 1);
  return true;
}

bool radio_receive_message(RadioComm *radio, SystemState *state) {
  if (!radio->initialized) {
    ESP_LOGE(TAG, "Radio not initialized");
    return false;
  }

  uint8_t status = nrf24_get_status(radio);
  
  if (status & NRF24_STATUS_RX_DR) {
    uint8_t payload[NRF24_PAYLOAD_SIZE];
    nrf24_read_payload(radio, payload, NRF24_PAYLOAD_SIZE);
    
    // Clear RX_DR flag
    nrf24_write_register(radio, NRF24_REG_STATUS, NRF24_STATUS_RX_DR);
    
    // Parse payload (assuming format: state(1), seconds(2), sequence(1))
    if (payload[0] <= 2) { // Valid state values
      state->display_state = payload[0];
      state->seconds = (payload[1] << 8) | payload[2];
      state->sequence = payload[3];
      state->last_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
      state->link_alive = true;
      
      ESP_LOGI(TAG, "Message received: state=%d, seconds=%d, seq=%d",
               state->display_state, state->seconds, state->sequence);
      return true;
    }
  }

  return false;
}

void radio_start_listening(RadioComm *radio) {
  if (!radio->initialized)
    return;

  gpio_set_level(radio->ce_pin, 0);
  nrf24_write_register(radio, NRF24_REG_CONFIG, NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX);
  gpio_set_level(radio->ce_pin, 1);
  ESP_LOGI(TAG, "Starting radio listening");
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
  gpio_set_level(radio->csn_pin, 0);
  spi_transfer(radio, NRF24_CMD_FLUSH_RX);
  gpio_set_level(radio->csn_pin, 1);
  ESP_LOGD(TAG, "Flushing RX buffer");
}

void nrf24_power_up(RadioComm* radio) {
  uint8_t config = nrf24_read_register(radio, NRF24_REG_CONFIG);
  nrf24_write_register(radio, NRF24_REG_CONFIG, config | NRF24_CONFIG_PWR_UP);
  vTaskDelay(pdMS_TO_TICKS(5));
}

void nrf24_power_down(RadioComm* radio) {
  uint8_t config = nrf24_read_register(radio, NRF24_REG_CONFIG);
  nrf24_write_register(radio, NRF24_REG_CONFIG, config & ~NRF24_CONFIG_PWR_UP);
}
