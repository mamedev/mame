/*************************************************************************

    Atari "Stella on Steroids" hardware

*************************************************************************/

#include "machine/atarigen.h"

class beathead_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, beathead_state(machine)); }

	beathead_state(running_machine &machine)
		: atarigen_state(machine) { }

	UINT32 *		vram_bulk_latch;
	UINT32 *		palette_select;

	UINT32			finescroll;
	offs_t			vram_latch_offset;

	offs_t			hsyncram_offset;
	offs_t			hsyncram_start;
	UINT8			hsyncram[0x800];

	UINT32 *		ram_base;
	UINT32 *		rom_base;

	double			hblank_offset;

	UINT8			irq_line_state;
	UINT8			irq_enable[3];
	UINT8			irq_state[3];

	UINT8			eeprom_enabled;
};


/*----------- defined in video/beathead.c -----------*/

VIDEO_START( beathead );
VIDEO_UPDATE( beathead );

WRITE32_HANDLER( beathead_vram_transparent_w );
WRITE32_HANDLER( beathead_vram_bulk_w );
WRITE32_HANDLER( beathead_vram_latch_w );
WRITE32_HANDLER( beathead_vram_copy_w );
WRITE32_HANDLER( beathead_finescroll_w );
WRITE32_HANDLER( beathead_palette_w );
READ32_HANDLER( beathead_hsync_ram_r );
WRITE32_HANDLER( beathead_hsync_ram_w );
