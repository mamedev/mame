// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Vindicators hardware

    driver by Aaron Giles

    Games supported:
        * Vindicators (1988) [8 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"

#include "cpu/m68000/m68010.h"
#include "machine/eeprompar.h"
#include "machine/timer.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vindictr_state : public driver_device
{
public:
	vindictr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_jsa(*this, "jsa"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_260010(*this, "260010")
	{ }

	void vindictr(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;
	required_ioport m_260010;

	uint8_t m_playfield_tile_bank = 0;
	uint16_t m_playfield_xscroll = 0;
	uint16_t m_playfield_yscroll = 0;

	void scanline_interrupt();
	void scanline_int_ack_w(uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint16_t port1_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	static const atari_motion_objects_config s_mob_config;
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(vindictr_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0x3ff;
	int const color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int const opaque = data & 0x8000;
	tileinfo.set(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(vindictr_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = (m_playfield_tile_bank * 0x1000) + (data & 0xfff);
	int const color = 0x10 + 2 * ((data >> 12) & 7);
	tileinfo.set(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config vindictr_state::s_mob_config =
{
	0,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0x100,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0,0,0,0x03ff }}, // mask for the link
	{{ 0x7fff,0,0,0 }}, // mask for the code index
	{{ 0,0x000f,0,0 }}, // mask for the color
	{{ 0,0xff80,0,0 }}, // mask for the X position
	{{ 0,0,0xff80,0 }}, // mask for the Y position
	{{ 0,0,0x0038,0 }}, // mask for the width, in tiles
	{{ 0,0,0x0007,0 }}, // mask for the height, in tiles
	{{ 0,0,0x0040,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0x0070,0,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};

void vindictr_state::video_start()
{
	// save states
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}



/*************************************
 *
 *  Palette RAM control
 *
 *************************************/

void vindictr_state::paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	static const int ztable[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };

	// first blend the data
	COMBINE_DATA(&m_paletteram[offset]);
	data = m_paletteram[offset];

	// now generate colors at all 16 intensities
	for (int c = 0; c < 8; c++)
	{
		int const i = ztable[((data >> 12) + (c * 2)) & 15];
		int const r = ((data >> 8) & 15) * i;
		int const g = ((data >> 4) & 15) * i;
		int const b = ((data >> 0) & 15) * i;

		m_palette->set_pen_color(offset + c * 2048, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(vindictr_state::scanline_update)
{
	int const scanline = param;

	// keep in range
	int offset = ((scanline - 8) / 8) * 64 + 42;
	if (offset < 0)
		offset += 0x7c0;
	else if (offset >= 0x7c0)
		return;

	// update the current parameters
	for (int x = 42; x < 64; x++)
	{
		uint16_t const data = m_alpha_tilemap->basemem_read(offset++);

		switch ((data >> 9) & 7)
		{
			case 2:     // /PFB
				if (m_playfield_tile_bank != (data & 7))
				{
					m_screen->update_partial(scanline - 1);
					m_playfield_tile_bank = data & 7;
					m_playfield_tilemap->mark_all_dirty();
				}
				break;

			case 3:     // /PFHSLD
				if (m_playfield_xscroll != (data & 0x1ff))
				{
					m_screen->update_partial(scanline - 1);
					m_playfield_tilemap->set_scrollx(0, data);
					m_playfield_xscroll = data & 0x1ff;
				}
				break;

			case 4:     // /MOHS
				if (m_mob->xscroll() != (data & 0x1ff))
				{
					m_screen->update_partial(scanline - 1);
					m_mob->set_xscroll(data & 0x1ff);
				}
				break;

			case 5:     // /PFSPC
				break;

			case 6:     // /VIRQ
				scanline_interrupt();
				break;

			case 7:     // /PFVS
			{
				// a new vscroll latches the offset into a counter; we must adjust for this
				int offset = scanline;
				const rectangle &visible_area = m_screen->visible_area();
				if (offset > visible_area.bottom())
					offset -= visible_area.bottom() + 1;

				if (m_playfield_yscroll != ((data - offset) & 0x1ff))
				{
					m_screen->update_partial(scanline - 1);
					m_playfield_tilemap->set_scrolly(0, data - offset);
					m_mob->set_yscroll((data - offset) & 0x1ff);
				}
				break;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t vindictr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* partially verified via schematics (there are a lot of PALs involved!):

					    SHADE = PAL(MPR1-0, LB7-0, PFX6-5, PFX3-2, PF/M)

					    if (SHADE)
					        CRA |= 0x100

					    MOG3-1 = ~MAT3-1 if MAT6==1 and MSD3==1
					*/
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
						continue;

					// MO pen 1 doesn't draw, but it sets the SHADE flag and bumps the palette offset
					if ((mo[x] & 0x0f) == 1)
					{
						if ((mo[x] & 0xf0) != 0)
							pf[x] |= 0x100;
					}
					else
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

					// don't erase yet -- we need to make another pass later
				}
		}

	// add the alpha on top
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// now go back and process the upper bit of MO priority
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority might mean palette kludges
					if (mopriority & 4)
					{
						// if bit 2 is set, start setting high palette bits
						if (mo[x] & 2)
							m_mob->apply_stain(bitmap, pf, mo, x, y);

						// if the upper bit of pen data is set, we adjust the final intensity
						if (mo[x] & 8)
							pf[x] |= (~mo[x] & 0xe0) << 6;
					}
				}
		}
	return 0;
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void vindictr_state::scanline_interrupt()
{
	m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void vindictr_state::scanline_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

uint16_t vindictr_state::port1_r()
{
	int result = m_260010->read();
	result ^= 0x0010;
	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void vindictr_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fffff);
	map(0x000000, 0x05ffff).rom();
	map(0x0e0000, 0x0e03ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x1f0000, 0x1fffff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x26001f).r(FUNC(vindictr_state::port1_r));
	map(0x260020, 0x26002f).portr("260020");
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_i_device::main_response_r));
	map(0x2e0000, 0x2e0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x360000, 0x360001).w(FUNC(vindictr_state::scanline_int_ack_w));
	map(0x360010, 0x360011).nopw();
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_i_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_i_device::main_command_w));
	map(0x3e0000, 0x3e0fff).ram().w(FUNC(vindictr_state::paletteram_w)).share(m_paletteram);
	map(0x3f0000, 0x3f1fff).mirror(0x8000).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0x3f2000, 0x3f3fff).mirror(0x8000).ram().share("mob");
	map(0x3f4000, 0x3f4f7f).mirror(0x8000).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0x3f4f80, 0x3f4fff).mirror(0x8000).ram().share("mob:slip");
	map(0x3f5000, 0x3f7fff).mirror(0x8000).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( vindictr )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Left Stick Fire")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Right Stick Fire")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Left Stick Thumb")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Right Stick Thumb")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) // TODO: what's this?
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Left Stick Fire")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Right Stick Fire")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Left Stick Thumb")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Right Stick Thumb")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("260020")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )
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


