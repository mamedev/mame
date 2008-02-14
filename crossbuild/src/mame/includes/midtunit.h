/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "midyunit.h"

/*----------- defined in machine/midtunit.c -----------*/

WRITE16_HANDLER( midtunit_cmos_enable_w );
WRITE16_HANDLER( midtunit_cmos_w );
READ16_HANDLER( midtunit_cmos_r );

READ16_HANDLER( midtunit_input_r );

DRIVER_INIT( mktunit );
DRIVER_INIT( jdreddp );
DRIVER_INIT( nbajam );
DRIVER_INIT( nbajamte );
DRIVER_INIT( mk2 );

MACHINE_RESET( midtunit );

READ16_HANDLER( midtunit_sound_state_r );
READ16_HANDLER( midtunit_sound_r );
WRITE16_HANDLER( midtunit_sound_w );


/*----------- defined in video/midtunit.c -----------*/

extern UINT8 midtunit_gfx_rom_large;

VIDEO_START( midtunit );
VIDEO_START( midwunit );
VIDEO_START( midxunit );

READ16_HANDLER( midtunit_gfxrom_r );
READ16_HANDLER( midwunit_gfxrom_r );

WRITE16_HANDLER( midtunit_vram_w );
WRITE16_HANDLER( midtunit_vram_data_w );
WRITE16_HANDLER( midtunit_vram_color_w );

READ16_HANDLER( midtunit_vram_r );
READ16_HANDLER( midtunit_vram_data_r );
READ16_HANDLER( midtunit_vram_color_r );

void midtunit_to_shiftreg(UINT32 address, UINT16 *shiftreg);
void midtunit_from_shiftreg(UINT32 address, UINT16 *shiftreg);

WRITE16_HANDLER( midtunit_control_w );
WRITE16_HANDLER( midwunit_control_w );
READ16_HANDLER( midwunit_control_r );

WRITE16_HANDLER( midtunit_paletteram_w );
WRITE16_HANDLER( midxunit_paletteram_w );
READ16_HANDLER( midxunit_paletteram_r );

READ16_HANDLER( midtunit_dma_r );
WRITE16_HANDLER( midtunit_dma_w );

void midtunit_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params);
void midxunit_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params);
