#include "audio_7509.h"

#define TDA7342_ADDR_WRITE        0x88u
#define TDA7342_SUB_INPUT         0x00u
#define TDA7342_SUB_MUTE          0x08u
#define TDA7342_SUB_SPK_AUTO      0x14u
#define TDA7342_SPK_MUTE          0x1Fu

static const uint8_t k_source_selector[4] = {
    0x38u, /* CDC -> IN3 */
    0x39u, /* FM  -> IN2 */
    0x3Au, /* TAPE-> IN1 */
    0x2Bu  /* AM  -> mono */
};

static bool bus_write(audio7509_t *a, const uint8_t *p, size_t n)
{
    if (!a || !a->bus_write || !p || n == 0u) {
        return false;
    }
    return a->bus_write(p, n, a->bus_ctx);
}

static bool write_source(audio7509_t *a)
{
    const uint8_t src = (uint8_t)(a->state82 & AUDIO7509_SOURCE_MASK);
    const uint8_t frame[3] = {
        TDA7342_ADDR_WRITE,
        TDA7342_SUB_INPUT,
        k_source_selector[src]
    };
    return bus_write(a, frame, sizeof(frame));
}

static bool write_global_mute(audio7509_t *a)
{
    /* 7509 uses 0x81 for mute, 0x80 for unmute at subaddress 8. */
    const uint8_t value =
        (a->state82 & AUDIO7509_F_GLOBAL_MUTE) ? 0x81u : 0x80u;
    const uint8_t frame[3] = {
        TDA7342_ADDR_WRITE,
        TDA7342_SUB_MUTE,
        value
    };
    return bus_write(a, frame, sizeof(frame));
}

static bool write_speakers(audio7509_t *a)
{
    /*
     * 7509 native equivalent of the block beginning at HC05 $104F:
     *   88 14 LF LR RF RR
     * 0x14 = autoincrement + starting speaker subaddress 4.
     */
    const uint8_t frame[6] = {
        TDA7342_ADDR_WRITE,
        TDA7342_SUB_SPK_AUTO,
        a->speaker_att[AUDIO7509_SPK_LF],
        a->speaker_att[AUDIO7509_SPK_LR],
        a->speaker_att[AUDIO7509_SPK_RF],
        a->speaker_att[AUDIO7509_SPK_RR]
    };
    return bus_write(a, frame, sizeof(frame));
}

void audio7509_init(audio7509_t *a,
                    audio7509_bus_write_fn bus_write_fn,
                    void *bus_ctx)
{
    if (!a) {
        return;
    }

    a->state82 = 0u; /* HC05 cold/audio-init also starts from CLR $82. */
    a->speaker_att[0] = 0u;
    a->speaker_att[1] = 0u;
    a->speaker_att[2] = 0u;
    a->speaker_att[3] = 0u;
    a->bus_write = bus_write_fn;
    a->bus_ctx = bus_ctx;
}

uint8_t audio7509_state82(const audio7509_t *a)
{
    return a ? a->state82 : 0u;
}

audio7509_source_t audio7509_source(const audio7509_t *a)
{
    if (!a) {
        return AUDIO7509_SRC_CDC;
    }
    return (audio7509_source_t)(a->state82 & AUDIO7509_SOURCE_MASK);
}

bool audio7509_set_source(audio7509_t *a, audio7509_source_t source)
{
    uint8_t s;

    if (!a || (unsigned)source > (unsigned)AUDIO7509_SRC_AM) {
        return false;
    }

    s = a->state82;
    s &= (uint8_t)~AUDIO7509_SOURCE_MASK;
    s |= (uint8_t)source;
    a->state82 = s;

    return write_source(a);
}

bool audio7509_set_global_mute(audio7509_t *a, bool mute)
{
    if (!a) {
        return false;
    }

    if (mute) {
        a->state82 |= AUDIO7509_F_GLOBAL_MUTE;
    } else {
        a->state82 &= (uint8_t)~AUDIO7509_F_GLOBAL_MUTE;
    }

    return write_global_mute(a);
}

void audio7509_set_mode_bits(audio7509_t *a, uint8_t mode_bits_3_2)
{
    if (!a) {
        return;
    }

    a->state82 &= (uint8_t)~AUDIO7509_MODE_MASK;
    a->state82 |= (uint8_t)(mode_bits_3_2 & AUDIO7509_MODE_MASK);
}

void audio7509_set_transition_flags(audio7509_t *a,
                                    bool ramp_active,
                                    bool ramp_phase,
                                    bool transition_hold)
{
    uint8_t s;

    if (!a) {
        return;
    }

    s = a->state82;
    s &= (uint8_t)~(AUDIO7509_F_RAMP_ACTIVE |
                    AUDIO7509_F_RAMP_PHASE |
                    AUDIO7509_F_TRANSITION_HOLD);

    if (ramp_active) {
        s |= AUDIO7509_F_RAMP_ACTIVE;
    }
    if (ramp_phase) {
        s |= AUDIO7509_F_RAMP_PHASE;
    }
    if (transition_hold) {
        s |= AUDIO7509_F_TRANSITION_HOLD;
    }

    a->state82 = s;
}

bool audio7509_set_speaker_atten(audio7509_t *a,
                                 audio7509_speaker_t speaker,
                                 uint8_t attenuation)
{
    if (!a || (unsigned)speaker > (unsigned)AUDIO7509_SPK_RR) {
        return false;
    }

    if (attenuation > TDA7342_SPK_MUTE) {
        attenuation = TDA7342_SPK_MUTE;
    }

    a->speaker_att[speaker] = attenuation;
    return write_speakers(a);
}

bool audio7509_set_speaker_mute(audio7509_t *a,
                                audio7509_speaker_t speaker,
                                bool mute)
{
    if (!a || (unsigned)speaker > (unsigned)AUDIO7509_SPK_RR) {
        return false;
    }

    /*
     * We intentionally preserve the last non-mute value only in the caller.
     * The HC05 code computes the full LF/LR/RF/RR image from balance/fader
     * state before each write; it does not need a hidden per-channel restore
     * value here.
     */
    a->speaker_att[speaker] = mute ? TDA7342_SPK_MUTE : 0u;
    return write_speakers(a);
}

bool audio7509_apply_all(audio7509_t *a)
{
    bool ok = true;

    if (!a) {
        return false;
    }

    ok = write_source(a) && ok;
    ok = write_speakers(a) && ok;
    ok = write_global_mute(a) && ok;
    return ok;
}
