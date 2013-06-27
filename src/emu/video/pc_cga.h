#include "emu.h"

#define CGA_PALETTE_SETS 83 /* one for colour, one for mono,
                 * 81 for colour composite */

#define CGA_SCREEN_NAME "screen"
#define CGA_MC6845_NAME "mc6845_cga"

MACHINE_CONFIG_EXTERN( pcvideo_cga );
MACHINE_CONFIG_EXTERN( pcvideo_cga32k );
MACHINE_CONFIG_EXTERN( pcvideo_mc1502 );
INPUT_PORTS_EXTERN( pcvideo_cga );
INPUT_PORTS_EXTERN( pcvideo_mc1502 );
SCREEN_UPDATE_RGB32( mc6845_cga );
VIDEO_START( pc_cga_superimpose );

/* has a special 640x200 in 16 color mode, 4 banks at 0xb8000 */
MACHINE_CONFIG_EXTERN( pcvideo_pc1512 );
INPUT_PORTS_EXTERN( pcvideo_pc1512 );


/* Used in machine/pc.c */
DECLARE_READ16_HANDLER( pc1512_16le_r );
DECLARE_WRITE16_HANDLER( pc1512_16le_w );
DECLARE_WRITE16_HANDLER( pc1512_videoram16le_w );

MACHINE_CONFIG_EXTERN( pcvideo_poisk2 );
