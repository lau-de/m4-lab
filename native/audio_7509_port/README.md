# HC05 7509 native audio port

This directory contains the proven audio/source portion of the Audi Concert HC05 7509 ROM ported to native C.

## Proven mapping

HC05 RAM `$82`:

- bits `1:0`: source
  - `0` = CDC -> TDA7342 IN3 -> selector `0x38`
  - `1` = FM -> TDA7342 IN2 -> selector `0x39`
  - `2` = TAPE -> TDA7342 IN1 -> selector `0x3A`
  - `3` = AM mono -> selector `0x2B`
- bits `3:2`: current audio-mode mirror
- bit `4`: attenuation/ramp active
- bit `5`: ramp/state phase
- bit `6`: transition/hold state
- bit `7`: global soft mute

## TDA7342 writes reproduced from 7509

### Source selector

```text
88 00 38   CDC / IN3
88 00 39   FM  / IN2
88 00 3A   TAPE/ IN1
88 00 2B   AM mono
```

### Global soft mute

```text
88 08 81   mute
88 08 80   unmute
```

This is the native equivalent of the HC05 `$0F55/$0F70` pair and `$82.7`.

### Speaker attenuation / channel mute

The HC05 block at `$104F` sends one auto-increment transaction:

```text
88 14 LF LR RF RR
```

The four values map to TDA7342 speaker attenuator subaddresses 4..7.
Each value is `0x00..0x1F`; `0x1F` is the hardware mute code for that one output.

Native RAM image in `audio7509_t`:

```text
speaker_att[0] = LF
speaker_att[1] = LR
speaker_att[2] = RF
speaker_att[3] = RR
```

## Low-level bus callback

`audio7509_bus_write_fn` receives a complete transaction including the TDA7342 8-bit write address (`0x88`) in byte 0.

The existing RA4M1 PA7/PA6 open-drain bit-bang layer should therefore expose a small adapter of this form:

```c
static bool audio_bus_write(const uint8_t *bytes, size_t len, void *ctx)
{
    (void)ctx;
    /* START; send bytes[0..len-1] MSB-first with ACK checks; STOP. */
    return tda_bus_write_raw(bytes, len);
}
```

Then initialize once:

```c
static audio7509_t g_audio;
audio7509_init(&g_audio, audio_bus_write, NULL);
```

## Important design decision

D830 does **not** directly write `$82` and there are no proven `D830_MUTE_L` / `D830_MUTE_R` command bits. D830 reports tape/changer state; the master state machine selects the source and drives TDA7342.

For that reason this port deliberately does not invent D830-to-mute mappings.

Bits `$82.4/.5/.6` are retained in the native state representation, but their higher-level transition sequencing should only be wired after the remaining 7509 call paths are reproduced exactly.
