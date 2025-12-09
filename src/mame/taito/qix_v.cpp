// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Zsolt Vasvari
// thanks-to: John Butler, Ed Mueller
/***************************************************************************

    Taito Qix hardware

***************************************************************************/

#include "emu.h"
#include "qix.h"


/*************************************
 *
 *  Start
 *
 *************************************/

void qix_state::video_start()
{
	/* set up save states */
	save_item(NAME(m_flip));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_leds));
}



/*************************************
 *
 *  Current scanline read
 *
 *************************************/

void qix_state::display_enable_changed(int state)
{
	/* on the rising edge, latch the scanline */
	if (state)
	{
		uint16_t const ma = m_crtc->get_ma();
		uint8_t const ra = m_crtc->get_ra();

		/* RA0-RA2 goes to D0-D2 and MA5-MA9 goes to D3-D7 */
		*m_scanline_latch = ((ma >> 2) & 0xf8) | (ra & 0x07);
	}
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

void qix_state::flip_screen_w(int state)
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

uint8_t qix_state::videoram_r(offs_t offset)
{
	/* add in the upper bit of the address latch */
	offset += (m_videoram_address[0] & 0x80) << 8;
	return m_videoram[offset];
}


void qix_state::videoram_w(offs_t offset, uint8_t data)
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


void slither_state::slither_videoram_w(offs_t offset, uint8_t data)
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

uint8_t qix_state::addresslatch_r()
{
	/* compute the value at the address latch */
	offs_t const offset = (m_videoram_address[0] << 8) | m_videoram_address[1];
	return m_videoram[offset];
}


void qix_state::addresslatch_w(uint8_t data)
{
	/* update the screen in case the game is writing "behind" the beam */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* compute the value at the address latch */
	offs_t const offset = (m_videoram_address[0] << 8) | m_videoram_address[1];

	/* write the data */
	m_videoram[offset] = data;
}


void slither_state::slither_addresslatch_w(uint8_t data)
{
	/* update the screen in case the game is writing "behind" the beam */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* compute the value at the address latch */
	offs_t const offset = (m_videoram_address[0] << 8) | m_videoram_address[1];

	/* blend the data */
	m_videoram[offset] = (m_videoram[offset] & ~*m_videoram_mask) | (data & *m_videoram_mask);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/


void qix_state::paletteram_w(offs_t offset, uint8_t data)
{
	uint8_t const old_data = m_palette->read8(offset);

	/* trigger an update if a currently visible pen has changed */
	if (((offset >> 8) == m_palette_bank) &&
		(old_data != data))
	{
	//  m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
	}

	/* set the palette RAM value */
	m_palette->write8(offset, data);
}


void qix_state::palettebank_w(uint8_t data)
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


rgb_t qix_state::qix_R2G2B2I2(uint32_t raw)
{
	/* this conversion table should be about right. It gives a reasonable */
	/* gray scale in the test screen, and the red, green and blue squares */
	/* in the same screen are barely visible, as the manual requires. */
	static const uint8_t table[16] =
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

	/* compute R, G, B from the table */
	int const intensity = (raw >> 0) & 0x03;
	int bits = (raw >> 6) & 0x03;
	int const r = table[(bits << 2) | intensity];
	bits = (raw >> 4) & 0x03;
	int const g = table[(bits << 2) | intensity];
	bits = (raw >> 2) & 0x03;
	int const b = table[(bits << 2) | intensity];

	/* update the palette */
	return rgb_t(r, g, b);
}



/*************************************
 *
 *  M6845 callbacks for updating
 *  the screen
 *
 *************************************/

MC6845_BEGIN_UPDATE(qix_state::crtc_begin_update)
{
#if 0
	// note the confusing bit order!
	popmessage("self test leds: %d%d %d%d%d%d",BIT(leds,7),BIT(leds,5),BIT(leds,6),BIT(leds,4),BIT(leds,2),BIT(leds,3));
#endif
}


MC6845_UPDATE_ROW(qix_state::crtc_update_row)
{
	uint32_t *const dest = &bitmap.pix(y);
	pen_t const *const pens = &m_palette->pens()[m_palette_bank << 8];

	/* the memory is hooked up to the MA, RA lines this way */
	offs_t const offs = ((ma << 6) & 0xf800) | ((ra << 8) & 0x0700);
	offs_t const offs_xor = m_flip ? 0xffff : 0;

	for (uint16_t x = 0; x < x_count * 8; x++)
		dest[x] = pens[m_videoram[(offs + x) ^ offs_xor]];
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void qix_state::qix_video_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(qix_state::videoram_r), FUNC(qix_state::videoram_w));
	map(0x8000, 0x83ff).ram().share("sharedram");
	map(0x8400, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8800).mirror(0x03ff).w(FUNC(qix_state::palettebank_w));
	map(0x8c00, 0x8c00).mirror(0x03fe).rw(FUNC(qix_state::data_firq_r), FUNC(qix_state::data_firq_w));
	map(0x8c01, 0x8c01).mirror(0x03fe).rw(FUNC(qix_state::video_firq_ack_r), FUNC(qix_state::video_firq_ack_w));
	map(0x9000, 0x93ff).ram().w(FUNC(qix_state::paletteram_w)).share("palette");
	map(0x9400, 0x9400).mirror(0x03fc).rw(FUNC(qix_state::addresslatch_r), FUNC(qix_state::addresslatch_w));
	map(0x9402, 0x9403).mirror(0x03fc).writeonly().share(m_videoram_address);
	map(0x9800, 0x9800).mirror(0x03ff).readonly().share(m_scanline_latch);
	map(0x9c00, 0x9c00).mirror(0x03fe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x9c01, 0x9c01).mirror(0x03fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xa000, 0xffff).rom();
}


void kram3_state::video_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(kram3_state::videoram_r), FUNC(kram3_state::videoram_w));
	map(0x8000, 0x83ff).ram().share("sharedram");
	map(0x8400, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8800).mirror(0x03ff).w(FUNC(kram3_state::palettebank_w));
	map(0x8c00, 0x8c00).mirror(0x03fe).rw(FUNC(kram3_state::data_firq_r), FUNC(kram3_state::data_firq_w));
	map(0x8c01, 0x8c01).mirror(0x03fe).rw(FUNC(kram3_state::video_firq_ack_r), FUNC(kram3_state::video_firq_ack_w));
	map(0x9000, 0x93ff).ram().w(FUNC(kram3_state::paletteram_w)).share("palette");
	map(0x9400, 0x9400).mirror(0x03fc).rw(FUNC(kram3_state::addresslatch_r), FUNC(kram3_state::addresslatch_w));
	map(0x9402, 0x9403).mirror(0x03fc).writeonly().share(m_videoram_address);
	map(0x9800, 0x9800).mirror(0x03ff).readonly().share(m_scanline_latch);
	map(0x9c00, 0x9c00).mirror(0x03fe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x9c01, 0x9c01).mirror(0x03fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xa000, 0xffff).bankr(m_videobank);
}


