#ifndef _KONAMIIC_H
#define _KONAMIIC_H

void K056800_init(void (* irq_callback)(int));
READ32_HANDLER(K056800_host_r);
WRITE32_HANDLER(K056800_host_w);
READ16_HANDLER(K056800_sound_r);
WRITE16_HANDLER(K056800_sound_w);

#endif
