#ifndef AUDIO_7509_H
#define AUDIO_7509_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    AUDIO7509_SRC_CDC  = 0,
    AUDIO7509_SRC_FM   = 1,
    AUDIO7509_SRC_TAPE = 2,
    AUDIO7509_SRC_AM   = 3,
} audio7509_source_t;

#define AUDIO7509_SOURCE_MASK        0x03u
#define AUDIO7509_MODE_MASK          0x0Cu
#define AUDIO7509_F_RAMP_ACTIVE      0x10u
#define AUDIO7509_F_RAMP_PHASE       0x20u
#define AUDIO7509_F_TRANSITION_HOLD  0x40u
#define AUDIO7509_F_GLOBAL_MUTE      0x80u

typedef struct {
    uint8_t state82;
    uint8_t logical_volume85;
    uint8_t ramp_current88;
    uint8_t ramp_target59;
    int8_t  ramp_delta89;
    uint8_t ramp_timer8a;
    bool    volume_valid;
} audio7509_t;

void audio7509_init(audio7509_t *a);
bool audio7509_set_source(audio7509_t *a, audio7509_source_t source);
bool audio7509_set_global_mute(audio7509_t *a, bool mute);
void audio7509_set_transition_flags(audio7509_t *a, uint8_t flags);
void audio7509_set_logical_volume(audio7509_t *a, uint8_t volume);
void audio7509_ramp_retarget(audio7509_t *a);
bool audio7509_ramp_tick(audio7509_t *a);

#endif