void zookeep_state::video_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(zookeep_state::videoram_r), FUNC(zookeep_state::videoram_w));
	map(0x8000, 0x83ff).ram().share("sharedram");
	map(0x8400, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8800).mirror(0x03fe).w(FUNC(zookeep_state::palettebank_w));
	map(0x8801, 0x8801).mirror(0x03fe).w(FUNC(zookeep_state::bankswitch_w));
	map(0x8c00, 0x8c00).mirror(0x03fe).rw(FUNC(zookeep_state::data_firq_r), FUNC(zookeep_state::data_firq_w));
	map(0x8c01, 0x8c01).mirror(0x03fe).rw(FUNC(zookeep_state::video_firq_ack_r), FUNC(zookeep_state::video_firq_ack_w));
	map(0x9000, 0x93ff).ram().w(FUNC(zookeep_state::paletteram_w)).share("palette");
	map(0x9400, 0x9400).mirror(0x03fc).rw(FUNC(zookeep_state::addresslatch_r), FUNC(zookeep_state::addresslatch_w));
	map(0x9402, 0x9403).mirror(0x03fc).writeonly().share(m_videoram_address);
	map(0x9800, 0x9800).mirror(0x03ff).readonly().share(m_scanline_latch);
	map(0x9c00, 0x9c00).mirror(0x03fe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x9c01, 0x9c01).mirror(0x03fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xa000, 0xbfff).bankr(m_videobank);
	map(0xc000, 0xffff).rom();
}


void slither_state::video_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(slither_state::videoram_r), FUNC(slither_state::slither_videoram_w));
	map(0x8000, 0x83ff).ram().share("sharedram");
	map(0x8400, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8800).mirror(0x03ff).w(FUNC(slither_state::palettebank_w));
	map(0x8c00, 0x8c00).mirror(0x03fe).rw(FUNC(slither_state::data_firq_r), FUNC(slither_state::data_firq_w));
	map(0x8c01, 0x8c01).mirror(0x03fe).rw(FUNC(slither_state::video_firq_ack_r), FUNC(slither_state::video_firq_ack_w));
	map(0x9000, 0x93ff).ram().w(FUNC(slither_state::paletteram_w)).share("palette");
	map(0x9400, 0x9400).mirror(0x03fc).rw(FUNC(slither_state::addresslatch_r), FUNC(slither_state::slither_addresslatch_w));
	map(0x9401, 0x9401).mirror(0x03fc).writeonly().share(m_videoram_mask);
	map(0x9402, 0x9403).mirror(0x03fc).writeonly().share(m_videoram_address);
	map(0x9800, 0x9800).mirror(0x03ff).readonly().share(m_scanline_latch);
	map(0x9c00, 0x9c00).mirror(0x03fe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x9c01, 0x9c01).mirror(0x03fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xa000, 0xffff).rom();
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void qix_state::qix_video(machine_config &config)
{
	constexpr XTAL QIX_CHARACTER_CLOCK = (XTAL(20'000'000)/2/16);

	MC6809E(config, m_videocpu, MAIN_CLOCK_OSC/4/4); /* 1.25 MHz */
	m_videocpu->set_addrmap(AS_PROGRAM, &qix_state::qix_video_map);

	MC6845(config, m_crtc, QIX_CHARACTER_CLOCK);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_begin_update_callback(FUNC(qix_state::crtc_begin_update));
	m_crtc->set_update_row_callback(FUNC(qix_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(qix_state::display_enable_changed));
	m_crtc->out_vsync_callback().set(FUNC(qix_state::vsync_changed));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(QIX_CHARACTER_CLOCK*8, 0x148, 0, 0x100, 0x111, 0, 0x100); /* from CRTC */
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::BLACK).set_format(1, &qix_state::qix_R2G2B2I2, 0x400);
}

void kram3_state::video(machine_config &config)
{
	m_videocpu->set_addrmap(AS_PROGRAM, &kram3_state::video_map);
	m_videocpu->lic().set(FUNC(kram3_state::lic_videocpu_changed));
}

void zookeep_state::video(machine_config &config)
{
	m_videocpu->set_addrmap(AS_PROGRAM, &zookeep_state::video_map);
}

void slither_state::video(machine_config &config)
{
	m_videocpu->set_clock(SLITHER_CLOCK_OSC/4/4);   /* 1.34 MHz */
	m_videocpu->set_addrmap(AS_PROGRAM, &slither_state::video_map);
}
