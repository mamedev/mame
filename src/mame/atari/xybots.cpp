// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari Xybots hardware

    driver by Aaron Giles

    Games supported:
        * Xybots (1987) [5 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"
#include "slapstic.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class xybots_state : public driver_device
{
public:
	xybots_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slapstic(*this, "slapstic"),
		m_slapstic_bank(*this, "slapstic_bank"),
		m_jsa(*this, "jsa"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_ffe200(*this, "FFE200")
	{ }

	void xybots(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<atari_slapstic_device> m_slapstic;
	required_memory_bank m_slapstic_bank;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_ioport m_ffe200;

	uint16_t m_h256 = 0;

	void video_int_ack_w(uint16_t data = 0);
	uint16_t special_port1_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;

	static const atari_motion_objects_config s_mob_config;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(xybots_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0x3ff;
	int const color = (data >> 12) & 7;
	int const opaque = data & 0x8000;
	tileinfo.set(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(xybots_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = data & 0x1fff;
	int const color = (data >> 11) & 0x0f;
	tileinfo.set(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config xybots_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	0,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	0,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0x300,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0x3f }},         // mask for the link (dummy)
	{{ 0x3fff,0,0,0 }}, // mask for the code index
	{{ 0,0,0,0x000f }}, // mask for the color
	{{ 0,0,0,0xff80 }}, // mask for the X position
	{{ 0,0,0xff80,0 }}, // mask for the Y position
	{{ 0 }},            // mask for the width, in tiles
	{{ 0,0,0x0007,0 }}, // mask for the height, in tiles
	{{ 0x8000,0,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0x000f,0,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t xybots_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
					/* verified via schematics:

					    PRIEN = ~(~MOPIX3 & ~MOPIX2 & ~MOPIX1) = (MOPIX3-0 > 1)

					    if (PRIEN)
					        PF/MO = (~MOPRI3-0 > PFCOL3-0)
					    else
					        PF/MO = (~MOPRI3-0 >= PFCOL3-0)

					    if (PF/MO | ~(PRIEN & MOCOL3))
					        GPC(P3-0) = PFPIX3-0
					    else
					        GPC(P3-0) = ~MOCOL3-0
					*/
					int const mopriority = (mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT) ^ 15;
					int const pfcolor = (pf[x] >> 4) & 0x0f;
					int const prien = ((mo[x] & 0x0f) > 1);

					if (prien)
					{
						if (mopriority <= pfcolor)
						{
							/* this first case doesn't make sense from the schematics, but it has
							   the correct effect */
							if (mo[x] & 0x80)
								pf[x] = (mo[x] ^ 0x2f0) & atari_motion_objects_device::DATA_MASK;
							else
								pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
						}
					}
					else
					{
						if (mopriority < pfcolor)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}
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

void xybots_state::video_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}



/*************************************
 *
 *  I/O handlers
 *
 *************************************/

uint16_t xybots_state::special_port1_r()
{
	int result = m_ffe200->read();
	result ^= m_h256 ^= 0x0400;
	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full map verified from schematics
void xybots_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x007fff).mirror(0x7c0000).rom();
	map(0x008000, 0x009fff).mirror(0x7c6000).bankr(m_slapstic_bank); // slapstic maps here
	map(0x010000, 0x03ffff).mirror(0x7c0000).rom();
	map(0x800000, 0x800fff).mirror(0x7f8000).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0x801000, 0x802dff).mirror(0x7f8000).ram();
	map(0x802e00, 0x802fff).mirror(0x7f8000).ram().share("mob");
	map(0x803000, 0x803fff).mirror(0x7f8000).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0x804000, 0x8047ff).mirror(0x7f8800).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x805000, 0x8053ff).mirror(0x7f8c00).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x806000, 0x8060ff).mirror(0x7f8000).r(m_jsa, FUNC(atari_jsa_i_device::main_response_r)).umask16(0x00ff);
	map(0x806100, 0x8061ff).mirror(0x7f8000).portr("FFE100");
	map(0x806200, 0x8062ff).mirror(0x7f8000).r(FUNC(xybots_state::special_port1_r));
	map(0x806800, 0x8068ff).mirror(0x7f8000).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x806900, 0x8069ff).mirror(0x7f8000).w(m_jsa, FUNC(atari_jsa_i_device::main_command_w)).umask16(0x00ff);
	map(0x806a00, 0x806aff).mirror(0x7f8000).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x806b00, 0x806bff).mirror(0x7f8000).w(FUNC(xybots_state::video_int_ack_w));
	map(0x806e00, 0x806eff).mirror(0x7f8000).w(m_jsa, FUNC(atari_jsa_i_device::sound_reset_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( xybots )
	PORT_START("FFE100")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Twist Right") PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Twist Left") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Twist Right")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Twist Left")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("FFE200")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDBUSY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_CUSTOM ) // 256H
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") // VBLANK
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )
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


