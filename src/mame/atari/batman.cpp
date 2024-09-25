// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Batman hardware

    driver by Aaron Giles

    Games supported:
        * Batman (1991)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"
#include "atarivad.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class batman_state : public driver_device
{
public:
	batman_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_mob(*this, "vad:mob")
	{ }

	void init_batman();
	void batman(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t m_latch_data = 0U;
	uint8_t m_alpha_tile_bank = 0U;

	static const atari_motion_objects_config s_mob_config;

	void latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(batman_state::get_alpha_tile_info)
{
	uint16_t const data = m_vad->alpha().basemem_read(tile_index);
	int const code = ((data & 0x400) ? (m_alpha_tile_bank * 0x400) : 0) + (data & 0x3ff);
	int const color = (data >> 11) & 0x0f;
	int const opaque = data & 0x8000;
	tileinfo.set(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield().extmem_read(tile_index) & 0xff;
	int const code = data1 & 0x7fff;
	int const color = 0x10 + (data2 & 0x0f);
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield2_tile_info)
{
	uint16_t const data1 = m_vad->playfield2().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield2().extmem_read(tile_index) >> 8;
	int const code = data1 & 0x7fff;
	int const color = data2 & 0x0f;
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config batman_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	1,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
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



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t batman_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob().draw_async(cliprect);

	// draw the playfield
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield().draw(screen, bitmap, cliprect, 0, 0x00);
	m_vad->playfield().draw(screen, bitmap, cliprect, 1, 0x01);
	m_vad->playfield().draw(screen, bitmap, cliprect, 2, 0x02);
	m_vad->playfield().draw(screen, bitmap, cliprect, 3, 0x03);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 0, 0x80);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 1, 0x84);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 2, 0x88);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 3, 0x8c);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			uint8_t const *const pri = &priority_bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* verified on real hardware:

					    for all MO colors, MO priority 0:
					        obscured by low fg playfield pens priority 1-3
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only

					    for all MO colors, MO priority 1:
					        obscured by low fg playfield pens priority 2-3
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only

					    for all MO colors, MO priority 2-3:
					        obscured by low fg playfield pens priority 3 only
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only
					*/
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
						continue;

					// foreground playfield case
					if (pri[x] & 0x80)
					{
						int const pfpriority = (pri[x] >> 2) & 3;

						// playfield priority 3 always wins
						if (pfpriority == 3)
							;

						// priority is consistent for upper pens in playfield
						else if (pf[x] & 0x08)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

						// otherwise, we need to compare
						else if (mopriority >= pfpriority)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					// background playfield case
					else
					{
						int const pfpriority = pri[x] & 3;

						// playfield priority 3 always wins
						if (pfpriority == 3)
							;

						// otherwise, MOs get shown
						else
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					// don't erase yet -- we need to make another pass later
				}
		}

	// add the alpha on top
	m_vad->alpha().draw(screen, bitmap, cliprect, 0, 0);

	// now go back and process the upper bit of MO priority
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
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
							m_vad->mob().apply_stain(bitmap, pf, mo, x, y);
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

void batman_state::machine_start()
{
	save_item(NAME(m_latch_data));
	save_item(NAME(m_alpha_tile_bank));
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

void batman_state::latch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const oldword = m_latch_data;
	COMBINE_DATA(&m_latch_data);

	// bit 4 is connected to the /RESET pin on the 6502
	if (m_latch_data & 0x0010)
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	/* alpha bank is selected by the upper 4 bits */
	if ((oldword ^ m_latch_data) & 0x7000)
	{
		m_screen->update_partial(m_screen->vpos());
		m_vad->alpha().mark_all_dirty();
		m_alpha_tile_bank = (m_latch_data >> 12) & 7;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full map verified from schematics and GALs
// addresses in the 1xxxxx region map to /WAIT
// addresses in the 2xxxxx region map to /WAIT2
void batman_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fffff);
	map(0x000000, 0x0bffff).rom();
	map(0x100000, 0x10ffff).mirror(0x010000).ram();
	map(0x120000, 0x120fff).mirror(0x01f000).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x260000, 0x260001).mirror(0x11ff8c).portr("260000");
	map(0x260002, 0x260003).mirror(0x11ff8c).portr("260002");
	map(0x260010, 0x260011).mirror(0x11ff8e).portr("260010");
	map(0x260031, 0x260031).mirror(0x11ff8e).r(m_jsa, FUNC(atari_jsa_iii_device::main_response_r));
	map(0x260041, 0x260041).mirror(0x11ff8e).w(m_jsa, FUNC(atari_jsa_iii_device::main_command_w));
	map(0x260050, 0x260051).mirror(0x11ff8e).w(FUNC(batman_state::latch_w));
	map(0x260060, 0x260061).mirror(0x11ff8e).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x2a0000, 0x2a0001).mirror(0x11fffe).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x2e0000, 0x2e0fff).mirror(0x100000).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x2effc0, 0x2effff).mirror(0x100000).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x2f0000, 0x2f1fff).mirror(0x100000).ram().w(m_vad, FUNC(atari_vad_device::playfield2_latched_msb_w)).share("vad:playfield2");
	map(0x2f2000, 0x2f3fff).mirror(0x100000).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_lsb_w)).share("vad:playfield");
	map(0x2f4000, 0x2f5fff).mirror(0x100000).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x2f6000, 0x2f7fff).mirror(0x100000).ram().share("vad:mob");
	map(0x2f8000, 0x2f8eff).mirror(0x100000).ram().w(m_vad, FUNC(atari_vad_device::alpha_w)).share("vad:alpha");
	map(0x2f8f00, 0x2f8f7f).mirror(0x100000).ram().share("vad:eof");
	map(0x2f8f80, 0x2f8fff).mirror(0x100000).ram().share("vad:mob:slip");
	map(0x2f9000, 0x2fffff).mirror(0x100000).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( batman )
	PORT_START("260000")
	PORT_BIT( 0x01ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260002")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")   // Input buffer full (@260030)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")   // Output buffer full (@260040)
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
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


