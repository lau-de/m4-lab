# Audi Concert RA4M1 native C v0.50 checkpoint

This Git checkpoint records the v0.50 audio state reached before porting HC05 7509 block $0F8B-$104F.

Key state:
- D830 RX dispatch and external-source bridge from v0.48
- source switching under TDA7342 soft mute from v0.49
- stock-like ramp core from $0D80-$0EEE in v0.50
- exact ramp tables/timer behavior retained
- physical ramp-to-TDA volume commit not yet connected because $0F8B-$104F still needs to be reproduced

Full local artifact SHA-256:
`5e41cbb155a5a91b260c2621f34ac20702fc23d32a7e25d7e3ab27f1e2c11154`
