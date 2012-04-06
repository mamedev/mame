/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/qix.h"
#include "cpu/m6809/m6809.h"



/*************************************
 *
 *  Device tag
 *
 *************************************/

#define MC6845_TAG		("vid_u18")



/*************************************
 *
 *  Static function prototypes
 *
 *************************************/

static MC6845_BEGIN_UPDATE( begin_update );
static MC6845_UPDATE_ROW( update_row );
static WRITE_LINE_DEVICE_HANDLER( display_enable_changed );



/*************************************
 *
 *  Start
 *
 *************************************/

static VIDEO_START( qix )
{
	qix_state *state = machine.driver_data<qix_state>();

	/* allocate memory for the full video RAM */
	state->m_videoram = auto_alloc_array(machine, UINT8, 256 * 256);

	/* set up save states */
	state->save_pointer(NAME(state->m_videoram), 256 * 256);
	state->save_item(NAME(state->m_flip));
	state->save_item(NAME(state->m_palette_bank));
	state->save_item(NAME(state->m_leds));
}



/*************************************
 *
 *  Current scanline read
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( display_enable_changed )
{
	qix_state *driver_state = device->machine().driver_data<qix_state>();

	/* on the rising edge, latch the scanline */
	if (state)
	{
		UINT16 ma = downcast<mc6845_device *>(device)->get_ma();
		UINT8 ra = downcast<mc6845_device *>(device)->get_ra();

		/* RA0-RA2 goes to D0-D2 and MA5-MA9 goes to D3-D7 */
		*driver_state->m_scanline_latch = ((ma >> 2) & 0xf8) | (ra & 0x07);
	}
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

WRITE8_DEVICE_HANDLER( qix_flip_screen_w )
{
	qix_state *state = device->machine().driver_data<qix_state>();

	state->m_flip = data;
}



/*************************************
 *
 *  Direct video RAM read/write
 *
 *  The screen is 256x256 with eight
 *  bit pixels (64K).  The screen is
 *  divided into two halves each half
 *  mapped by the video CPU at
 *  $0000-$7FFF.  The high order bit
 *  of the address latch at $9402
 *  specifies which half of the screen
 *  is being accessed.
 *
 *************************************/

READ8_MEMBER(qix_state::qix_videoram_r)
{

	/* add in the upper bit of the address latch */
	offset += (m_videoram_address[0] & 0x80) << 8;
	return m_videoram[offset];
}


WRITE8_MEMBER(qix_state::qix_videoram_w)
{

	/* update the screen in case the game is writing "behind" the beam -
       Zookeeper likes to do this */
	machine().primary_screen->update_now();

	/* add in the upper bit of the address latch */
	offset += (m_videoram_address[0] & 0x80) << 8;

	/* write the data */
	m_videoram[offset] = data;
}


WRITE8_MEMBER(qix_state::slither_videoram_w)
{

	/* update the screen in case the game is writing "behind" the beam -
       Zookeeper likes to do this */
	machine().primary_screen->update_now();

	/* add in the upper bit of the address latch */
	offset += (m_videoram_address[0] & 0x80) << 8;

	/* blend the data */
	m_videoram[offset] = (m_videoram[offset] & ~*m_videoram_mask) | (data & *m_videoram_mask);
}



/*************************************
 *
 *  Latched video RAM read/write
 *
 *  The address latch works as follows.
 *  When the video CPU accesses $9400,
 *  the screen address is computed by
 *  using the values at $9402 (high
 *  byte) and $9403 (low byte) to get
 *  a value between $0000-$FFFF.  The
 *  value at that location is either
 *  returned or written.
 *
 *************************************/

READ8_MEMBER(qix_state::qix_addresslatch_r)
{

	/* compute the value at the address latch */
	offset = (m_videoram_address[0] << 8) | m_videoram_address[1];
	return m_videoram[offset];
}


WRITE8_MEMBER(qix_state::qix_addresslatch_w)
{

	/* update the screen in case the game is writing "behind" the beam */
	machine().primary_screen->update_now();

	/* compute the value at the address latch */
	offset = (m_videoram_address[0] << 8) | m_videoram_address[1];

	/* write the data */
	m_videoram[offset] = data;
}


