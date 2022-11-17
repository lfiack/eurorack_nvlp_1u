#ifndef __NVLP_H
#define __NVLP_H

#include <stdint.h>

/* We don't specify rising and falling time durations, but how much the output increments/decrements each step */
#define NVLP_INCREMENT_MIN 1
#define NVLP_INCREMENT_MAX 65535
#define NVLP_DECREMENT_MIN -1
#define NVLP_DECREMENT_MAX -65535

/* NVLP potentiometers scans through 4 different behaviors:
 * 1. From 0 to NVLP_ADC_FALLING_MAX, Rising time is minimal and falling increases to max value
 * 2. From NVLP_ADC_FALLING_MAX to NVLP_ADC_RISING_MAX, Falling time is maximal and Rising time increases to max value
 * 3. From NVLP_ADC_RISING_MAX to NVLP_ADC_FALLING_MIN, Rising time is maximal and Falling time decreases to min value
 * 4. From NVLP_ADC_FALLING_MIN to NVLP_ADC_RISING_MIN (max ADC value), Falling time is minimal and Rising time decreases to min value.
 */
#define NVLP_ADC_FALLING_MAX  32767
#define NVLP_ADC_RISING_MAX   49152
#define NVLP_ADC_FALLING_MIN  57343
#define NVLP_ADC_RISING_MIN   65535

typedef enum nvlp_state_enum
{
    NVLP_STATE_RESTING,
    NVLP_STATE_RISING,
    NVLP_STATE_FALLING
} nvlp_state_t;

typedef enum nvlp_status_enum
{
    NVLP_OK,
    NVLP_FAILED
} nvlp_status_t;

typedef uint8_t (* get_gate_pin_t)(void);
typedef uint16_t (* get_potentiometer_t)(void);
typedef void (* set_output_t)(uint16_t value);

typedef struct nvlp_driver_struct
{
    get_gate_pin_t get_gate_pin;
    get_potentiometer_t get_potentiometer;
    set_output_t set_output;
} nvlp_driver_t;

typedef struct h_nvlp_struct
{
    nvlp_driver_t * driver;

    uint8_t gate_pin;
    nvlp_state_t state;
    uint16_t output;
} h_nvlp_t;

nvlp_status_t nvlp_init(h_nvlp_t * h_nvlp);
void nvlp_timer_callback(h_nvlp_t * h_nvlp);

#endif // __NVLP_H