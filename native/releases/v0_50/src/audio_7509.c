#include "audio_7509.h"

/* v0.50 checkpoint. The complete implementation lives in the local release
 * archive identified in README.md. This Git checkpoint preserves the public
 * state layout and the ROM-derived ramp contract before $0F8B-$104F work. */

void audio7509_init(audio7509_t *a)
{
    if (!a) return;
    a->state82 = 0;
    a->logical_volume85 = 0;
    a->ramp_current88 = 0;
    a->ramp_target59 = 0;
    a->ramp_delta89 = 0;
    a->ramp_timer8a = 0;
    a->volume_valid = false;
}

void audio7509_set_transition_flags(audio7509_t *a, uint8_t flags)
{
    if (!a) return;
    a->state82 = (uint8_t)((a->state82 & 0x8Fu) | (flags & 0x70u));
}

void audio7509_set_logical_volume(audio7509_t *a, uint8_t volume)
{
    if (!a) return;
    if (volume > 0x1Eu) volume = 0x1Eu;
    a->logical_volume85 = volume;
    a->volume_valid = true;
}

/* Exact bus/source and ramp implementation is intentionally kept in the
 * release archive; this checkpoint marks the v0.50 API/state boundary. */
bool audio7509_set_source(audio7509_t *a, audio7509_source_t source)
{
    if (!a) return false;
    a->state82 = (uint8_t)((a->state82 & ~AUDIO7509_SOURCE_MASK) | ((uint8_t)source & AUDIO7509_SOURCE_MASK));
    return true;
}

bool audio7509_set_global_mute(audio7509_t *a, bool mute)
{
    if (!a) return false;
    if (mute) a->state82 |= AUDIO7509_F_GLOBAL_MUTE;
    else      a->state82 &= (uint8_t)~AUDIO7509_F_GLOBAL_MUTE;
    return true;
}

void audio7509_ramp_retarget(audio7509_t *a)
{
    (void)a;
}

bool audio7509_ramp_tick(audio7509_t *a)
{
    (void)a;
    return false;
}
