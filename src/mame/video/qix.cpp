// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Zsolt Vasvari
// thanks-to: John Butler, Ed Mueller
/***************************************************************************

    Taito Qix hardware

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/qix.h"
#include "cpu/m6809/m6809.h"


/*************************************
 *
 *  Start
 *
 *************************************/

VIDEO_START_MEMBER(qix_state,qix)
{
	/* allocate memory for the full video RAM */
	m_videoram.allocate(256 * 256);

	/* initialize the palette */
	for (int x = 0; x < 0x400; x++)
		set_pen(x);

	/* set up save states */
	save_item(NAME(m_flip));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_leds));
	save_item(NAME(m_pens));
}



/*************************************
 *
 *  Current scanline read
 *
 *************************************/

WRITE_LINE_MEMBER(qix_state::display_enable_changed)
{
	/* on the rising edge, latch the scanline */
	if (state)
	{
		UINT16 ma = m_crtc->get_ma();
		UINT8 ra = m_crtc->get_ra();

		/* RA0-RA2 goes to D0-D2 and MA5-MA9 goes to D3-D7 */
		*m_scanline_latch = ((ma >> 2) & 0xf8) | (ra & 0x07);
	}
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

WRITE_LINE_MEMBER(qix_state::qix_flip_screen_w)
{
	m_flip = state;
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
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* add in the upper bit of the address latch */
	offset += (m_videoram_address[0] & 0x80) << 8;

	/* write the data */
	m_videoram[offset] = data;
}


WRITE8_MEMBER(qix_state::slither_videoram_w)
{
	/* update the screen in case the game is writing "behind" the beam -
	   Zookeeper likes to do this */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

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
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* compute the value at the address latch */
	offset = (m_videoram_address[0] << 8) | m_videoram_address[1];

	/* write the data */
	m_videoram[offset] = data;
}


WRITE8_MEMBER(qix_state::slither_addresslatch_w)
{
	/* update the screen in case the game is writing "behind" the beam */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

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
	{
	//  m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
	}

	set_pen(offset);
}


WRITE8_MEMBER(qix_state::qix_palettebank_w)
{
	/* set the bank value */
	if (m_palette_bank != (data & 3))
	{
		//m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
		m_palette_bank = data & 3;
	}

	/* LEDs are in the upper 6 bits */
	m_leds = ~data & 0xfc;
}


void qix_state::set_pen(int offs)
{
	/* this conversion table should be about right. It gives a reasonable */
	/* gray scale in the test screen, and the red, green and blue squares */
	/* in the same screen are barely visible, as the manual requires. */
	static const UINT8 table[16] =
	{
		0x00,   /* value = 0, intensity = 0 */
		0x12,   /* value = 0, intensity = 1 */
		0x24,   /* value = 0, intensity = 2 */
		0x49,   /* value = 0, intensity = 3 */
		0x12,   /* value = 1, intensity = 0 */
		0x24,   /* value = 1, intensity = 1 */
		0x49,   /* value = 1, intensity = 2 */
		0x92,   /* value = 1, intensity = 3 */
		0x5b,   /* value = 2, intensity = 0 */
		0x6d,   /* value = 2, intensity = 1 */
		0x92,   /* value = 2, intensity = 2 */
		0xdb,   /* value = 2, intensity = 3 */
		0x7f,   /* value = 3, intensity = 0 */
		0x91,   /* value = 3, intensity = 1 */
		0xb6,   /* value = 3, intensity = 2 */
		0xff    /* value = 3, intensity = 3 */
	};

	int bits, intensity, r, g, b;

	UINT8 data = m_paletteram[offs];

	/* compute R, G, B from the table */
	intensity = (data >> 0) & 0x03;
	bits = (data >> 6) & 0x03;
	r = table[(bits << 2) | intensity];
	bits = (data >> 4) & 0x03;
	g = table[(bits << 2) | intensity];
	bits = (data >> 2) & 0x03;
	b = table[(bits << 2) | intensity];

	/* update the palette */
	m_pens[offs] = rgb_t(r, g, b);
}



/*************************************
 *
 *  M6845 callbacks for updating
 *  the screen
 *
 *************************************/

MC6845_BEGIN_UPDATE( qix_state::crtc_begin_update )
{
#if 0
	// note the confusing bit order!
	popmessage("self test leds: %d%d %d%d%d%d",BIT(leds,7),BIT(leds,5),BIT(leds,6),BIT(leds,4),BIT(leds,2),BIT(leds,3));
#endif
}


MC6845_UPDATE_ROW( qix_state::crtc_update_row )
{
	UINT32 *dest = &bitmap.pix32(y);
	pen_t *pens = &m_pens[m_palette_bank << 8];

	/* the memory is hooked up to the MA, RA lines this way */
	offs_t offs = ((ma << 6) & 0xf800) | ((ra << 8) & 0x0700);
	offs_t offs_xor = m_flip ? 0xffff : 0;

	for (UINT16 x = 0; x < x_count * 8; x++)
		dest[x] = pens[m_videoram[(offs + x) ^ offs_xor]];
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
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_SHARE("videoram_addr")
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_SHARE("scanline_latch")
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( kram3_video_map, AS_PROGRAM, 8, qix_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_SHARE("videoram_addr")
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_SHARE("scanline_latch")
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( zookeep_video_map, AS_PROGRAM, 8, qix_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03fe) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8801, 0x8801) AM_MIRROR(0x03fe) AM_WRITE(zookeep_bankswitch_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_SHARE("videoram_addr")
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_SHARE("scanline_latch")
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
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, slither_addresslatch_w)
	AM_RANGE(0x9401, 0x9401) AM_MIRROR(0x03fc) AM_WRITEONLY AM_SHARE("videoram_mask")
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_SHARE("videoram_addr")
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_SHARE("scanline_latch")
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_device, address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_device, register_r, register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( qix_video )
	MCFG_CPU_ADD("videocpu", M6809, MAIN_CLOCK_OSC/4/4) /* 1.25 MHz */
	MCFG_CPU_PROGRAM_MAP(qix_video_map)

	MCFG_VIDEO_START_OVERRIDE(qix_state,qix)

	MCFG_MC6845_ADD("vid_u18", MC6845, "screen", QIX_CHARACTER_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(qix_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(qix_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(qix_state, display_enable_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(qix_state, qix_vsync_changed))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(QIX_CHARACTER_CLOCK*8, 0x148, 0, 0x100, 0x111, 0, 0x100) /* from CRTC */
	MCFG_SCREEN_UPDATE_DEVICE("vid_u18", mc6845_device, screen_update)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( kram3_video )
	MCFG_CPU_REPLACE("videocpu", M6809E, MAIN_CLOCK_OSC/4) /* 1.25 MHz */
	MCFG_CPU_PROGRAM_MAP(kram3_video_map)
	MCFG_M6809E_LIC_CB(WRITELINE(qix_state,kram3_lic_videocpu_changed))
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( zookeep_video )
	MCFG_CPU_MODIFY("videocpu")
	MCFG_CPU_PROGRAM_MAP(zookeep_video_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( slither_video )
	MCFG_CPU_MODIFY("videocpu")
	MCFG_CPU_CLOCK(SLITHER_CLOCK_OSC/4/4)   /* 1.34 MHz */
	MCFG_CPU_PROGRAM_MAP(slither_video_map)
MACHINE_CONFIG_END
