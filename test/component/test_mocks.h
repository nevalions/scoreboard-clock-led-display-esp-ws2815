#pragma once

#include <stdint.h>
#include <stdbool.h>

// Mock definitions for testing without hardware
#define LED_STRIP_PIN 13
#define LED_COUNT 900
#define PLAY_CLOCK_DIGITS 2
#define SEGMENTS_PER_DIGIT 7

// Mock segment indices
typedef enum {
    SEGMENT_A = 0,
    SEGMENT_B = 1,
    SEGMENT_C = 2,
    SEGMENT_D = 3,
    SEGMENT_E = 4,
    SEGMENT_F = 5,
    SEGMENT_G = 6
} segment_t;

// Mock display modes
typedef enum {
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_STOP,
    DISPLAY_MODE_RUN,
    DISPLAY_MODE_RESET,
    DISPLAY_MODE_ERROR,
    DISPLAY_MODE_LINK_WARNING
} display_mode_t;

// Mock SystemState for testing
struct SystemState {
    uint8_t display_state;
    uint16_t seconds;
    uint8_t sequence;
    uint32_t last_status_time;
    bool link_alive;
};

// Mock StatusFrame for testing
struct __attribute__((packed)) StatusFrame {
    uint8_t frame_type;
    uint8_t state;
    uint16_t seconds;
    uint16_t ms_lowres;
    uint8_t sequence;
    uint8_t crc8;
};

// Mock constants
#define STATUS_FRAME_TYPE   0xA1
#define NRF24_CHANNEL     100
#define GPIO_NUM_NC       -1