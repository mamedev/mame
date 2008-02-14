#include "sound/custom.h"

/*----------- defined in audio/attckufo.c -----------*/

void attckufo_soundport_w (int offset, int data);
void *attckufo_custom_start(int clock, const struct CustomSound_interface *config);


/*----------- defined in video/attckufo.c -----------*/

extern const rgb_t attckufo_palette[16];
extern UINT8 attckufo_regs[16];

VIDEO_UPDATE( attckufo );

WRITE8_HANDLER ( attckufo_port_w );
READ8_HANDLER ( attckufo_port_r );

INTERRUPT_GEN( attckufo_raster_interrupt );

