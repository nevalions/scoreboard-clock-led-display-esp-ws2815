#pragma once

#include "../../radio-common/include/radio_common.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <stdbool.h>
#include <stdint.h>

// System state structure
typedef struct {
  uint16_t seconds;
  uint8_t sequence;
  uint32_t last_status_time;
  bool link_alive;
} SystemState;

// Use RadioCommon from radio_common.h instead of RadioComm
typedef RadioCommon RadioComm;

// Function declarations
bool radio_begin(RadioComm *radio, gpio_num_t ce, gpio_num_t csn);
bool radio_receive_message(RadioComm *radio, SystemState *state);
void radio_start_listening(RadioComm *radio);
void radio_stop_listening(RadioComm *radio);
bool radio_is_data_available(RadioComm *radio);
void radio_flush_rx(RadioComm *radio);

// Use radio_common functions for low-level operations
// uint8_t nrf24_read_register(RadioCommon* radio, uint8_t reg);
// bool nrf24_write_register(RadioCommon* radio, uint8_t reg, uint8_t value);
// bool nrf24_read_payload(RadioCommon* radio, uint8_t* data, uint8_t length);
// bool nrf24_write_payload(RadioCommon* radio, uint8_t* data, uint8_t length);
// uint8_t nrf24_get_status(RadioCommon* radio);
// void nrf24_power_up(RadioCommon* radio);
// void nrf24_power_down(RadioCommon* radio);

// Debug functions
void radio_dump_registers(RadioComm *radio);
