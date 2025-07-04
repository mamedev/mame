// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari Toobin' hardware

    driver by Aaron Giles

    Games supported:
        * Toobin' (1988) [6 sets]

    Known bugs:
        * none at this time

    The video sync chain is almost identical to System 2, though many other
    hardware aspects are very different.

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"

#include "cpu/m68000/m68010.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class toobin_state : public driver_device
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_interrupt_scan(*this, "interrupt_scan"),
		m_xscroll(*this, "xscroll"),
		m_yscroll(*this, "yscroll")
	{ }

	void toobin(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_interrupt_scan;
	required_shared_ptr<uint16_t> m_xscroll;
	required_shared_ptr<uint16_t> m_yscroll;

	double m_brightness = 0;
	bitmap_ind16 m_pfbitmap;

	emu_timer *m_scanline_interrupt_timer = nullptr;

	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data);

	void interrupt_scan_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void intensity_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void slip_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

	static const atari_motion_objects_config s_mob_config;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(toobin_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0x3ff;
	int const color = (data >> 12) & 0x0f;
	tileinfo.set(2, code, color, (data >> 10) & 1);
}


TILE_GET_INFO_MEMBER(toobin_state::get_playfield_tile_info)
{
	uint32_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = data & 0x3fff;
	int const color = (data >> 16) & 0x0f;
	tileinfo.set(0, code, color, TILE_FLIPYX(data >> 14));
	tileinfo.category = (data >> 20) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config toobin_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	1,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	1024,               // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0,                  // transparent pen index

	{{ 0,0,0x00ff,0 }}, // mask for the link
	{{ 0,0x3fff,0,0 }}, // mask for the code index
	{{ 0,0,0,0x000f }}, // mask for the color
	{{ 0,0,0,0xffc0 }}, // mask for the X position
	{{ 0x7fc0,0,0,0 }}, // mask for the Y position
	{{ 0x0007,0,0,0 }}, // mask for the width, in tiles
	{{ 0x0038,0,0,0 }}, // mask for the height, in tiles
	{{ 0,0x4000,0,0 }}, // mask for the horizontal flip
	{{ 0,0x8000,0,0 }}, // mask for the vertical flip
	{{ 0 }},            // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0x8000,0,0,0 }}, // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};

void toobin_state::video_start()
{
	// allocate a playfield bitmap for rendering
	m_screen->register_screen_bitmap(m_pfbitmap);

	save_item(NAME(m_brightness));
}



/*************************************
 *
 *  Palette RAM write handler
 *
 *************************************/

void toobin_state::paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	uint16_t const newword = m_paletteram[offset];

	{
		int red = (((newword >> 10) & 31) * 224) >> 5;
		int green = (((newword >> 5) & 31) * 224) >> 5;
		int blue = ((newword & 31) * 224) >> 5;

		if (red) red += 38;
		if (green) green += 38;
		if (blue) blue += 38;

		m_palette->set_pen_color(offset & 0x3ff, rgb_t(red, green, blue));
		if (BIT(~newword, 15))
			m_palette->set_pen_contrast(offset & 0x3ff, m_brightness);
		else
			m_palette->set_pen_contrast(offset & 0x3ff, 1.0);
	}
}


void toobin_state::intensity_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_brightness = (double)(~data & 0x1f) / 31.0;

		for (int i = 0; i < 0x400; i++)
			if (BIT(~m_paletteram[i], 15))
				m_palette->set_pen_contrast(i, m_brightness);
	}
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

void toobin_state::xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldscroll = *m_xscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	// if anything has changed, force a partial update
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	// update the playfield scrolling - hscroll is clocked on the following scanline
	m_playfield_tilemap->set_scrollx(0, newscroll >> 6);
	m_mob->set_xscroll(newscroll >> 6);

	// update the data
	*m_xscroll = newscroll;
}


void toobin_state::yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldscroll = *m_yscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	// if anything has changed, force a partial update
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	// if bit 4 is zero, the scroll value is clocked in right away
	m_playfield_tilemap->set_scrolly(0, newscroll >> 6);
	m_mob->set_yscroll((newscroll >> 6) & 0x1ff);

	// update the data
	*m_yscroll = newscroll;
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

