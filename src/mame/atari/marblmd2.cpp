// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Supported Games:

    Marble Madness II (prototype)

    Not Dumped:

    Marble Man: Marble Madness II (earlier version of the game before it was changed to use Joysticks)

    -------------------------------------------

    TODO:
    - Issues in service mode (eg. RAM check fails) could just be prototype issues
    - verify XTALs, clocks, volume balance, dipswitches etc.

    Non-Bugs:
    - objects appear over text boxes during scroll before levels, verified to happen on hardware

*/

#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"
#include "atarivad.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class marblmd2_state : public driver_device
{
public:
	marblmd2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_mob(*this, "vad:mob")
	{ }

	void marblmd2(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<atari_motion_objects_device> m_mob;

	void marblmd2_map(address_map &map) ATTR_COLD;

	void latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_tempbitmap;
	uint16_t m_latch_data = 0U;

	static const atari_motion_objects_config s_mob_config;
};

void marblmd2_state::machine_start()
{
	save_item(NAME(m_latch_data));
}

void marblmd2_state::video_start()
{
	m_screen->register_screen_bitmap(m_tempbitmap);
	m_tempbitmap.fill(0);
}

uint32_t marblmd2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob().draw_async(cliprect);

	// draw the playfield
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield().draw(screen, m_tempbitmap, cliprect, 0, 0x00);
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const src = &m_tempbitmap.pix(y);
		uint16_t *const dst = &bitmap.pix(y);

		// top bit of the gfxdata appears to be priority, so we don't want it in the render bitmap
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			dst[x] = src[x] & 0x007f;
		}
	}

	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
	{
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const pf2 = &m_tempbitmap.pix(y);
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
			{
				if (mo[x] != 0xffff)
				{
					if (pf2[x] & 0x80) // check against top bit of temp pf render
					{
						if (mo[x] & 0x80)
							pf[x] = mo[x];
					}
					else
					{
						pf[x] = mo[x] | 0x80;
					}
				}
			}
		}
	}

	return 0;
}

// not verified
void marblmd2_state::latch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_latch_data);

	if (m_latch_data & 0x0010)
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


void marblmd2_state::marblmd2_map(address_map& map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x600000, 0x600001).portr("600000");
	map(0x600002, 0x600003).portr("600002");
	map(0x600010, 0x600011).portr("600010");
	map(0x600012, 0x600013).portr("600012");
	map(0x600020, 0x600021).portr("600020");
	map(0x600031, 0x600031).r(m_jsa, FUNC(atari_jsa_iii_device::main_response_r));
	map(0x600041, 0x600041).w(m_jsa, FUNC(atari_jsa_iii_device::main_command_w));

	map(0x600050, 0x600051).w(FUNC(marblmd2_state::latch_w));

	map(0x600060, 0x600061).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));

	map(0x601000, 0x601fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);

	map(0x607000, 0x607001).nopw();

	map(0x7c0000, 0x7c03ff).rw("palette", FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0xff00).share("palette");
	map(0x7cffc0, 0x7cffff).ram().rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));

	map(0x7d0000, 0x7d7fff).ram();
	map(0x7d8000, 0x7d9fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_lsb_w)).share("vad:playfield");
	map(0x7da000, 0x7dbeff).ram().share("vad:mob");
	map(0x7dbf00, 0x7dbf7f).ram().share("vad:eof");
	map(0x7dbf80, 0x7dbfff).ram().share("vad:mob:slip");
	map(0x7f8000, 0x7fbfff).ram();
}


static INPUT_PORTS_START( marblmd2 )
	PORT_START("600000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) // also acts as START3
	PORT_BIT( 0x00fe, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // also acts as START1
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("600002")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // acts as a 'freeze' input, probably not connected
	PORT_BIT( 0x00fe, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // also acts as START2
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("600012") // assume at least one bank of 8 dips lives here, but could be something else
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Number of Players (Test Mode)" ) // this one controls 'number of players' in Control Test
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Number of Players (Game)" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("600020")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("600010")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")   // Input buffer full
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")   // Output buffer full
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2) + 0, 8,RGN_FRAC(1,2) + 8,  16,RGN_FRAC(1,2) + 16,24,RGN_FRAC(1,2) + 24 },
	{ 0 * 32, 1 * 32, 2 * 32, 3 * 32, 4 * 32, 5 * 32, 6 * 32, 7 * 32 },
	8 * 32
};

static const gfx_layout molayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4, RGN_FRAC(1,2) + 8, RGN_FRAC(1,2) + 12, 8, 12  },
	{ 0 * 16, 1 * 16, 2 * 16, 3 * 16, 4 * 16, 5 * 16, 6 * 16, 7 * 16 },
	16 * 8
};


static GFXDECODE_START( gfx_mm2 )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout,   0x0, 1  )
	GFXDECODE_ENTRY( "gfx2", 0, molayout,   0x0, 0x10  )
GFXDECODE_END


