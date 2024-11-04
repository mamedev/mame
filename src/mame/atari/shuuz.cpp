// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari Shuuz hardware

    driver by Aaron Giles

    Games supported:
        * Shuuz (1990) [2 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarimo.h"
#include "atarivad.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shuuz_state : public driver_device
{
public:
	shuuz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_vad(*this, "vad")
		, m_system(*this, "SYSTEM")
		, m_track(*this, "TRACK%c", 'X')
	{ }

	void shuuz(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_vad_device> m_vad;

	required_ioport m_system;
	required_ioport_array<2> m_track;

	int16_t m_cur[2]{};

	static const atari_motion_objects_config s_mob_config;

	void latch_w(uint16_t data);
	uint16_t leta_r(offs_t offset);
	uint16_t special_port0_r();

	int get_hblank() const { return (m_screen->hpos() > (m_screen->width() * 9 / 10)); }

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(shuuz_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield().extmem_read(tile_index) >> 8;
	int const code = data1 & 0x3fff;
	int const color = data2 & 0x0f;
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config shuuz_state::s_mob_config =
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

	{{ 0x00ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }}, // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles
	{{ 0,0,0,0x0007 }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0 }},            // mask for the priority
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

uint32_t shuuz_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob().draw_async(cliprect);

	// draw the playfield
	m_vad->playfield().draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
			{
				if (mo[x] != 0xffff)
				{
					/* verified from the GALs on the real PCB; equations follow
					 *
					 *      --- O13 is 1 if (PFS7-4 == 0xf)
					 *      O13=PFS6*PFS7*(PFS5&PFS4)
					 *
					 *      --- PF/M is 1 if MOs have priority, or 0 if playfield has priority
					 *      MO/PF=!PFS7*!(LBD7&LBD6)*!M1*!O13
					 *         +!PFS7*!(LBD7&LBD6)*!M2*!O13
					 *         +!PFS7*!(LBD7&LBD6)*!M3*!O13
					 *         +PFS7*(LBD7&LBD6)*!M1*!O13
					 *         +PFS7*(LBD7&LBD6)*!M2*!O13
					 *         +PFS7*(LBD7&LBD6)*!M3*!O13
					 *
					 */

					// This is based on observations, and not verified against schematics and GAL equations.
					// TODO:
					// * Locate schematics for (or trace out) video mixing section.
					// * Obtain equations for video mixing GALs.
					bool const o13 = (pf[x] & 0xf0) == 0xf0;
					bool const mopf = ((pf[x] & 0x80) ? ((mo[x] & 0xc0) == 0xc0) : ((mo[x] & 0xc0) != 0xc0)) && !o13;

					// if MO/PF is asserted, we draw the MO
					if (mopf)
					{
						if (mo[x] & 0x0e)       // solid colors
							pf[x] = mo[x];
						else if (mo[x] & 0x01)  // shadows
							pf[x] |= 0x200;
					}
				}
			}
		}
	return 0;
}


void shuuz_state::machine_start()
{
	save_item(NAME(m_cur));
}

/*************************************
 *
 *  Initialization
 *
 *************************************/

void shuuz_state::latch_w(uint16_t data) // TODO
{
}



/*************************************
 *
 *  LETA I/O
 *
 *************************************/

uint16_t shuuz_state::leta_r(offs_t offset)
{
	// trackball -- rotated 45 degrees?
	int const which = offset & 1;

	// when reading the even ports, do a real analog port update
	if (which == 0)
	{
		int const dx = (int8_t)m_track[0]->read();
		int const dy = (int8_t)m_track[1]->read();

		m_cur[0] = dx + dy;
		m_cur[1] = dx - dy;
	}

	return m_cur[which];
}



/*************************************
 *
 *  Additional I/O
 *
 *************************************/

uint16_t shuuz_state::special_port0_r()
{
	int result = m_system->read();

	if ((result & 0x0800) && get_hblank())
		result &= ~0x0800;

	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void shuuz_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x101000, 0x101fff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x102000, 0x102001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x103000, 0x103003).r(FUNC(shuuz_state::leta_r));
	map(0x105000, 0x105001).rw(FUNC(shuuz_state::special_port0_r), FUNC(shuuz_state::latch_w));
	map(0x105002, 0x105003).portr("BUTTONS");
	map(0x106001, 0x106001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x107000, 0x107007).noprw();
	map(0x3e0000, 0x3e07ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x3effc0, 0x3effff).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x3f4000, 0x3f5eff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_msb_w)).share("vad:playfield");
	map(0x3f5f00, 0x3f5f7f).ram().share("vad:eof");
	map(0x3f5f80, 0x3f5fff).share("vad:mob:slip");
	map(0x3f6000, 0x3f7fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x3f8000, 0x3fcfff).ram();
	map(0x3fd000, 0x3fd3ff).ram().share("vad:mob");
	map(0x3fd400, 0x3fffff).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( shuuz )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x07fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x07fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( shuuz2 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Step Debug SW") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0600, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Playfield Debug SW") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Debug SW") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Crosshair Debug SW") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Freeze Debug SW") PORT_CODE(KEYCODE_F)

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Replay Debug SW") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0600, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, 0+RGN_FRAC(1,2), 4+RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static GFXDECODE_START( gfx_shuuz )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  256, 16 )      // sprites & playfield
	GFXDECODE_ENTRY( "gfx2", 0, pfmolayout,    0, 16 )      // sprites & playfield
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void shuuz_state::shuuz(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &shuuz_state::main_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, "palette", gfx_shuuz);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 1024);

	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	TILEMAP(config, "vad:playfield", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(shuuz_state::get_playfield_tile_info));
	ATARI_MOTION_OBJECTS(config, "vad:mob", 0, m_screen, shuuz_state::s_mob_config).set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived
	   the board uses a VAD chip to generate video signals */
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(shuuz_state::screen_update));
	m_screen->set_palette("palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 14.318181_MHz_XTAL / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( shuuz )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136083-4010.23p",     0x00000, 0x20000, CRC(1c2459f8) SHA1(4b8daf196e3ba17cf958a3c1af4e4dacfb79b9e7) )
	ROM_LOAD16_BYTE( "136083-4011.13p",     0x00001, 0x20000, CRC(6db53a85) SHA1(7f9b3ea78fa65221931bfdab1aa5f1913ffed753) )

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136083-2030.43x", 0x000000, 0x20000, CRC(8ecf1ed8) SHA1(47143f1eaf43027c5301eb6009d8a56a98328894) )
	ROM_LOAD( "136083-2032.20x", 0x020000, 0x20000, CRC(5af184e6) SHA1(630969466c606d1f51da81911fb365a4cac4685c) )
	ROM_LOAD( "136083-2031.87x", 0x040000, 0x20000, CRC(72e9db63) SHA1(be13830b38c2603bbd6b875abdc1675788a60b24) )
	ROM_LOAD( "136083-2033.65x", 0x060000, 0x20000, CRC(8f552498) SHA1(7fd323f3b30747a8645d7a9676fdf8f973b6632a) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136083-1020.43u", 0x000000, 0x20000, CRC(d21ad039) SHA1(5389745eff6690c1890f98a9630869b1084fb2f3) )
	ROM_LOAD( "136083-1022.20u", 0x020000, 0x20000, CRC(0c10bc90) SHA1(11272757ecad42a4fae49046bd1b01d5ff7f7d4f) )
	ROM_LOAD( "136083-1024.43m", 0x040000, 0x20000, CRC(adb09347) SHA1(5294dfb3d4aa83525795ca03c2f328ab9a666baf) )
	ROM_LOAD( "136083-1026.20m", 0x060000, 0x20000, CRC(9b20e13d) SHA1(726b6fb548c0906a5baa90b9698f99a6af9ecc36) )
	ROM_LOAD( "136083-1021.87u", 0x080000, 0x20000, CRC(8388910c) SHA1(62c6b1885bed042ef72fb62464923a33f9b464f1) )
	ROM_LOAD( "136083-1023.65u", 0x0a0000, 0x20000, CRC(71353112) SHA1(0aab14379e1b562b81cdd52eb209e264a12232c4) )
	ROM_LOAD( "136083-1025.87m", 0x0c0000, 0x20000, CRC(f7b20a64) SHA1(667c539fa809d3ae4a1c127e2044dd3a4e533266) )
	ROM_LOAD( "136083-1027.65m", 0x0e0000, 0x20000, CRC(55d54952) SHA1(73e1a388ea48bab567bde8958ee228432ebfbf67) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "136083-1040.75b", 0x00000, 0x20000, CRC(0896702b) SHA1(d826bb4812d393889584c7c656c317fd5745a05f) )
	ROM_LOAD( "136083-1041.65b", 0x20000, 0x20000, CRC(b3b07ce9) SHA1(f1128a143b72867c16b9803b0beb0188420cbfb5) )

	ROM_REGION( 0x0005, "pals", 0 )
	ROM_LOAD( "136083-1050.55c", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1051.45e", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1052.32l", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1053.85n", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1054.97n", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
ROM_END


ROM_START( shuuz2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136083-23p.rom",     0x00000, 0x20000, CRC(98aec4e7) SHA1(8cbe6e7835ecf0ef74a2de723ef970a63d3bddd1) )
	ROM_LOAD16_BYTE( "136083-13p.rom",     0x00001, 0x20000, CRC(dd9d5d5c) SHA1(0bde6be55532c232b1d27824c2ce61f33501cbb0) )

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136083-2030.43x", 0x000000, 0x20000, CRC(8ecf1ed8) SHA1(47143f1eaf43027c5301eb6009d8a56a98328894) )
	ROM_LOAD( "136083-2032.20x", 0x020000, 0x20000, CRC(5af184e6) SHA1(630969466c606d1f51da81911fb365a4cac4685c) )
	ROM_LOAD( "136083-2031.87x", 0x040000, 0x20000, CRC(72e9db63) SHA1(be13830b38c2603bbd6b875abdc1675788a60b24) )
	ROM_LOAD( "136083-2033.65x", 0x060000, 0x20000, CRC(8f552498) SHA1(7fd323f3b30747a8645d7a9676fdf8f973b6632a) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136083-1020.43u", 0x000000, 0x20000, CRC(d21ad039) SHA1(5389745eff6690c1890f98a9630869b1084fb2f3) )
	ROM_LOAD( "136083-1022.20u", 0x020000, 0x20000, CRC(0c10bc90) SHA1(11272757ecad42a4fae49046bd1b01d5ff7f7d4f) )
	ROM_LOAD( "136083-1024.43m", 0x040000, 0x20000, CRC(adb09347) SHA1(5294dfb3d4aa83525795ca03c2f328ab9a666baf) )
	ROM_LOAD( "136083-1026.20m", 0x060000, 0x20000, CRC(9b20e13d) SHA1(726b6fb548c0906a5baa90b9698f99a6af9ecc36) )
	ROM_LOAD( "136083-1021.87u", 0x080000, 0x20000, CRC(8388910c) SHA1(62c6b1885bed042ef72fb62464923a33f9b464f1) )
	ROM_LOAD( "136083-1023.65u", 0x0a0000, 0x20000, CRC(71353112) SHA1(0aab14379e1b562b81cdd52eb209e264a12232c4) )
	ROM_LOAD( "136083-1025.87m", 0x0c0000, 0x20000, CRC(f7b20a64) SHA1(667c539fa809d3ae4a1c127e2044dd3a4e533266) )
	ROM_LOAD( "136083-1027.65m", 0x0e0000, 0x20000, CRC(55d54952) SHA1(73e1a388ea48bab567bde8958ee228432ebfbf67) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "136083-1040.75b", 0x00000, 0x20000, CRC(0896702b) SHA1(d826bb4812d393889584c7c656c317fd5745a05f) )
	ROM_LOAD( "136083-1041.65b", 0x20000, 0x20000, CRC(b3b07ce9) SHA1(f1128a143b72867c16b9803b0beb0188420cbfb5) )

	ROM_REGION( 0x0005, "pals", 0 )
	ROM_LOAD( "136083-1050.55c", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1051.45e", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1052.32l", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1053.85n", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
	ROM_LOAD( "136083-1054.97n", 0x0000, 0x0001, NO_DUMP ) // GAL16V8A-25LP
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, shuuz,  0,     shuuz, shuuz,  shuuz_state, empty_init, ROT0, "Atari Games", "Shuuz (version 8.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, shuuz2, shuuz, shuuz, shuuz2, shuuz_state, empty_init, ROT0, "Atari Games", "Shuuz (version 7.1)", MACHINE_SUPPORTS_SAVE )