void toobin_state::slip_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldslip = m_mob->slipram(offset);
	uint16_t newslip = oldslip;
	COMBINE_DATA(&newslip);

	// if the SLIP is changing, force a partial update first
	if (oldslip != newslip)
		m_screen->update_partial(m_screen->vpos());

	// update the data
	m_mob->slipram(offset) = newslip;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t toobin_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 3, 3);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	pen_t const *const palette = m_palette->pens();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dest = &bitmap.pix(y);
		uint16_t const *const mo = &mobitmap.pix(y);
		uint16_t const *const pf = &m_pfbitmap.pix(y);
		uint8_t const *const pri = &priority_bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint16_t pix = pf[x];
			if (mo[x] != 0xffff)
			{
				/* not verified: logic is all controlled in a PAL

				   factors: LBPRI1-0, LBPIX3, ANPIX1-0, PFPIX3, PFPRI1-0,
				            (~LBPIX3 & ~LBPIX2 & ~LBPIX1 & ~LBPIX0)
				*/

				// only draw if not high priority PF
				if (!pri[x] || !(pix & 8))
					pix = mo[x];
			}
			dest[x] = palette[pix];
		}
	}

	// add the alpha on top
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

TIMER_CALLBACK_MEMBER(toobin_state::scanline_interrupt)
{
	m_maincpu->set_input_line(M68K_IRQ_IPL0, ASSERT_LINE);
	m_scanline_interrupt_timer->adjust(m_screen->frame_period());
}

void toobin_state::machine_start()
{
	m_scanline_interrupt_timer = timer_alloc(FUNC(toobin_state::scanline_interrupt), this);
}



/*************************************
 *
 *  Interrupt handlers
 *
 *************************************/

void toobin_state::interrupt_scan_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const oldword = m_interrupt_scan[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	// if something changed, update the word in memory
	if (oldword != newword)
	{
		m_interrupt_scan[offset] = newword;
		m_scanline_interrupt_timer->adjust(m_screen->time_until_pos(newword & 0x1ff));
	}
}

void toobin_state::scanline_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_IPL0, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full address map decoded from schematics
void toobin_state::main_map(address_map &map)
{
	map.global_mask(0xc7ffff);
	map(0x000000, 0x07ffff).rom();
	map(0xc00000, 0xc07fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0xc08000, 0xc097ff).mirror(0x046000).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0xc09800, 0xc09fff).mirror(0x046000).ram().share("mob");
	map(0xc10000, 0xc107ff).mirror(0x047800).ram().w(FUNC(toobin_state::paletteram_w)).share(m_paletteram);
	map(0x826000, 0x826001).mirror(0x4500fe).nopr();     // who knows? read at controls time
	map(0x828000, 0x828001).mirror(0x4500fe).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x828101, 0x828101).mirror(0x4500fe).w(m_jsa, FUNC(atari_jsa_i_device::main_command_w));
	map(0x828300, 0x828301).mirror(0x45003e).w(FUNC(toobin_state::intensity_w));
	map(0x828340, 0x828341).mirror(0x45003e).w(FUNC(toobin_state::interrupt_scan_w)).share(m_interrupt_scan);
	map(0x828380, 0x828381).mirror(0x45003e).ram().w(FUNC(toobin_state::slip_w)).share("mob:slip");
	map(0x8283c0, 0x8283c1).mirror(0x45003e).w(FUNC(toobin_state::scanline_int_ack_w));
	map(0x828400, 0x828401).mirror(0x4500fe).w(m_jsa, FUNC(atari_jsa_i_device::sound_reset_w));
	map(0x828500, 0x828501).mirror(0x4500fe).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x828600, 0x828601).mirror(0x4500fe).w(FUNC(toobin_state::xscroll_w)).share(m_xscroll);
	map(0x828700, 0x828701).mirror(0x4500fe).w(FUNC(toobin_state::yscroll_w)).share(m_yscroll);
	map(0x828800, 0x828801).mirror(0x4507fe).portr("FF8800");
	map(0x829000, 0x829001).mirror(0x4507fe).portr("FF9000");
	map(0x829801, 0x829801).mirror(0x4507fe).r(m_jsa, FUNC(atari_jsa_i_device::main_response_r));
	map(0x82a000, 0x82a3ff).mirror(0x451c00).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x82c000, 0x82ffff).mirror(0x450000).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( toobin )
	PORT_START("FF8800")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 R Paddle Forward") PORT_CODE(KEYCODE_L) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 L Paddle Forward") PORT_CODE(KEYCODE_J) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 L Paddle Backward") PORT_CODE(KEYCODE_U) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 R Paddle Backward") PORT_CODE(KEYCODE_O) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 R Paddle Forward") PORT_CODE(KEYCODE_D) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 L Paddle Forward") PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 L Paddle Backward") PORT_CODE(KEYCODE_Q) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 R Paddle Backward") PORT_CODE(KEYCODE_E) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Throw") PORT_CODE(KEYCODE_LCONTROL) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Throw") PORT_CODE(KEYCODE_RCONTROL) PORT_PLAYER(2)
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FF9000")
	PORT_BIT( 0x03ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::hblank))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout molayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	8*64
};


