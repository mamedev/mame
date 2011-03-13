/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "machine/nvram.h"

/* protection data types */
struct protection_data
{
	UINT16	reset_sequence[3];
	UINT16	data_sequence[100];
};

struct dma_state_t
{
	UINT32		offset;			/* source offset, in bits */
	INT32		rowbytes;		/* source bytes to skip each row */
	INT32		xpos;			/* x position, clipped */
	INT32		ypos;			/* y position, clipped */
	INT32		width;			/* horizontal pixel count */
	INT32		height;			/* vertical pixel count */
	UINT16		palette;		/* palette base */
	UINT16		color;			/* current foreground color with palette */
};


class midyunit_state : public driver_device
{
public:
	midyunit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *cmos_ram;
	UINT32 cmos_page;
	UINT8 *	gfx_rom;
	size_t gfx_rom_size;
	UINT16 prot_result;
	UINT16 prot_sequence[3];
	UINT8 prot_index;
	UINT8 term2_analog_select;
	const struct protection_data *prot_data;
	UINT8 cmos_w_enable;
	UINT8 chip_type;
	UINT16 *t2_hack_mem;
	UINT8 *cvsd_protection_base;
	UINT8 autoerase_enable;
	UINT32 palette_mask;
	pen_t *	pen_map;
	UINT16 *	local_videoram;
	UINT8 videobank_select;
	UINT8 yawdim_dma;
	UINT16 dma_register[16];
	dma_state_t dma_state;
};


/*----------- defined in machine/midyunit.c -----------*/

WRITE16_HANDLER( midyunit_cmos_w );
READ16_HANDLER( midyunit_cmos_r );

WRITE16_HANDLER( midyunit_cmos_enable_w );
READ16_HANDLER( midyunit_protection_r );

READ16_HANDLER( midyunit_input_r );

DRIVER_INIT( narc );
DRIVER_INIT( trog );
DRIVER_INIT( smashtv );
DRIVER_INIT( hiimpact );
DRIVER_INIT( shimpact );
DRIVER_INIT( strkforc );
DRIVER_INIT( mkyunit );
DRIVER_INIT( mkyturbo );
DRIVER_INIT( mkyawdim );
DRIVER_INIT( term2 );
DRIVER_INIT( term2la3 );
DRIVER_INIT( term2la2 );
DRIVER_INIT( term2la1 );
DRIVER_INIT( totcarn );

MACHINE_RESET( midyunit );

WRITE16_HANDLER( midyunit_sound_w );


/*----------- defined in video/midyunit.c -----------*/

VIDEO_START( midyunit_4bit );
VIDEO_START( midyunit_6bit );
VIDEO_START( mkyawdim );
VIDEO_START( midzunit );

READ16_HANDLER( midyunit_gfxrom_r );

WRITE16_HANDLER( midyunit_vram_w );
READ16_HANDLER( midyunit_vram_r );

void midyunit_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void midyunit_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);

WRITE16_HANDLER( midyunit_control_w );
WRITE16_HANDLER( midyunit_paletteram_w );

READ16_HANDLER( midyunit_dma_r );
WRITE16_HANDLER( midyunit_dma_w );

void midyunit_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