static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_vindictr )
	GFXDECODE_ENTRY( "sprites_pf", 0, pfmolayout,  256, 32 )
	GFXDECODE_ENTRY( "chars",      0, anlayout,      0, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void vindictr_state::vindictr(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vindictr_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(vindictr_state::scanline_update), m_screen, 0, 8);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vindictr);
	PALETTE(config, m_palette).set_entries(2048*8);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(vindictr_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(vindictr_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, vindictr_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a SYNGEN chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(vindictr_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ATARI_JSA_I(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(12);
	m_jsa->add_route(0, "lspeaker", 1.0);
	m_jsa->add_route(1, "rspeaker", 1.0);
	config.device_remove("jsa:tms");
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( vindictr )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-5117.d1",  0x000000, 0x010000, CRC(2e5135e4) SHA1(804b3ba201088ac2c35cfcbd530acbd73548ea8c) )
	ROM_LOAD16_BYTE( "136059-5118.d3",  0x000001, 0x010000, CRC(e357fa79) SHA1(220a10287f4bf9d981fd412c8dd0a9c106eaf342) )
	ROM_LOAD16_BYTE( "136059-5119.f1",  0x020000, 0x010000, CRC(0deb7330) SHA1(e9fb311e96bcf57f2136fff87a973a5a3b5208b3) )
	ROM_LOAD16_BYTE( "136059-5120.f3",  0x020001, 0x010000, CRC(a6ae4753) SHA1(e69067ba0f1e5a4e446356e2fee3763dd4bcdd5a) )
	ROM_LOAD16_BYTE( "136059-5121.k1",  0x040000, 0x010000, CRC(96b150c5) SHA1(405c848f7990c981fefd355ca635bfb0ac24eb26) )
	ROM_LOAD16_BYTE( "136059-5122.k3",  0x040001, 0x010000, CRC(6415d312) SHA1(0115e32c1c42421cb3d978cc8642f7f88d492043) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictre )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-5717.d1",  0x000000, 0x010000, CRC(af5ba4a8) SHA1(fdb6e7f0707af94b39368cc39ae45c53209ce32e) )
	ROM_LOAD16_BYTE( "136059-5718.d3",  0x000001, 0x010000, CRC(c87b0581) SHA1(f33c72e83e8c811d3405deb470573327c7b68ea6) )
	ROM_LOAD16_BYTE( "136059-5719.f1",  0x020000, 0x010000, CRC(1e5f94e1) SHA1(bf14e4d3c26507ad3a78ad28b6b54e4ea0939ceb) )
	ROM_LOAD16_BYTE( "136059-5720.f3",  0x020001, 0x010000, CRC(cace40d7) SHA1(e897c56aa6134f39fc8e96f5ff96ca9c71623a32) )
	ROM_LOAD16_BYTE( "136059-5721.k1",  0x040000, 0x010000, CRC(96b150c5) SHA1(405c848f7990c981fefd355ca635bfb0ac24eb26) )
	ROM_LOAD16_BYTE( "136059-5722.k3",  0x040001, 0x010000, CRC(6415d312) SHA1(0115e32c1c42421cb3d978cc8642f7f88d492043) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictrg )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-1217.d1",  0x000000, 0x010000, CRC(0a589e9a) SHA1(6770212b57599cd9bcdeb126aec30d9815608005) )
	ROM_LOAD16_BYTE( "136059-1218.d3",  0x000001, 0x010000, CRC(e8b7959a) SHA1(b63747934b188f44a5e59a54f52d15b33f9d676b) )
	ROM_LOAD16_BYTE( "136059-1219.f1",  0x020000, 0x010000, CRC(2534fcbc) SHA1(d8a2121de88efabf99a153fd477c7bf2fddc88c9) )
	ROM_LOAD16_BYTE( "136059-1220.f3",  0x020001, 0x010000, CRC(d0947780) SHA1(5dc0f510f809eb2f75792cfdcfd35087d3aa28a6) )
	ROM_LOAD16_BYTE( "136059-1221.k1",  0x040000, 0x010000, CRC(ee1b1014) SHA1(ddfe01cdec4654a42c9e49660e3532e5c865a9b7) )
	ROM_LOAD16_BYTE( "136059-1222.k3",  0x040001, 0x010000, CRC(517b33f0) SHA1(f6430862bb00e11a68e964c89adcad1f05bc021b) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1223.16n", 0x000000, 0x004000, CRC(d27975bb) SHA1(a8ab8bdbd9fbcbcf73e8621b2a4447d25bf612b8) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictre4 )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-1117.d1",  0x000000, 0x010000, CRC(2e5135e4) SHA1(804b3ba201088ac2c35cfcbd530acbd73548ea8c) )
	ROM_LOAD16_BYTE( "136059-1118.d3",  0x000001, 0x010000, CRC(e357fa79) SHA1(220a10287f4bf9d981fd412c8dd0a9c106eaf342) )
	ROM_LOAD16_BYTE( "136059-4719.f1",  0x020000, 0x010000, CRC(3b27ab80) SHA1(330a6fe0e0265cce40c913aa5c3607429afe510b) )
	ROM_LOAD16_BYTE( "136059-4720.f3",  0x020001, 0x010000, CRC(e5ac9933) SHA1(6c9b617219d27678fae0af83f6eaa6bd95a02d35) )
	ROM_LOAD16_BYTE( "136059-4121.k1",  0x040000, 0x010000, CRC(9a0444ee) SHA1(211be931a8b6ca42dd140baf3e165ce23f75431f) )
	ROM_LOAD16_BYTE( "136059-4122.k3",  0x040001, 0x010000, CRC(d5022d78) SHA1(eeb6876ee6994f5736114a786c5c4ba97f26ef01) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictr4 )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-1117.d1",  0x000000, 0x010000, CRC(2e5135e4) SHA1(804b3ba201088ac2c35cfcbd530acbd73548ea8c) )
	ROM_LOAD16_BYTE( "136059-1118.d3",  0x000001, 0x010000, CRC(e357fa79) SHA1(220a10287f4bf9d981fd412c8dd0a9c106eaf342) )
	ROM_LOAD16_BYTE( "136059-4119.f1",  0x020000, 0x010000, CRC(44c77ee0) SHA1(f47307126a4960d59d19d1783497971f76ee00a5) )
	ROM_LOAD16_BYTE( "136059-4120.f3",  0x020001, 0x010000, CRC(4deaa77f) SHA1(1c582186d07f39dadf81e90a65928ff1520a60cc) )
	ROM_LOAD16_BYTE( "136059-4121.k1",  0x040000, 0x010000, CRC(9a0444ee) SHA1(211be931a8b6ca42dd140baf3e165ce23f75431f) )
	ROM_LOAD16_BYTE( "136059-4122.k3",  0x040001, 0x010000, CRC(d5022d78) SHA1(eeb6876ee6994f5736114a786c5c4ba97f26ef01) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictre3 )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-3117.d1",  0x000000, 0x010000, CRC(af5ba4a8) SHA1(fdb6e7f0707af94b39368cc39ae45c53209ce32e) )
	ROM_LOAD16_BYTE( "136059-3118.d3",  0x000001, 0x010000, CRC(c87b0581) SHA1(f33c72e83e8c811d3405deb470573327c7b68ea6) )
	ROM_LOAD16_BYTE( "136059-3119.f1",  0x020000, 0x010000, CRC(f0516142) SHA1(16f23a9a8939cead728108fc23fccebf2529d553) )
	ROM_LOAD16_BYTE( "136059-3120.f3",  0x020001, 0x010000, CRC(32a3729f) SHA1(cbddef0c4993e2d8cb6e70890dd5192de2cd56e0) )
	ROM_LOAD16_BYTE( "136059-2121.k1",  0x040000, 0x010000, CRC(9b6111e0) SHA1(427197b21a5db2a06751ab281fde7a2f63818db8) )
	ROM_LOAD16_BYTE( "136059-2122.k3",  0x040001, 0x010000, CRC(8d029a28) SHA1(a166d2a767f70050397f0f12add44ad1f5bc9fde) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictr2 )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-1117.d1",  0x000000, 0x010000, CRC(2e5135e4) SHA1(804b3ba201088ac2c35cfcbd530acbd73548ea8c) )
	ROM_LOAD16_BYTE( "136059-1118.d3",  0x000001, 0x010000, CRC(e357fa79) SHA1(220a10287f4bf9d981fd412c8dd0a9c106eaf342) )
	ROM_LOAD16_BYTE( "136059-2119.f1",  0x020000, 0x010000, CRC(7f8c044e) SHA1(56cd047ff12ff2968bf403b38b86fdceb9c2b83d) )
	ROM_LOAD16_BYTE( "136059-2120.f3",  0x020001, 0x010000, CRC(4260cd3b) SHA1(54fe16202e32ea6cf89da1837ff68b32eaf20dfc) )
	ROM_LOAD16_BYTE( "136059-2121.k1",  0x040000, 0x010000, CRC(9b6111e0) SHA1(427197b21a5db2a06751ab281fde7a2f63818db8) )
	ROM_LOAD16_BYTE( "136059-2122.k3",  0x040001, 0x010000, CRC(8d029a28) SHA1(a166d2a767f70050397f0f12add44ad1f5bc9fde) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END


ROM_START( vindictr1 )
	ROM_REGION( 0x60000, "maincpu", 0 )  // 68000 code
	ROM_LOAD16_BYTE( "136059-1117.d1",  0x000000, 0x010000, CRC(2e5135e4) SHA1(804b3ba201088ac2c35cfcbd530acbd73548ea8c) )
	ROM_LOAD16_BYTE( "136059-1118.d3",  0x000001, 0x010000, CRC(e357fa79) SHA1(220a10287f4bf9d981fd412c8dd0a9c106eaf342) )
	ROM_LOAD16_BYTE( "136059-1119.f1",  0x020000, 0x010000, CRC(48938c95) SHA1(061771b074135b945621d781fbde7ec1260f31a1) )
	ROM_LOAD16_BYTE( "136059-1120.f3",  0x020001, 0x010000, CRC(ed1de5e3) SHA1(3bf4faba019c63523d3fbd347075a2fdd5353345) )
	ROM_LOAD16_BYTE( "136059-1121.k1",  0x040000, 0x010000, CRC(9b6111e0) SHA1(427197b21a5db2a06751ab281fde7a2f63818db8) )
	ROM_LOAD16_BYTE( "136059-1122.k3",  0x040001, 0x010000, CRC(a94773f1) SHA1(2be841ab755d4ce319f3d562e9990918923384ee) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136059-1124.2k",  0x00000, 0x10000, CRC(d2212c0a) SHA1(df11fe76d74abc0cea23f18264cef4b0f33b1ffd) )

	ROM_REGION( 0x100000, "sprites_pf", ROMREGION_INVERT )
	ROM_LOAD( "136059-1104.12p", 0x000000, 0x020000, CRC(062f8e52) SHA1(0968b8c822d8fee1cf7ddcf9c3b1bf059e446417) )
	ROM_LOAD( "136059-1116.19p", 0x020000, 0x010000, CRC(0e4366fa) SHA1(1891f6b818f7b0e447e8a83ad0c12aade0b776ee) )
	ROM_RELOAD(                  0x030000, 0x010000 )
	ROM_LOAD( "136059-1103.8p",  0x040000, 0x020000, CRC(09123b57) SHA1(ddd5a4033b5f5ed45f639909364fe5ccd7a0cb53) )
	ROM_LOAD( "136059-1115.2p",  0x060000, 0x010000, CRC(6b757bca) SHA1(2d615b1b42f554bbfebc34928c106c3dd93dc7b2) )
	ROM_RELOAD(                  0x070000, 0x010000 )
	ROM_LOAD( "136059-1102.12r", 0x080000, 0x020000, CRC(a5268c4f) SHA1(99f1f1f2e88f8b2f235070e525aaed9aff6e91c6) )
	ROM_LOAD( "136059-1114.19r", 0x0a0000, 0x010000, CRC(609f619e) SHA1(64b5c2b0f5da07a9dd148aa19bb87e2b2cb1c395) )
	ROM_RELOAD(                  0x0b0000, 0x010000 )
	ROM_LOAD( "136059-1101.8r",  0x0c0000, 0x020000, CRC(2d07fdaa) SHA1(b6772fd764ddc1d2fa1c44c931b269aab9ad5e2b) )
	ROM_LOAD( "136059-1113.2r",  0x0e0000, 0x010000, CRC(0a2aba63) SHA1(e4780c790278034f0332697d5f06e6ed6b57d273) )
	ROM_RELOAD(                  0x0f0000, 0x010000 )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136059-1123.16n", 0x000000, 0x004000, CRC(f99b631a) SHA1(7a2430b6810c77b0f717d6e9d71823eadbcf6013) )

	ROM_REGION( 0x00800, "plds", 0 )
	ROM_LOAD( "pal16l8a-136059-1150.c3",  0x0000, 0x0104, CRC(09d02b00) SHA1(3851f0c0958db983ab907f64ac370a1051c2b76a) )
	ROM_LOAD( "pal16l8a-136059-1151.d17", 0x0200, 0x0104, CRC(797dcde7) SHA1(0c9db6610c40d3bf58117aa9bc8826d33f063dff) )
	ROM_LOAD( "pal16l8a-136059-1152.e17", 0x0400, 0x0104, CRC(56634c58) SHA1(c52db58572d0d8f8eeab6abf891455115b6ed146) )
	ROM_LOAD( "pal16r6a-136059-1153.n7",  0x0600, 0x0104, CRC(61076033) SHA1(c860835a8fa48e141f3d24732395ac35a4b908a4) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, vindictr,  0,        vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (rev 5)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictre, vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (Europe, rev 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictrg, vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (German, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictre4,vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (Europe, rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictr4, vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (rev 4)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictre3,vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (Europe, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictr2, vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (rev 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1988, vindictr1, vindictr, vindictr, vindictr, vindictr_state, empty_init, ROT0, "Atari Games", "Vindicators (rev 1)",         MACHINE_SUPPORTS_SAVE )