static GFXDECODE_START( gfx_toobin )
	GFXDECODE_ENTRY( "tiles",   0, pflayout,     0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, molayout,   256, 16 )
	GFXDECODE_ENTRY( "chars",   0, anlayout,   512, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void toobin_state::toobin(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 32_MHz_XTAL;

	// basic machine hardware
	m68010_device &maincpu(M68010(config, m_maincpu, MASTER_CLOCK / 4));
	maincpu.set_addrmap(AS_PROGRAM, &toobin_state::main_map);
	maincpu.set_interrupt_mixer(false);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	// video hardware
	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 4, 8, 8, TILEMAP_SCAN_ROWS, 128, 64).set_info_callback(FUNC(toobin_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 48, 0).set_info_callback(FUNC(toobin_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, toobin_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(MASTER_CLOCK / 2, 640, 0, 512, 416, 0, 384);
	m_screen->set_screen_update(FUNC(toobin_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_toobin);
	PALETTE(config, m_palette).set_entries(1024);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ATARI_JSA_I(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_IPL1);
	m_jsa->test_read_cb().set_ioport("FF9000").bit(12);
	m_jsa->add_route(0, "speaker", 1.0, 0);
	m_jsa->add_route(1, "speaker", 1.0, 1);
	config.device_remove("jsa:tms");
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( toobin )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "3133-1j.061",  0x000000, 0x010000, CRC(79a92d02) SHA1(72eebb96a3963f94558bb204e0afe08f2b4c1864) )
	ROM_LOAD16_BYTE( "3137-1f.061",  0x000001, 0x010000, CRC(e389ef60) SHA1(24861fe5eb49de852987993a905fefe4dd43b204) )
	ROM_LOAD16_BYTE( "3134-2j.061",  0x020000, 0x010000, CRC(3dbe9a48) SHA1(37fe2534fed5708a63995e53ea0cb1d2d23fc1b9) )
	ROM_LOAD16_BYTE( "3138-2f.061",  0x020001, 0x010000, CRC(a17fb16c) SHA1(ae0a2c675a88dfaafffe47971c46c83dc7552148) )
	ROM_LOAD16_BYTE( "3135-4j.061",  0x040000, 0x010000, CRC(dc90b45c) SHA1(78c648be8e0aec6d1be45f909f2e468f3b572957) )
	ROM_LOAD16_BYTE( "3139-4f.061",  0x040001, 0x010000, CRC(6f8a719a) SHA1(bca7280155a4c44f55b402aed59927343c651acc) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END


ROM_START( toobine )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "3733-1j.061",  0x000000, 0x010000, CRC(286c7fad) SHA1(1f06168327bdc356f1bc4cf9a951f914932c491a) )
	ROM_LOAD16_BYTE( "3737-1f.061",  0x000001, 0x010000, CRC(965c161d) SHA1(30d959a945cb7dc7f00ad4ca9db027a377024030) )
	ROM_LOAD16_BYTE( "3134-2j.061",  0x020000, 0x010000, CRC(3dbe9a48) SHA1(37fe2534fed5708a63995e53ea0cb1d2d23fc1b9) )
	ROM_LOAD16_BYTE( "3138-2f.061",  0x020001, 0x010000, CRC(a17fb16c) SHA1(ae0a2c675a88dfaafffe47971c46c83dc7552148) )
	ROM_LOAD16_BYTE( "3135-4j.061",  0x040000, 0x010000, CRC(dc90b45c) SHA1(78c648be8e0aec6d1be45f909f2e468f3b572957) )
	ROM_LOAD16_BYTE( "3139-4f.061",  0x040001, 0x010000, CRC(6f8a719a) SHA1(bca7280155a4c44f55b402aed59927343c651acc) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END


ROM_START( toobing )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "3233-1j.061",  0x000000, 0x010000, CRC(b04eb760) SHA1(760525b4f72fad47cfc457e14db70ade30a9ddac) )
	ROM_LOAD16_BYTE( "3237-1f.061",  0x000001, 0x010000, CRC(4e41a470) SHA1(3a4c9b0d93cf4cff80978c0568bb9ef9eeb878dd) )
	ROM_LOAD16_BYTE( "3234-2j.061",  0x020000, 0x010000, CRC(8c60f1b4) SHA1(0ff3f4fede83410d73027b6e7445e83044e4b21e) )
	ROM_LOAD16_BYTE( "3238-2f.061",  0x020001, 0x010000, CRC(c251b3a2) SHA1(a12add64541dc300611cd5beea642037dc8eb4d0) )
	ROM_LOAD16_BYTE( "3235-4j.061",  0x040000, 0x010000, CRC(1121b5f4) SHA1(54ef54b7104366626ffe9b9f86793de24dc5e5d4) )
	ROM_LOAD16_BYTE( "3239-4f.061",  0x040001, 0x010000, CRC(385c5a80) SHA1(e0cab1b6ac178b90f4f95d28cdf9470aad1ac92f) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END


ROM_START( toobin2e )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2733-1j.061",  0x000000, 0x010000, CRC(a6334cf7) SHA1(39e540619c24af65bda44160a5bdaebf3600b64b) )
	ROM_LOAD16_BYTE( "2737-1f.061",  0x000001, 0x010000, CRC(9a52dd20) SHA1(a370ae3e4c7af55ea61b57a203a900f2be3ce6b9) )
	ROM_LOAD16_BYTE( "2134-2j.061",  0x020000, 0x010000, CRC(2b8164c8) SHA1(aeeaff9df9fda23b295b59efadf52160f084d256) )
	ROM_LOAD16_BYTE( "2138-2f.061",  0x020001, 0x010000, CRC(c09cadbd) SHA1(93598a512d17664c111e3d88397fde37a492b4a6) )
	ROM_LOAD16_BYTE( "2135-4j.061",  0x040000, 0x010000, CRC(90477c4a) SHA1(69b4bcf5c329d8710d0985ce3e45bd40a7102a91) )
	ROM_LOAD16_BYTE( "2139-4f.061",  0x040001, 0x010000, CRC(47936958) SHA1(ac7c99272f3b21d15e5673d2e8f206d60c32f4f9) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END


ROM_START( toobin2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2133-1j.061",  0x000000, 0x010000, CRC(2c3382e4) SHA1(39919e9b5b586b630e0581adabfe25d83b2bfaef) )
	ROM_LOAD16_BYTE( "2137-1f.061",  0x000001, 0x010000, CRC(891c74b1) SHA1(2f39d0e4934ccf48bb5fc0737f34fc5a65cfd903) )
	ROM_LOAD16_BYTE( "2134-2j.061",  0x020000, 0x010000, CRC(2b8164c8) SHA1(aeeaff9df9fda23b295b59efadf52160f084d256) )
	ROM_LOAD16_BYTE( "2138-2f.061",  0x020001, 0x010000, CRC(c09cadbd) SHA1(93598a512d17664c111e3d88397fde37a492b4a6) )
	ROM_LOAD16_BYTE( "2135-4j.061",  0x040000, 0x010000, CRC(90477c4a) SHA1(69b4bcf5c329d8710d0985ce3e45bd40a7102a91) )
	ROM_LOAD16_BYTE( "2139-4f.061",  0x040001, 0x010000, CRC(47936958) SHA1(ac7c99272f3b21d15e5673d2e8f206d60c32f4f9) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END


ROM_START( toobin1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "1133-1j.061",  0x000000, 0x010000, CRC(caeb5d1b) SHA1(8036871a04b5206fd383ac0fd9a9d3218128088b) )
	ROM_LOAD16_BYTE( "1137-1f.061",  0x000001, 0x010000, CRC(9713d9d3) SHA1(55791150312de201bdd330bfd4cbb132cb3959e4) )
	ROM_LOAD16_BYTE( "1134-2j.061",  0x020000, 0x010000, CRC(119f5d7b) SHA1(edd0b1ab29bb9c15c3b80037635c3b6d5fb434dc) )
	ROM_LOAD16_BYTE( "1138-2f.061",  0x020001, 0x010000, CRC(89664841) SHA1(4ace8e4fd0026d0d73726d339a71d841652fdc87) )
	ROM_LOAD16_BYTE( "1135-4j.061",  0x040000, 0x010000, CRC(90477c4a) SHA1(69b4bcf5c329d8710d0985ce3e45bd40a7102a91) )
	ROM_LOAD16_BYTE( "1139-4f.061",  0x040001, 0x010000, CRC(a9f082a9) SHA1(b1d45e528d466efa3f7562c80d2ee0c8913a33a6) )
	ROM_LOAD16_BYTE( "1136-5j.061",  0x060000, 0x010000, CRC(5ae3eeac) SHA1(583b6c3f61e8ad4d98449205fedecf3e21ee993c) )
	ROM_LOAD16_BYTE( "1140-5f.061",  0x060001, 0x010000, CRC(dacbbd94) SHA1(0e3a93f439ff9f3dd57ee13604be02e9c74c8eec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "1141-2k.061",  0x00000, 0x10000, CRC(c0dcce1a) SHA1(285c13f08020cf5827eca2afcc2fa8a3a0a073e0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "1101-1a.061",  0x000000, 0x010000, CRC(02696f15) SHA1(51856c331c45d287e574e2e4013b62a6472ad720) )
	ROM_LOAD( "1102-2a.061",  0x010000, 0x010000, CRC(4bed4262) SHA1(eda16ece14cb60012edbe006b2839986d082822e) )
	ROM_LOAD( "1103-4a.061",  0x020000, 0x010000, CRC(e62b037f) SHA1(9a2341b822265269c07b65c4bc0fbc760c1bd456) )
	ROM_LOAD( "1104-5a.061",  0x030000, 0x010000, CRC(fa05aee6) SHA1(db0dbf94ba1f2c1bb3ad55df2f38a71b4ecb38e4) )
	ROM_LOAD( "1105-1b.061",  0x040000, 0x010000, CRC(ab1c5578) SHA1(e80a1c7d2f279a523dcc9d943bd5a1ce75045d2e) )
	ROM_LOAD( "1106-2b.061",  0x050000, 0x010000, CRC(4020468e) SHA1(fa83e3d903d254c598fcbf120492ac77777ae31f) )
	ROM_LOAD( "1107-4b.061",  0x060000, 0x010000, CRC(fe6f6aed) SHA1(11bd17be3c9fe409db8268cb17515040bfd92ee2) )
	ROM_LOAD( "1108-5b.061",  0x070000, 0x010000, CRC(26fe71e1) SHA1(cac22f969c943e184a58d7bb62072f93273638de) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "1143-10a.061", 0x000000, 0x020000, CRC(211c1049) SHA1(fcf4d9321b2871723a10b7607069da83d3402723) )
	ROM_LOAD( "1144-13a.061", 0x020000, 0x020000, CRC(ef62ed2c) SHA1(c2c21023b2f559b8a63e6ae9c59c33a3055cc465) )
	ROM_LOAD( "1145-16a.061", 0x040000, 0x020000, CRC(067ecb8a) SHA1(a42e4602e1de1cc83a30a901a9adb5519f426cff) )
	ROM_LOAD( "1146-18a.061", 0x060000, 0x020000, CRC(fea6bc92) SHA1(c502ab022fdafdef71a720237094fe95c3137d69) )
	ROM_LOAD( "1125-21a.061", 0x080000, 0x010000, CRC(c37f24ac) SHA1(341fab8244d8063a516a4a25d7ee778f708cd386) )
	ROM_RELOAD(               0x0c0000, 0x010000 )
	ROM_LOAD( "1126-23a.061", 0x090000, 0x010000, CRC(015257f0) SHA1(c5ae6a43b95ecb06a04ea877f435b1f925cff136) )
	ROM_RELOAD(               0x0d0000, 0x010000 )
	ROM_LOAD( "1127-24a.061", 0x0a0000, 0x010000, CRC(d05417cb) SHA1(5cbd54050364e82fe443ca2150c34fca84c42419) )
	ROM_RELOAD(               0x0e0000, 0x010000 )
	ROM_LOAD( "1128-25a.061", 0x0b0000, 0x010000, CRC(fba3e203) SHA1(5951571e6e64061e5448cae27af0cedd5bda2b1e) )
	ROM_RELOAD(               0x0f0000, 0x010000 )
	ROM_LOAD( "1147-10b.061", 0x100000, 0x020000, CRC(ca4308cf) SHA1(966970524915a0a5a77e3525e446b50ecde5b119) )
	ROM_LOAD( "1148-13b.061", 0x120000, 0x020000, CRC(23ddd45c) SHA1(8ee19e8982a928b56d6010f283fb2f720dc71cd6) )
	ROM_LOAD( "1149-16b.061", 0x140000, 0x020000, CRC(d77cd1d0) SHA1(148fa17c9b7a2453adf059325cb608073d1ef195) )
	ROM_LOAD( "1150-18b.061", 0x160000, 0x020000, CRC(a37157b8) SHA1(347cea600f28709fc3d942feb5cadce7def72dbb) )
	ROM_LOAD( "1129-21b.061", 0x180000, 0x010000, CRC(294aaa02) SHA1(69b42dfc444b2c9f2bb0fdcb96e2becb0df6226a) )
	ROM_RELOAD(               0x1c0000, 0x010000 )
	ROM_LOAD( "1130-23b.061", 0x190000, 0x010000, CRC(dd610817) SHA1(25f542ae4e7e77399d6df328edbc4cceb390db04) )
	ROM_RELOAD(               0x1d0000, 0x010000 )
	ROM_LOAD( "1131-24b.061", 0x1a0000, 0x010000, CRC(e8e2f919) SHA1(292dacb60db867cb2ae69942c7502af6526ab550) )
	ROM_RELOAD(               0x1e0000, 0x010000 )
	ROM_LOAD( "1132-25b.061", 0x1b0000, 0x010000, CRC(c79f8ffc) SHA1(6e90044755097387011e7cc04548bedb399b7cc3) )
	ROM_RELOAD(               0x1f0000, 0x010000 )

	ROM_REGION( 0x004000, "chars", 0 )
	ROM_LOAD( "1142-20h.061", 0x000000, 0x004000, CRC(a6ab551f) SHA1(6a11e16f3965416c81737efcb81e751484ba5ace) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, toobin,   0,      toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (rev 3)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, toobine,  toobin, toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (Europe, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, toobing,  toobin, toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (German, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, toobin2,  toobin, toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (rev 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, toobin2e, toobin, toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (Europe, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, toobin1,  toobin, toobin, toobin, toobin_state, empty_init, ROT270, "Atari Games", "Toobin' (rev 1)",         MACHINE_SUPPORTS_SAVE )