static GFXDECODE_START( gfx_xybots )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_msb, 512, 16 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 256, 48 )
	GFXDECODE_ENTRY( "chars",   0, anlayout,               0, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void xybots_state::xybots(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &xybots_state::main_map);

	SLAPSTIC(config, m_slapstic, 107);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0x8000, 0xffff, 0x7c0000);
	m_slapstic->set_bank(m_slapstic_bank);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_xybots);
	PALETTE(config, "palette").set_format(palette_device::IRGB_4444, 1024);

	TILEMAP(config, m_playfield_tilemap, "gfxdecode", 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32).set_info_callback(FUNC(xybots_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, "gfxdecode", 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(xybots_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, xybots_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a SYNGEN chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(xybots_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_1, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ATARI_JSA_I(config, m_jsa, 0);
	m_jsa->set_swapped_coins(true);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	m_jsa->test_read_cb().set_ioport("FFE200").bit(8);
	m_jsa->add_route(0, "rspeaker", 1.0);
	m_jsa->add_route(1, "lspeaker", 1.0);
	config.device_remove("jsa:pokey");
	config.device_remove("jsa:tms");
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( xybots )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136054-2112.17cd", 0x000000, 0x010000, CRC(16d64748) SHA1(3c2ba8ec3185b69c4e1947ac842f2250ee35216e) )
	ROM_LOAD16_BYTE( "136054-2113.19cd", 0x000001, 0x010000, CRC(2677d44a) SHA1(23a3538df13a47f2fd78d4842b9f8b81e38c802e) )
	ROM_LOAD16_BYTE( "136054-2114.17b",  0x020000, 0x008000, CRC(d31890cb) SHA1(b58722a4dcc79e97484c2f5e35b8dbf8c3520bd9) )
	ROM_LOAD16_BYTE( "136054-2115.19b",  0x020001, 0x008000, CRC(750ab1b0) SHA1(0638de738bd804bde4b93cd23190ee0465887cf8) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136054-1116.2k",  0x00000, 0x10000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybotsg )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136054-3212.17cd", 0x000000, 0x010000, CRC(4cac5d7c) SHA1(79cdd754fb6055249dace31fe9f8939f13aae8ca) )
	ROM_LOAD16_BYTE( "136054-3213.19cd", 0x000001, 0x010000, CRC(bfcb0b00) SHA1(3e45f72051ea74b544c8578c6fc1284f925caa3d) )
	ROM_LOAD16_BYTE( "136054-3214.17b",  0x020000, 0x008000, CRC(4ad35093) SHA1(6d2d82fb481c68819ec6c87d483eed17d4ae5d1a) )
	ROM_LOAD16_BYTE( "136054-3215.19b",  0x020001, 0x008000, CRC(3a2afbaf) SHA1(61b88d15d95681eb24559d0696203cd4ee63d11f) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136054-1116.2k",  0x00000, 0x10000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybotsf )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136054-3612.17cd", 0x000000, 0x010000, CRC(b03a3f3c) SHA1(c88ad0ba5381562095f5b5a13d338d10fa0597f5) )
	ROM_LOAD16_BYTE( "136054-3613.19cd", 0x000001, 0x010000, CRC(ab33eb1f) SHA1(926c32f07c0bcc5832db3a1adf0357e55cae707a) )
	ROM_LOAD16_BYTE( "136054-3614.17b",  0x020000, 0x008000, CRC(7385e0b6) SHA1(98a69901069872b14413c1bfe48783fdb43c1c37) )
	ROM_LOAD16_BYTE( "136054-3615.19b",  0x020001, 0x008000, CRC(8e37b812) SHA1(40f973a49c4b40f3a5d982d332995e792f718dcc) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136054-1116.2k",  0x00000, 0x10000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybots1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136054-1112.17cd", 0x000000, 0x010000, CRC(2dbab363) SHA1(1473bf1246c6fb3e6b8b1f86a345b532ccf18e8d) )
	ROM_LOAD16_BYTE( "136054-1113.19cd", 0x000001, 0x010000, CRC(847b056e) SHA1(cc4b90f19d7eaee09569ba228c2654f64cec3200) )
	ROM_LOAD16_BYTE( "136054-1114.17b",  0x020000, 0x008000, CRC(7444f88f) SHA1(e2a27754a57a809398ee639fe5d0920b564d4c0b) )
	ROM_LOAD16_BYTE( "136054-1115.19b",  0x020001, 0x008000, CRC(848d072d) SHA1(c4d1181f0227200e60d99a99c1a83897275b055f) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136054-1116.2k",  0x00000, 0x10000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybots0 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136054-0112.17cd", 0x000000, 0x010000, CRC(4b830ac4) SHA1(1f6dc0c6648f74c4775b52e3f502e835a8741182) )
	ROM_LOAD16_BYTE( "136054-0113.19cd", 0x000001, 0x010000, CRC(dcfbf8a7) SHA1(0106cd7be55147f4b59e17391e5bb339aaf80535) )
	ROM_LOAD16_BYTE( "136054-0114.17b",  0x020000, 0x008000, CRC(18b875f7) SHA1(aa78553bd3556d0b209513ba80b782cfb0e3bb8b) )
	ROM_LOAD16_BYTE( "136054-0115.19b",  0x020001, 0x008000, CRC(7f116360) SHA1(d12c339ce973bd74be4a4ac9e9d293f6a6e358d6) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136054-0116.2k",  0x00000, 0x10000, BAD_DUMP CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) ) // not dumped from this PCB, ROM taken from another set instead

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136054-1102.12l", 0x000000, 0x008000, CRC(0d304e5b) SHA1(203c86c865667b1538f61c0950682fb17ebd9abb) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-1103.11l", 0x010000, 0x010000, CRC(a514da1d) SHA1(5af3c703e0c8e8d47123241ce39f202c88a8cdb0) )
	ROM_LOAD( "136054-1117.8l",  0x030000, 0x010000, CRC(6b79154d) SHA1(6fd47503c91a23f75046acd1ef8000b63f8e8ba6) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void xybots_state::machine_start()
{
	m_slapstic_bank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x2000);
	m_h256 = 0x0400;

	save_item(NAME(m_h256));
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, xybots,  0,      xybots, xybots, xybots_state, empty_init, ROT0, "Atari Games", "Xybots (rev 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, xybotsg, xybots, xybots, xybots, xybots_state, empty_init, ROT0, "Atari Games", "Xybots (German, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, xybotsf, xybots, xybots, xybots, xybots_state, empty_init, ROT0, "Atari Games", "Xybots (French, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, xybots1, xybots, xybots, xybots, xybots_state, empty_init, ROT0, "Atari Games", "Xybots (rev 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, xybots0, xybots, xybots, xybots, xybots_state, empty_init, ROT0, "Atari Games", "Xybots (rev 0)",         MACHINE_SUPPORTS_SAVE )
