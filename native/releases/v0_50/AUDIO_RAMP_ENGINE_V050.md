# AUDIO ramp engine v0.50

Native checkpoint for the 7509 audio ramp port.

Stock state reconstructed from HC05 7509:
- $85 logical volume 0..30
- $88 current ramp value
- $59 target
- $89 signed delta
- $8A reload/timer
- $82.4 ramp-active
- $82.5 ramp profile select
- $82.6 transition attenuation hold / force target toward $FF

Profile tables:
- phase=1: +32,+16,+8,0,0,-2,-4,-8
- phase=0: +12,+6,+2,0,0,-2,-4,-8
- -2 tail uses reload 3.

Source switching remains mute-before-selector; physical TDA volume commit is intentionally deferred until $0F8B-$104F is ported.
