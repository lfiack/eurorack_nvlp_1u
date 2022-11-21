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

void compute_attack_decay(h_nvlp_t * h_nvlp);
void compute_rising(h_nvlp_t * h_nvlp);
void compute_falling(h_nvlp_t * h_nvlp);

nvlp_status_t nvlp_init(h_nvlp_t * h_nvlp)
{
    h_nvlp->gate_pin = 0;
    h_nvlp->output = 0;
    h_nvlp->attack = NVLP_ATTACK_MIN;
    h_nvlp->decay = NVLP_DECAY_MIN;
    h_nvlp->state = NVLP_STATE_REST;

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
            h_nvlp->state = NVLP_STATE_ATTACK;
            h_nvlp->gate_pin = 1;

            // No debounce for now
            // TODO maybe add later?
        }
    }

    compute_attack_decay(h_nvlp);

    switch(h_nvlp->state)
    {
        case NVLP_STATE_REST:
            h_nvlp->output = 0;
            h_nvlp->driver->set_output(0);
            break;
        case NVLP_STATE_ATTACK:
            compute_rising(h_nvlp);
            break;
        case NVLP_STATE_DECAY:
            compute_falling(h_nvlp);
            break;
    }
}

void compute_attack_decay(h_nvlp_t * h_nvlp)
{
    uint16_t potentiometer = h_nvlp->driver->get_potentiometer();

    if (potentiometer < NVLP_ADC_DECAY_MAX)
    {
        // 1. From 0 to NVLP_ADC_DECAY_MAX, Attack time is minimal and Decay increases to max value
        h_nvlp->attack = NVLP_ATTACK_MIN;
        // decay = NVLP_DECAY_MIN when ADC is 0 ; decay = NVLP_DECAY_MAX when ADC = NVLP_ADC_DECAY_MAX
        int32_t decay = NVLP_DECAY_MIN + (int32_t)(potentiometer) * (NVLP_DECAY_MAX - NVLP_DECAY_MIN) / NVLP_ADC_DECAY_MAX;
        h_nvlp->decay = (uint16_t) decay;
    }
    else if (potentiometer < NVLP_ADC_ATTACK_MAX)
    {
        // 2. From NVLP_ADC_DECAY_MAX to NVLP_ADC_ATTACK_MAX, Decay time is maximal and Attack time increases to max value
        h_nvlp->decay = NVLP_DECAY_MAX;
        // attack = NVLP_ATTACK_MIN when ADC is NVLP_ADC_DECAY_MAX + 1 ; attack = NVLP_ATTACK_MAX when ADC = NVLP_ADC_DECAY_MIN
        int32_t attack = NVLP_ATTACK_MIN + ((int32_t)(potentiometer - NVLP_ADC_DECAY_MAX)) * (NVLP_ATTACK_MAX - NVLP_ATTACK_MIN) / (NVLP_ADC_ATTACK_MAX - NVLP_ADC_DECAY_MAX);
        h_nvlp->attack = (uint16_t) attack;
    }
    else if (potentiometer < NVLP_ADC_DECAY_MIN)
    {
        // 3. From NVLP_ADC_ATTACK_MAX to NVLP_ADC_DECAY_MIN, Attack time is maximal and Decay time decreases to min value
        h_nvlp->attack = NVLP_ATTACK_MAX;
        int32_t decay = NVLP_DECAY_MAX + (int32_t)(potentiometer - NVLP_ADC_ATTACK_MAX) * (NVLP_DECAY_MIN - NVLP_DECAY_MAX) / (NVLP_ADC_DECAY_MIN - NVLP_ADC_ATTACK_MAX);
        h_nvlp->decay = (uint16_t) decay;
    }
    else if (potentiometer < NVLP_ADC_ATTACK_MIN)
    {
        // 4. From NVLP_ADC_DECAY_MIN to NVLP_ADC_ATTACK_MIN (max ADC value), Decay time is minimal and Attack time decreases to min value.
        //    Decrement from NVLP_INCREMENT_MAX to NVLP_INCREMENT_MIN
        //    NVLP_ADC_DECAY_MIN -> NVLP_INCREMENT_MAX ; NVLP_ADC_ATTACK_MIN -> NVLP_INCREMENT_MIN
        h_nvlp->decay = NVLP_DECAY_MIN;
        int32_t attack = NVLP_ATTACK_MAX + (int32_t)(potentiometer - NVLP_ADC_DECAY_MIN) * (NVLP_ATTACK_MIN - NVLP_ATTACK_MAX) / (NVLP_ADC_ATTACK_MIN - NVLP_ADC_DECAY_MIN);
        h_nvlp->attack = (uint16_t) attack;
    }
    else
    {
        // 5. Pot to the max, Attack and Decay are minimal
        h_nvlp->attack = NVLP_ATTACK_MIN;
        h_nvlp->decay = NVLP_DECAY_MIN;
    }
}


void compute_rising(h_nvlp_t * h_nvlp)
{

}

void compute_falling(h_nvlp_t * h_nvlp)
{

}