static GFXDECODE_START( gfx_batman )
	GFXDECODE_ENTRY( "spr_tiles_2", 0, pfmolayout,  512, 32 )      // sprites & playfield
	GFXDECODE_ENTRY( "spr_tiles_1", 0, pfmolayout,  256, 16 )      // sprites & playfield
	GFXDECODE_ENTRY( "chars",       0, anlayout,      0, 64 )      // characters 8x8
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void batman_state::batman(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &batman_state::main_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_batman);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_vad->set_xoffsets(2, 6);

	TILEMAP(config, "vad:playfield", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(batman_state::get_playfield_tile_info));
	TILEMAP(config, "vad:playfield2", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64, 0).set_info_callback(FUNC(batman_state::get_playfield2_tile_info));
	TILEMAP(config, "vad:alpha", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(batman_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, batman_state::s_mob_config).set_gfxdecode("gfxdecode");
	m_mob->set_xoffset(-1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a VAD chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(batman_state::screen_update));
	m_screen->set_palette("palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_III(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(6);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( batman )
	ROM_REGION( 0xc0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136085-2030.10r",  0x00000, 0x20000, CRC(7cf4e5bf) SHA1(d6068a65fb524d839a34e596a272fac1ce90981c) )
	ROM_LOAD16_BYTE( "136085-2031.7r",   0x00001, 0x20000, CRC(7d7f3fc4) SHA1(8ee3e9ad3464006a26c36155b6099433110e2a6e) )
	ROM_LOAD16_BYTE( "136085-2032.91r",  0x40000, 0x20000, CRC(d80afb20) SHA1(5696627f6fa713dba4d12443c945f3e1cb6452a3) )
	ROM_LOAD16_BYTE( "136085-2033.6r",   0x40001, 0x20000, CRC(97efa2b8) SHA1(782e263ca22356c1747c50aed158d8c459364ad8) )
	ROM_LOAD16_BYTE( "136085-2034.9r",   0x80000, 0x20000, CRC(05388c62) SHA1(de037203d94e72e2922c89256da080ae023ca0e7) )
	ROM_LOAD16_BYTE( "136085-2035.5r",   0x80001, 0x20000, CRC(e77c92dd) SHA1(6d475092f7628114960d26b8ec1c5eae5e61ce25) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136085-1040.12c",  0x00000, 0x10000, CRC(080db83c) SHA1(ec084b7c1dc5878acd6d081e2e8b8d1e8b3d8a45) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "136085-2009.10m",  0x00000, 0x20000, CRC(a82d4923) SHA1(38e03eebd95347a383f3d7357462252961bd3c7f) )

	ROM_REGION( 0x100000, "spr_tiles_1", ROMREGION_INVERT )
	ROM_LOAD( "136085-1010.13r",  0x000000, 0x20000, CRC(466e1365) SHA1(318530e8a97c1b6ee3e671e677fc7684df5cc3a8) ) // graphics, plane 0
	ROM_LOAD( "136085-1014.14r",  0x020000, 0x20000, CRC(ef53475a) SHA1(9bfc66bb8dd02757e4a79a75928b260f4518a94b) )
	ROM_LOAD( "136085-1011.13m",  0x040000, 0x20000, CRC(8cda5efc) SHA1(b0410f9bf1f38f5f1e9add15079b03d7f19b4c8f) ) // graphics, plane 1
	ROM_LOAD( "136085-1015.14m",  0x060000, 0x20000, CRC(043e7f8b) SHA1(a3b2c539c1fa9b1e8d3dc8163b9a7c6e22408122) )
	ROM_LOAD( "136085-1012.13f",  0x080000, 0x20000, CRC(b017f2c3) SHA1(12846f0ae33e808dfb0795ea4138115b0eb36c4e) ) // graphics, plane 2
	ROM_LOAD( "136085-1016.14f",  0x0a0000, 0x20000, CRC(70aa2360) SHA1(5b944b533be40b859b7fae64559286034409ac6c) )
	ROM_LOAD( "136085-1013.13c",  0x0c0000, 0x20000, CRC(68b64975) SHA1(f3fb45dd74fc21dd2eece87e739c734963962f93) ) // graphics, plane 3
	ROM_LOAD( "136085-1017.14c",  0x0e0000, 0x20000, CRC(e4af157b) SHA1(ddf9eff84c882a096f7e435a6227b32d31029f9e) )

	ROM_REGION( 0x100000, "spr_tiles_2", ROMREGION_INVERT )
	ROM_LOAD( "136085-1018.15r",  0x000000, 0x20000, CRC(4c14f1e5) SHA1(2a65d0aafd944886d9e801c9de0f857f2e9db773) )
	ROM_LOAD( "136085-1022.16r",  0x020000, 0x20000, CRC(7476a15d) SHA1(779f9aec114aa71a268a7a646d998c1593f55d08) )
	ROM_LOAD( "136085-1019.15m",  0x040000, 0x20000, CRC(2046d9ec) SHA1(3b14b18545eac2e6cb1d3157ec1af251287b3e45) )
	ROM_LOAD( "136085-1023.16m",  0x060000, 0x20000, CRC(75cac686) SHA1(f3b1f485e51cc4af5809ec3fa2e7353fe9acb18f) )
	ROM_LOAD( "136085-1020.15f",  0x080000, 0x20000, CRC(cc4f4b94) SHA1(b8a11dbe436496898c8b6d64163a3a92eb843086) )
	ROM_LOAD( "136085-1024.16f",  0x0a0000, 0x20000, CRC(d60d35e0) SHA1(d74752090aec9fe0b7a67f62ae3024da74a004e3) )
	ROM_LOAD( "136085-1021.15c",  0x0c0000, 0x20000, CRC(9c8ef9ba) SHA1(c2540adfc227a654a3f91e2cfdcd98b3a04ae4fb) )
	ROM_LOAD( "136085-1025.16c",  0x0e0000, 0x20000, CRC(5d30bcd1) SHA1(817e225511ab98e7575ee512d659c51fcb7716dc) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )   // ADPCM
	ROM_LOAD( "136085-1041.19e",  0x00000, 0x20000, CRC(d97d5dbb) SHA1(7609841c773e3d1ae5a21da81e3387260fd8da41) )
	ROM_LOAD( "136085-1042.17e",  0x20000, 0x20000, CRC(8c496986) SHA1(07c84c68885e2ab3e81ee92942d6a0f29e4dffa8) )
	ROM_LOAD( "136085-1043.15e",  0x40000, 0x20000, CRC(51812d3b) SHA1(6748fecef753179a9257c0da5a7b7c9648437208) )
	ROM_LOAD( "136085-1044.12e",  0x60000, 0x20000, CRC(5e2d7f31) SHA1(737c7204d91f5dd5c9ed0321fc6c0d6194a18f8a) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "batman-eeprom.bin", 0x0000, 0x800, CRC(c859b535) SHA1(b7f37aab1e869e92fbcc69af98a9c14f7cf2b418) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "gal16v8a-136085-1001.m9",  0x0000, 0x0117, CRC(45dfc0cf) SHA1(39cbb27e504e09d97caea144bfdec2247a39caf9) )
	ROM_LOAD( "gal16v8a-136085-1002.l9",  0x0200, 0x0117, CRC(35c221ae) SHA1(6f3241fcd8b7e241036a0f553f118d8aec413732) )
	ROM_LOAD( "gal20v8a-136085-1003.c9",  0x0400, 0x0157, CRC(cbfb2b4f) SHA1(1566b2cf0a3e1cc982e0d9262ad7be3f3e452787) )
	ROM_LOAD( "gal20v8a-136085-1004.b9",  0x0600, 0x0157, CRC(227cd23a) SHA1(5592c08b0635f7aef233b7836cfe6b419d6123f2) )
	ROM_LOAD( "gal16v8a-136085-1005.d11", 0x0800, 0x0117, CRC(a2fe4402) SHA1(ef93eee062d51885504dcc58c34cad656e5b1ec4) )
	ROM_LOAD( "gal16v8a-136085-1006.d9",  0x0a00, 0x0117, CRC(87d1c2dc) SHA1(bfb0c45b63cd100a757d777a43202d4555036ef4) )
	ROM_LOAD( "gal16v8a-136085-1038.c17", 0x0c00, 0x0117, CRC(0a591138) SHA1(12137f30428fcb952bb6feb37a54451ad683d3b0) )
	ROM_LOAD( "gal16v8a-136085-1039.c19", 0x0e00, 0x0117, CRC(47565e09) SHA1(7aeac8de6f9fda23da50655352b845f692ab0747) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, batman, 0, batman, batman, batman_state, empty_init, ROT0, "Atari Games", "Batman", MACHINE_SUPPORTS_SAVE )
