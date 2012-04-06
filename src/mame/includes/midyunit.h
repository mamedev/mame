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
	midyunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_cmos_ram;
	UINT32 m_cmos_page;
	UINT8 *	m_gfx_rom;
	size_t m_gfx_rom_size;
	UINT16 m_prot_result;
	UINT16 m_prot_sequence[3];
	UINT8 m_prot_index;
	UINT8 m_term2_analog_select;
	const struct protection_data *m_prot_data;
	UINT8 m_cmos_w_enable;
	UINT8 m_chip_type;
	UINT16 *m_t2_hack_mem;
	UINT8 *m_cvsd_protection_base;
	UINT8 m_autoerase_enable;
	UINT32 m_palette_mask;
	pen_t *	m_pen_map;
	UINT16 *	m_local_videoram;
	UINT8 m_videobank_select;
	UINT8 m_yawdim_dma;
	UINT16 m_dma_register[16];
	dma_state_t m_dma_state;
	DECLARE_WRITE16_MEMBER(midyunit_cmos_w);
	DECLARE_READ16_MEMBER(midyunit_cmos_r);
	DECLARE_WRITE16_MEMBER(midyunit_cmos_enable_w);
	DECLARE_READ16_MEMBER(midyunit_protection_r);
	DECLARE_READ16_MEMBER(midyunit_input_r);
	DECLARE_WRITE16_MEMBER(midyunit_sound_w);
	DECLARE_READ16_MEMBER(term2_input_r);
	DECLARE_WRITE16_MEMBER(term2_sound_w);
	DECLARE_WRITE16_MEMBER(term2_hack_w);
	DECLARE_WRITE16_MEMBER(term2la3_hack_w);
	DECLARE_WRITE16_MEMBER(term2la2_hack_w);
	DECLARE_WRITE16_MEMBER(term2la1_hack_w);
	DECLARE_WRITE8_MEMBER(cvsd_protection_w);
	DECLARE_READ16_MEMBER(mkturbo_prot_r);
	DECLARE_READ16_MEMBER(midyunit_gfxrom_r);
	DECLARE_WRITE16_MEMBER(midyunit_vram_w);
	DECLARE_READ16_MEMBER(midyunit_vram_r);
	DECLARE_WRITE16_MEMBER(midyunit_control_w);
	DECLARE_WRITE16_MEMBER(midyunit_paletteram_w);
	DECLARE_READ16_MEMBER(midyunit_dma_r);
	DECLARE_WRITE16_MEMBER(midyunit_dma_w);
};


/*----------- defined in machine/midyunit.c -----------*/




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



/*----------- defined in video/midyunit.c -----------*/

VIDEO_START( midyunit_4bit );
VIDEO_START( midyunit_6bit );
VIDEO_START( mkyawdim );
VIDEO_START( midzunit );



void midyunit_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void midyunit_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);



void midyunit_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
