#include "nvlp.h"

/* Attente de front sur la GATE
 * Lors d'un front sur la GATE -> interruption
 * Passe la sortie à la valeur max (12 bits? 16 bits? -> Peut-être 16 bits left-aligned pour ne pas dépendre du DAC)
 * Idem pour l'ADC du coup
 * À chaque interruption d'ADC (toutes les millisecondes), on décrémente la sortie
 * Rythme de descente (valeur à décrémenter) en fonction du potard
 * Si re-front, alors re-trig de l'enveloppe
 * Au-delà d'un certain seuil (50%?, réglable?), on ne passe pas direct à 1, mais on monte
 */

void compute_rising(h_nvlp_t * h_nvlp);
void compute_falling(h_nvlp_t * h_nvlp);

nvlp_status_t nvlp_init(h_nvlp_t * h_nvlp)
{
    h_nvlp->gate_pin = 0;
    h_nvlp->output = 0;
    h_nvlp->state = NVLP_STATE_RESTING;

    return NVLP_OK;
}

void nvlp_interrupt_callback(h_nvlp_t * h_nvlp)
{
    if (0 == h_nvlp->driver->get_gate_pin())
    {
        h_nvlp->gate_pin = 0;
    }
    else
    {
        if (0 == h_nvlp->gate_pin)
        {
            h_nvlp->state = NVLP_STATE_RISING;
            h_nvlp->gate_pin = 1;

            // No debounce for no
            // TODO maybe add later?
        }
    }

    switch(h_nvlp->state)
    {
        case NVLP_STATE_RESTING:
            h_nvlp->output = 0;
            h_nvlp->driver->set_output(0);
            break;
        case NVLP_STATE_RISING:
            compute_rising(h_nvlp);
            break;
        case NVLP_STATE_FALLING:
            compute_falling(h_nvlp);
            break;
    }
}

void compute_rising(h_nvlp_t * h_nvlp)
{
    uint16_t potentiometer = h_nvlp->driver->get_potentiometer();
    int32_t increment;

    if (potentiometer < NVLP_ADC_FALLING_MAX)
    {
        // 1. From 0 to NVLP_ADC_FALLING_MAX, Rising time is minimal and falling increases to max value
        increment = NVLP_INCREMENT_MAX;
    }
    else if (potentiometer < NVLP_ADC_RISING_MAX)
    {
        // 2. From NVLP_ADC_FALLING_MAX to NVLP_ADC_RISING_MAX, Falling time is maximal and Rising time increases to max value
        //    Increment from NVLP_INCREMENT_MIN to NVLP_INCREMENT_MAX
        //    NVLP_ADC_FALLING_MAX -> NVLP_INCREMENT_MIN ; NVLP_ADC_RISING_MAX -> NVLP_INCREMENT_MAX
    }
    else if (potentiometer < NVLP_ADC_FALLING_MIN)
    {
        // 3. From NVLP_ADC_RISING_MAX to NVLP_ADC_FALLING_MIN, Rising time is maximal and Falling time decreases to min value
        increment = NVLP_INCREMENT_MIN;
    }
    else if (potentiometer < NVLP_ADC_RISING_MIN)
    {
        // 4. From NVLP_ADC_FALLING_MIN to NVLP_ADC_RISING_MIN (max ADC value), Falling time is minimal and Rising time decreases to min value.
        //    Decrement from NVLP_INCREMENT_MAX to NVLP_INCREMENT_MIN
        //    NVLP_ADC_FALLING_MIN -> NVLP_INCREMENT_MAX ; NVLP_ADC_RISING_MIN -> NVLP_INCREMENT_MIN
    }
    else
    {
        // 5. Pot to the max, Rising and Falling are minimal
        increment = NVLP_INCREMENT_MAX;
        h_nvlp->state = NVLP_STATE_FALLING;
    }
}

void compute_falling(h_nvlp_t * h_nvlp)
{
    uint16_t potentiometer = h_nvlp->driver->get_potentiometer();
    int32_t decrement;

    if (potentiometer < NVLP_ADC_FALLING_MAX)
    {
        // 1. From 0 to NVLP_ADC_FALLING_MAX, Rising time is minimal and falling increases to max value
        //    Increment from NVLP_DECREMENT_MIN to NVLP_DECREMENT_MAX
        //    0 -> NVLP_DECREMENT_MIN ; NVLP_ADC_FALLING_MAX -> NVLP_DECREMENT_MAX
    }
    else if (potentiometer < NVLP_ADC_RISING_MAX)
    {
        // 2. From NVLP_ADC_FALLING_MAX to NVLP_ADC_RISING_MAX, Falling time is maximal and Rising time increases to max value
        decrement = NVLP_DECREMENT_MIN;
    }
    else if (potentiometer < NVLP_ADC_FALLING_MIN)
    {
        // 3. From NVLP_ADC_RISING_MAX to NVLP_ADC_FALLING_MIN, Rising time is maximal and Falling time decreases to min value
        //    Decrement from NVLP_DECREMENT_MAX to NVLP_DECREMENT_MIN
        //    NVLP_ADC_RISING_MAX -> NVLP_DECREMENT_MAX ; NVLP_ADC_FALLING_MIN -> NVLP_DECREMENT_MIN
    }
    else if (potentiometer < NVLP_ADC_RISING_MIN)
    {
        // 4. From NVLP_ADC_FALLING_MIN to NVLP_ADC_RISING_MIN (max ADC value), Falling time is minimal and Rising time decreases to min value.
        decrement = NVLP_DECREMENT_MAX;
    }
    else
    {
        // 5. Pot to the max, Rising and Falling are minimal
        decrement = NVLP_DECREMENT_MAX;
        h_nvlp->state = NVLP_STATE_RESTING;
    }
}