WRITE8_MEMBER(qix_state::slither_addresslatch_w)
{

	/* update the screen in case the game is writing "behind" the beam */
	machine().primary_screen->update_now();

	/* compute the value at the address latch */
	offset = (m_videoram_address[0] << 8) | m_videoram_address[1];

	/* blend the data */
	m_videoram[offset] = (m_videoram[offset] & ~*m_videoram_mask) | (data & *m_videoram_mask);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/


WRITE8_MEMBER(qix_state::qix_paletteram_w)
{

	UINT8 old_data = m_paletteram[offset];

	/* set the palette RAM value */
	m_paletteram[offset] = data;

	/* trigger an update if a currently visible pen has changed */
	if (((offset >> 8) == m_palette_bank) &&
	    (old_data != data))
		machine().primary_screen->update_now();
}


WRITE8_MEMBER(qix_state::qix_palettebank_w)
{

	/* set the bank value */
	if (m_palette_bank != (data & 3))
	{
		machine().primary_screen->update_now();
		m_palette_bank = data & 3;
	}

	/* LEDs are in the upper 6 bits */
	m_leds = ~data & 0xfc;
}


static void get_pens(qix_state *state, pen_t *pens)
{
	offs_t offs;

	/* this conversion table should be about right. It gives a reasonable */
	/* gray scale in the test screen, and the red, green and blue squares */
	/* in the same screen are barely visible, as the manual requires. */
	static const UINT8 table[16] =
	{
		0x00,	/* value = 0, intensity = 0 */
		0x12,	/* value = 0, intensity = 1 */
		0x24,	/* value = 0, intensity = 2 */
		0x49,	/* value = 0, intensity = 3 */
		0x12,	/* value = 1, intensity = 0 */
		0x24,	/* value = 1, intensity = 1 */
		0x49,	/* value = 1, intensity = 2 */
		0x92,	/* value = 1, intensity = 3 */
		0x5b,	/* value = 2, intensity = 0 */
		0x6d,	/* value = 2, intensity = 1 */
		0x92,	/* value = 2, intensity = 2 */
		0xdb,	/* value = 2, intensity = 3 */
		0x7f,	/* value = 3, intensity = 0 */
		0x91,	/* value = 3, intensity = 1 */
		0xb6,	/* value = 3, intensity = 2 */
		0xff	/* value = 3, intensity = 3 */
	};

	for (offs = state->m_palette_bank << 8; offs < (state->m_palette_bank << 8) + NUM_PENS; offs++)
	{
		int bits, intensity, r, g, b;

		UINT8 data = state->m_paletteram[offs];

		/* compute R, G, B from the table */
		intensity = (data >> 0) & 0x03;
		bits = (data >> 6) & 0x03;
		r = table[(bits << 2) | intensity];
		bits = (data >> 4) & 0x03;
		g = table[(bits << 2) | intensity];
		bits = (data >> 2) & 0x03;
		b = table[(bits << 2) | intensity];

		/* update the palette */
		pens[offs & 0xff] = MAKE_RGB(r, g, b);
	}
}



/*************************************
 *
 *  M6845 callbacks for updating
 *  the screen
 *
 *************************************/

static MC6845_BEGIN_UPDATE( begin_update )
{
	qix_state *state = device->machine().driver_data<qix_state>();

#if 0
	// note the confusing bit order!
	popmessage("self test leds: %d%d %d%d%d%d",BIT(leds,7),BIT(leds,5),BIT(leds,6),BIT(leds,4),BIT(leds,2),BIT(leds,3));
#endif

	/* create the pens */
	get_pens(state, state->m_pens);

	return state->m_pens;
}


static MC6845_UPDATE_ROW( update_row )
{
	qix_state *state = device->machine().driver_data<qix_state>();
	UINT32 *dest = &bitmap.pix32(y);
	UINT16 x;

	pen_t *pens = (pen_t *)param;

	/* the memory is hooked up to the MA, RA lines this way */
	offs_t offs = ((ma << 6) & 0xf800) | ((ra << 8) & 0x0700);
	offs_t offs_xor = state->m_flip ? 0xffff : 0;

	for (x = 0; x < x_count * 8; x++)
		dest[x] = pens[state->m_videoram[(offs + x) ^ offs_xor]];
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( qix_video_map, AS_PROGRAM, 8, qix_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE(m_paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE(m_videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE(m_scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( zookeep_video_map, AS_PROGRAM, 8, qix_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03fe) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8801, 0x8801) AM_MIRROR(0x03fe) AM_WRITE(zookeep_bankswitch_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE(m_paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE(m_videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE(m_scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( slither_video_map, AS_PROGRAM, 8, qix_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, slither_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE(m_paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, slither_addresslatch_w)
	AM_RANGE(0x9401, 0x9401) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE(m_videoram_mask)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE(m_videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE(m_scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const mc6845_interface mc6845_intf =
{
	"screen",							/* screen we are acting on */
	8,									/* number of pixels per video memory address */
	begin_update,						/* before pixel update callback */
	update_row,							/* row update callback */
	NULL,								/* after pixel update callback */
	DEVCB_LINE(display_enable_changed),	/* callback for display state changes */
	DEVCB_NULL,							/* callback for cursor state changes */
	DEVCB_NULL,							/* HSYNC callback */
	DEVCB_LINE(qix_vsync_changed),		/* VSYNC callback */
	NULL								/* update address callback */
};


static const m6809_config encryption_config =
{
	TRUE,		/* encrypt only the first byte in 10 xx and 11 xx opcodes */
};


MACHINE_CONFIG_FRAGMENT( qix_video )
	MCFG_CPU_ADD("videocpu", M6809, MAIN_CLOCK_OSC/4/4)	/* 1.25 MHz */
	MCFG_CPU_PROGRAM_MAP(qix_video_map)
	MCFG_CPU_CONFIG(encryption_config)	// for kram3

	MCFG_VIDEO_START(qix)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, QIX_CHARACTER_CLOCK, mc6845_intf)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(QIX_CHARACTER_CLOCK*8, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( zookeep_video )
	MCFG_CPU_MODIFY("videocpu")
	MCFG_CPU_PROGRAM_MAP(zookeep_video_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( slither_video )
	MCFG_CPU_MODIFY("videocpu")
	MCFG_CPU_CLOCK(SLITHER_CLOCK_OSC/4/4)	/* 1.34 MHz */
	MCFG_CPU_PROGRAM_MAP(slither_video_map)
MACHINE_CONFIG_END
