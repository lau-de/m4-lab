#ifndef AUDIO_7509_H
#define AUDIO_7509_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Native-C representation of HC05 7509 audio state byte $82.
 *
 * Proven mapping:
 *   bits 1:0  source selector
 *   bits 3:2  current audio-mode mirror
 *   bit 4     attenuation/ramp active
 *   bit 5     ramp/state phase
 *   bit 6     transition/hold state
 *   bit 7     global soft mute
 */
#define AUDIO7509_SOURCE_MASK          0x03u
#define AUDIO7509_MODE_MASK            0x0Cu
#define AUDIO7509_F_RAMP_ACTIVE        0x10u
#define AUDIO7509_F_RAMP_PHASE         0x20u
#define AUDIO7509_F_TRANSITION_HOLD    0x40u
#define AUDIO7509_F_GLOBAL_MUTE        0x80u

typedef enum {
    AUDIO7509_SRC_CDC  = 0u, /* TDA7342 IN3, selector byte 0x38 */
    AUDIO7509_SRC_FM   = 1u, /* TDA7342 IN2, selector byte 0x39 */
    AUDIO7509_SRC_TAPE = 2u, /* TDA7342 IN1, selector byte 0x3A */
    AUDIO7509_SRC_AM   = 3u  /* TDA7342 AM mono, selector byte 0x2B */
} audio7509_source_t;

typedef enum {
    AUDIO7509_SPK_LF = 0u,
    AUDIO7509_SPK_LR = 1u,
    AUDIO7509_SPK_RF = 2u,
    AUDIO7509_SPK_RR = 3u
} audio7509_speaker_t;

/*
 * Low-level transport callback.
 * The buffer contains the complete TDA7342 I2C transaction bytes,
 * including the 8-bit write address 0x88 as byte 0.
 * Return true if the transaction was accepted by the bus layer.
 */
typedef bool (*audio7509_bus_write_fn)(const uint8_t *bytes,
                                       size_t len,
                                       void *ctx);

typedef struct {
    uint8_t state82;

    /* TDA7342 speaker-attenuator register images: LF, LR, RF, RR. */
    uint8_t speaker_att[4];

    audio7509_bus_write_fn bus_write;
    void *bus_ctx;
} audio7509_t;

void audio7509_init(audio7509_t *a,
                    audio7509_bus_write_fn bus_write,
                    void *bus_ctx);

uint8_t audio7509_state82(const audio7509_t *a);
audio7509_source_t audio7509_source(const audio7509_t *a);

bool audio7509_set_source(audio7509_t *a, audio7509_source_t source);
bool audio7509_set_global_mute(audio7509_t *a, bool mute);

void audio7509_set_mode_bits(audio7509_t *a, uint8_t mode_bits_3_2);
void audio7509_set_transition_flags(audio7509_t *a,
                                    bool ramp_active,
                                    bool ramp_phase,
                                    bool transition_hold);

/*
 * TDA7342 speaker attenuator values are clamped to 0x00..0x1F.
 * 0x1F is the hardware mute code for that one output.
 */
bool audio7509_set_speaker_atten(audio7509_t *a,
                                 audio7509_speaker_t speaker,
                                 uint8_t attenuation);
bool audio7509_set_speaker_mute(audio7509_t *a,
                                audio7509_speaker_t speaker,
                                bool mute);

/* Push current source, global mute state and LF/LR/RF/RR image. */
bool audio7509_apply_all(audio7509_t *a);

#endif /* AUDIO_7509_H */