const atari_motion_objects_config marblmd2_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x000,              // base palette entry
	0x100,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0x03ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }}, // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles
	{{ 0,0,0,0x0007 }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0,0x0070,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0,                  // resulting value to indicate "special"
};

TILE_GET_INFO_MEMBER(marblmd2_state::get_playfield_tile_info)
{
	uint16_t data1 = m_vad->playfield().basemem_read(tile_index);
	int code = data1 & 0x3fff;
	tileinfo.set(0, code, 0, (data1 >> 15) & 1);
}

// XTALs are guessed based on other Atari titles of the same period
void marblmd2_state::marblmd2(machine_config &config)
{
	M68000(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &marblmd2_state::marblmd2_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mm2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(marblmd2_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_vad->set_xoffsets(4, 4);

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, marblmd2_state::s_mob_config).set_gfxdecode("gfxdecode");

	TILEMAP(config, "vad:playfield", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(marblmd2_state::get_playfield_tile_info));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_III(config, m_jsa, 0);
	m_jsa->set_swapped_coins(true);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("600010").bit(6);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( marblmd2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "rom0l.18c",  0x00001, 0x20000, CRC(a4db40d9) SHA1(ae8313c9bb513143472347a7705bec33783bad7e) )
	ROM_LOAD16_BYTE( "rom0h.20c",  0x00000, 0x20000, CRC(d1a17d67) SHA1(7ac434858fa94e4bb4bb7d0603f699667494aa0d) )
	ROM_LOAD16_BYTE( "rom1l.18e",  0x40001, 0x20000, CRC(b6fb08b5) SHA1(0ce9c1a5d70133ffc879cfe548646c04de371e13) )
	ROM_LOAD16_BYTE( "rom1h.20e",  0x40000, 0x20000, CRC(b2a361a8) SHA1(b7c58404642a494532597cceb946463e3a6f56b3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "aud0.12c",  0x00000, 0x10000, CRC(89a8d90a) SHA1(cd73483d0bcfe2c8134d005c4417975f9a2cb658) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "pf0l.3p",  0x00000, 0x20000, CRC(a4fe377a) SHA1(a8a1a8027da778e5ad406a65814eec999b0f81af) )
	ROM_LOAD( "pf1l.3m",  0x20000, 0x20000, CRC(5dc7aaa8) SHA1(4fb815e9bcf6bcdf1b7976a3dea2b6d1dd6a8f6b) )
	ROM_LOAD( "pf2l.3k",  0x40000, 0x20000, CRC(0c7c5f74) SHA1(26f1d36f70f4e8354537d0d67764a1c9be35e8f7) )
	ROM_LOAD( "pf3l.3j",  0x60000, 0x20000, CRC(0a780429) SHA1(a9d7d564507c31dafc448726b04293d6a582cff5) )
	ROM_LOAD( "pf0h.1p",  0x80000, 0x20000, CRC(a6297a83) SHA1(ffe9ea41d1ba7bb3d0260f3fcf0e970112098d46) )
	ROM_LOAD( "pf1h.1m",  0xa0000, 0x20000, CRC(5b40f1bb) SHA1(cf0de8679ab0dd9460324ce72b4bfac029591506) )
	ROM_LOAD( "pf2h.1k",  0xc0000, 0x20000, CRC(18323df9) SHA1(9c4add4733bcfe7202b53d86f1bca4b9d207e22a) )
	ROM_LOAD( "pf3h.1j",  0xe0000, 0x20000, CRC(05d86ef8) SHA1(47eefd7112a3a3be16f0b4496cf034c8f7a69b1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mo0l.7p",  0x00000, 0x20000, CRC(950d95a3) SHA1(3f38da7b6eeaa87cc84b98c9d535468b0c060f6d) )
	ROM_LOAD( "mo1l.10p", 0x20000, 0x20000, CRC(b62b6ebf) SHA1(3781cd81780c10cd245871bb8f7b6260f7bb53b7) )
	ROM_LOAD( "mo0h.12p", 0x40000, 0x20000, CRC(e47d92b0) SHA1(7953e8342450c02408e4d90f132144d55de2f491) )
	ROM_LOAD( "mo1h.14p", 0x60000, 0x20000, CRC(317a03fb) SHA1(23a7cfe7c5601c858e8b346de31441788c7a8e97) )

	// loading based on batman, there are 2 unpopulated positions on the PCB
	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "sound.19e",  0x00000, 0x20000, CRC(e916bef7) SHA1(e07ddc8a3e1656d7307b767e692cf4a575ca47a3) )
	ROM_LOAD( "sound.12e",  0x60000, 0x20000, CRC(bab2f8e5) SHA1(bbe2d693d40e5eeba315fe7b6380a2030b66f23e) )
ROM_END

} // anonymous namespace


GAME( 1991, marblmd2, 0, marblmd2, marblmd2, marblmd2_state, empty_init, ROT0,  "Atari Games", "Marble Madness II (prototype)", MACHINE_SUPPORTS_SAVE )
