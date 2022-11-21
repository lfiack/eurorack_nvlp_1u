#ifndef __NVLP_H
#define __NVLP_H

#include <stdint.h>

/* Attack and Decay durations in tick (so it depends on timer configuration. I suggest 1ms) */
#define NVLP_ATTACK_MIN 1
#define NVLP_ATTACK_MAX 3000
#define NVLP_DECAY_MIN 1
#define NVLP_DECAY_MAX 3000

/* NVLP potentiometers scans through 4 different behaviors:
 * 1. From 0 to NVLP_ADC_DECAY_MAX, Rising time is minimal and falling increases to max value
 * 2. From NVLP_ADC_DECAY_MAX to NVLP_ADC_ATTACK_MAX, Falling time is maximal and Rising time increases to max value
 * 3. From NVLP_ADC_ATTACK_MAX to NVLP_ADC_DECAY_MIN, Rising time is maximal and Falling time decreases to min value
 * 4. From NVLP_ADC_DECAY_MIN to NVLP_ADC_ATTACK_MIN (max ADC value), Falling time is minimal and Rising time decreases to min value.
 */
#define NVLP_ADC_DECAY_MAX  32767
#define NVLP_ADC_ATTACK_MAX   49151
#define NVLP_ADC_DECAY_MIN  57343
#define NVLP_ADC_ATTACK_MIN   65535

typedef enum nvlp_state_enum
{
    NVLP_STATE_REST,
    NVLP_STATE_ATTACK,
    NVLP_STATE_DECAY
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

    uint16_t attack;
    uint16_t decay;
    uint8_t gate_pin;
    nvlp_state_t state;
    uint16_t output;
} h_nvlp_t;

nvlp_status_t nvlp_init(h_nvlp_t * h_nvlp);
void nvlp_interrupt_callback(h_nvlp_t * h_nvlp);

#endif // __NVLP_H