// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari ThunderJaws hardware

    driver by Aaron Giles

    Games supported:
        * ThunderJaws (1990)

    Known bugs:
        * none at this time

****************************************************************************

    Video controller interface woes:

    Sigh. CPU #1 reads the video controller register twice per frame, once at
    the beginning of interrupt and once near the end. It stores these values in a
    table starting at $163484. CPU #2 periodically looks at this table to make
    sure that it is getting interrupts at the appropriate times, and that the
    VBLANK bit is set appropriately. Unfortunately, due to all the calls
    we make to synchronize the two CPUs, we occasionally get out of time
    and generate the interrupt outside of the tight tolerances CPU #2 expects.

    So we fake it. Returning scanlines $f5 and $f7 alternately provides the
    correct answer that causes CPU #2 to be happy and not aggressively trash
    memory (which is what it does if this interrupt test fails -- see the code
    at $1E56 to see!)

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

class thunderj_state : public driver_device
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_maincpu(*this, "maincpu"),
		m_extra(*this, "extra"),
		m_260012(*this, "260012")
	{ }

	void thunderj(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<screen_device> m_screen;
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_extra;

	required_ioport m_260012;

	uint8_t m_alpha_tile_bank = 0;

	static const atari_motion_objects_config s_mob_config;

	void scanline_int_write_line(int state);
	uint16_t special_port2_r();
	void latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void extra_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(thunderj_state::get_alpha_tile_info)
{
	uint16_t const data = m_vad->alpha().basemem_read(tile_index);
	int const code = ((data & 0x200) ? (m_alpha_tile_bank * 0x200) : 0) + (data & 0x1ff);
	int const color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int const opaque = data & 0x8000;
	tileinfo.set(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(thunderj_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield().extmem_read(tile_index) & 0xff;
	int const code = data1 & 0x7fff;
	int const color = 0x10 + (data2 & 0x0f);
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(thunderj_state::get_playfield2_tile_info)
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

const atari_motion_objects_config thunderj_state::s_mob_config =
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

uint32_t thunderj_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
					/* verified from the GALs on the real PCB; equations follow
					 *
					 *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
					 *      PF/M=MPX0*!MPX1*!MPX2*!MPX3*!MPX4*!MPX5*!MPX6*!MPX7
					 *          +PFX3*PFX8*PFX9*!MPR0
					 *          +PFX3*PFX8*!MPR0*!MPR1
					 *          +PFX3*PFX9*!MPR1
					 *
					 *      --- CS1 is 1 if the playfield should be displayed
					 *      CS1=PF/M*!ALBG*!APIX0*!APIX1
					 *         +!MPX0*!MPX1*!MPX2*!MPX3*!ALBG*!APIX0*!APIX1
					 *
					 *      --- CS0 is 1 if the MOs should be displayed
					 *      CS0=!PF/M*MPX0*!ALBG*!APIX0*!APIX1
					 *         +!PF/M*MPX1*!ALBG*!APIX0*!APIX1
					 *         +!PF/M*MPX2*!ALBG*!APIX0*!APIX1
					 *         +!PF/M*MPX3*!ALBG*!APIX0*!APIX1
					 *
					 *      --- CRA10 is the 0x200 bit of the color RAM index; set if pf is displayed
					 *      CRA10:=CS1
					 *
					 *      --- CRA9 is the 0x100 bit of the color RAM index; set if mo is displayed
					 *          or if the playfield selected is playfield #2
					 *      CRA9:=PFXS*CS1
					 *          +!CS1*CS0
					 *
					 *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
					 *      CRA8:=CS1*PFX7
					 *          +!CS1*MPX7*CS0
					 *          +!CS1*!CS0*ALC4
					 *
					 *      CRA7:=CS1*PFX6
					 *          +!CS1*MPX6*CS0
					 *
					 *      CRA6:=CS1*PFX5
					 *          +MPX5*!CS1*CS0
					 *          +!CS1*!CS0*ALC3
					 *
					 *      CRA5:=CS1*PFX4
					 *          +MPX4*!CS1*CS0
					 *          +!CS1*ALC2*!CS0
					 *
					 *      CRA4:=CS1*PFX3
					 *          +!CS1*MPX3*CS0
					 *          +!CS1*!CS0*ALC1
					 *
					 *      CRA3:=CS1*PFX2
					 *          +MPX2*!CS1*CS0
					 *          +!CS1*!CS0*ALC0
					 *
					 *      CRA2:=CS1*PFX1
					 *          +MPX1*!CS1*CS0
					 *          +!CS1*!CS0*APIX1
					 *
					 *      CRA1:=CS1*PFX0
					 *          +MPX0*!CS1*CS0
					 *          +!CS1*!CS0*APIX0
					 */
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
						continue;

					// determine pf/m signal
					int pfm = 0;
					if ((mo[x] & 0xff) == 1)
						pfm = 1;
					else if (pf[x] & 8)
					{
						int const pfpriority = (pri[x] & 0x80) ? ((pri[x] >> 2) & 3) : (pri[x] & 3);
						if (((pfpriority == 3) && !(mopriority & 1)) ||
							((pfpriority & 1) && (mopriority == 0)) ||
							((pfpriority & 2) && !(mopriority & 2)))
							pfm = 1;
					}

					// if pfm is low, we display the mo
					if (!pfm)
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

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
 *  Initialization & interrupts
 *
 *************************************/

void thunderj_state::scanline_int_write_line(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_4, state ? ASSERT_LINE : CLEAR_LINE);
	m_extra->set_input_line(M68K_IRQ_4, state ? ASSERT_LINE : CLEAR_LINE);
}


void thunderj_state::machine_start()
{
	save_item(NAME(m_alpha_tile_bank));
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

uint16_t thunderj_state::special_port2_r()
{
	int result = m_260012->read();
	result ^= 0x0010;
	return result;
}


void thunderj_state::latch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// reset extra CPU
	if (ACCESSING_BITS_0_7)
	{
		// 0 means hold CPU 2's reset low
		if (data & 1)
			m_extra->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		else
			m_extra->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

		// bits 2-5 are the alpha bank
		if (m_alpha_tile_bank != ((data >> 2) & 7))
		{
			m_screen->update_partial(m_screen->vpos());
			m_vad->alpha().mark_all_dirty();
			m_alpha_tile_bank = (data >> 2) & 7;
		}
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void thunderj_state::main_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom();
	map(0x0e0000, 0x0e0fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x160000, 0x16ffff).ram().share("sharedram");
	map(0x1f0000, 0x1fffff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x260011).portr("260010");
	map(0x260012, 0x260013).r(FUNC(thunderj_state::special_port2_r));
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_ii_device::main_response_r));
	map(0x2e0000, 0x2e0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x360010, 0x360011).w(FUNC(thunderj_state::latch_w));
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_ii_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_ii_device::main_command_w));
	map(0x3e0000, 0x3e0fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x3effc0, 0x3effff).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x3f0000, 0x3f1fff).ram().w(m_vad, FUNC(atari_vad_device::playfield2_latched_msb_w)).share("vad:playfield2");
	map(0x3f2000, 0x3f3fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_lsb_w)).share("vad:playfield");
	map(0x3f4000, 0x3f5fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x3f6000, 0x3f7fff).ram().share("vad:mob");
	map(0x3f8000, 0x3f8eff).ram().w(m_vad, FUNC(atari_vad_device::alpha_w)).share("vad:alpha");
	map(0x3f8f00, 0x3f8f7f).ram().share("vad:eof");
	map(0x3f8f80, 0x3f8fff).ram().share("vad:mob:slip");
	map(0x3f9000, 0x3fffff).ram();
}



/*************************************
 *
 *  Extra CPU memory handlers
 *
 *************************************/

void thunderj_state::extra_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x07ffff).rom();
	map(0x160000, 0x16ffff).ram().share("sharedram");
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x260011).portr("260010");
	map(0x260012, 0x260013).r(FUNC(thunderj_state::special_port2_r));
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_ii_device::main_response_r));
	map(0x360010, 0x360011).w(FUNC(thunderj_state::latch_w));
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_ii_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_ii_device::main_command_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( thunderj )
	PORT_START("260000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260012")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")   // Input buffer full (@260030)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")  // Output buffer full (@360030)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
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


static GFXDECODE_START( gfx_thunderj )
	GFXDECODE_ENTRY( "sprites_tiles1", 0, gfx_8x8x4_planar,  512,  96 )
	GFXDECODE_ENTRY( "sprites_tiles2", 0, gfx_8x8x4_planar,  256, 112 )
	GFXDECODE_ENTRY( "chars",          0, anlayout,            0, 512 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void thunderj_state::thunderj(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderj_state::main_map);

	M68000(config, m_extra, 14.318181_MHz_XTAL / 2);
	m_extra->set_addrmap(AS_PROGRAM, &thunderj_state::extra_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// perfect synchronization due to shared RAM
	config.set_perfect_quantum(m_maincpu);

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_thunderj);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set(FUNC(thunderj_state::scanline_int_write_line));
	TILEMAP(config, "vad:playfield", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(thunderj_state::get_playfield_tile_info));
	TILEMAP(config, "vad:playfield2", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64, 0).set_info_callback(FUNC(thunderj_state::get_playfield2_tile_info));
	TILEMAP(config, "vad:alpha", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(thunderj_state::get_alpha_tile_info));
	ATARI_MOTION_OBJECTS(config, "vad:mob", 0, m_screen, thunderj_state::s_mob_config).set_gfxdecode("gfxdecode");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a VAD chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(thunderj_state::screen_update));
	m_screen->set_palette("palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_II(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260012").bit(1);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 0.8);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( thunderj )
	ROM_REGION( 0xa0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136076-3001.14e",   0x00000, 0x10000, CRC(cda7f027) SHA1(50dc7de1eea1ad0539c0be4e23fc50de2ba6ca40) ) // Rev 3 program ROM
	ROM_LOAD16_BYTE( "136076-3002.14c",   0x00001, 0x10000, CRC(752cead3) SHA1(474af2dbb0b8bf5e0030d9a0ee9879c92250c2e0) ) // Rev 3 program ROM
	ROM_LOAD16_BYTE( "136076-2003.15e",   0x20000, 0x10000, CRC(6e155469) SHA1(ba87d0a510304fd8a0f91c81580c4f09fc4d1886) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2004.15c",   0x20001, 0x10000, CRC(e9ff1e42) SHA1(416dd68ec2ae174a5b14fa6fb4fb88bc1afdda66) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2005.16e",   0x40000, 0x10000, CRC(a40242e7) SHA1(ea2311f064885912054ae2e50a60664b216a6253) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2006.16c",   0x40001, 0x10000, CRC(aa18b94c) SHA1(867fd1d5485eacb705ed4ec736ee8c2e78aa9bf4) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-1009.15h",   0x60000, 0x10000, CRC(05474ebb) SHA1(74a32dba5ffe2953c81ad9639d99ed01b31b0dba) )
	ROM_LOAD16_BYTE( "136076-1010.16h",   0x60001, 0x10000, CRC(ccff21c8) SHA1(7df8facf563cc1bb8de6cac6f2ddcc58ae0aa8b4) )
	ROM_LOAD16_BYTE( "136076-1007.17e",   0x80000, 0x10000, CRC(9c2a8aba) SHA1(10e4fc04e64bb6a5083a56f630224b5d1af241b2) )
	ROM_LOAD16_BYTE( "136076-1008.17c",   0x80001, 0x10000, CRC(22109d16) SHA1(8725696271c4a617f9f050d9d483fe4141bf1e00) )

	ROM_REGION( 0x80000, "extra", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136076-1011.17l",    0x00000, 0x10000, CRC(bbbbca45) SHA1(977e785e0272a84c8d7e28e25f45064d1b37aad1) )
	ROM_LOAD16_BYTE( "136076-1012.17n",    0x00001, 0x10000, CRC(53e5e638) SHA1(75593e5d328ede105b8db64005dd5d1c5cae11ed) )
	ROM_COPY( "maincpu", 0x60000, 0x60000, 0x20000 )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136076-2015.1b", 0x00000, 0x10000, CRC(d8feb7fb) SHA1(684ebf2f0c0df742c98e7f45f74de86a11c8d6e8) ) // Rev 2 program ROM

	ROM_REGION( 0x100000, "sprites_tiles1", ROMREGION_INVERT )
	ROM_LOAD( "136076-1021.5s",   0x000000, 0x10000, CRC(d8432766) SHA1(04e7d820974c0890fde1257b4710cf7b520d7d48) ) // graphics, plane 0 */
	ROM_LOAD( "136076-1025.5r",   0x010000, 0x10000, CRC(839feed5) SHA1(c683ef5b78f8fd63dd557a630544f1e21aebe665) )
	ROM_LOAD( "136076-1029.3p",   0x020000, 0x10000, CRC(fa887662) SHA1(5d19022e8d40be86b85d0bcc28c97207ab9ec403) )
	ROM_LOAD( "136076-1033.6p",   0x030000, 0x10000, CRC(2addda79) SHA1(5a04c718055a5637b7549598ec39ca3cc9883698) )
	ROM_LOAD( "136076-1022.9s",   0x040000, 0x10000, CRC(dcf50371) SHA1(566e71e1dcb8e0266ca870af04b11f7bbee21b18) ) // graphics, plane 1 */
	ROM_LOAD( "136076-1026.9r",   0x050000, 0x10000, CRC(216e72c8) SHA1(b6155584c8760c4dee3cf2a6320c53ea2161464b) )
	ROM_LOAD( "136076-1030.10s",  0x060000, 0x10000, CRC(dc51f606) SHA1(aa401808d915b2e6cdb17a1d58814a753648c9bb) )
	ROM_LOAD( "136076-1034.10r",  0x070000, 0x10000, CRC(f8e35516) SHA1(dcb23ed69f5a70ac842c6004039ec403bac68d72) )
	ROM_LOAD( "136076-1023.13s",  0x080000, 0x10000, CRC(b6dc3f13) SHA1(c3369b58012e02ad2fd85f1c9643ee5792f4b3de) ) // graphics, plane 2 */
	ROM_LOAD( "136076-1027.13r",  0x090000, 0x10000, CRC(621cc2ce) SHA1(15db80d61f1c624c09085ed86341f8577bfac168) )
	ROM_LOAD( "136076-1031.14s",  0x0a0000, 0x10000, CRC(4682ceb5) SHA1(609ccd20f654982e01bcc6aea89801c01afe083e) )
	ROM_LOAD( "136076-1035.14r",  0x0b0000, 0x10000, CRC(7a0e1b9e) SHA1(b9a2270ee7e3b3dcf05a47085890d87bf5b3e167) )
	ROM_LOAD( "136076-1024.17s",  0x0c0000, 0x10000, CRC(d84452b5) SHA1(29bc994e37bc08fa40326b811339e7aa3290302c) ) // graphics, plane 3 */
	ROM_LOAD( "136076-1028.17r",  0x0d0000, 0x10000, CRC(0cc20245) SHA1(ebdcb47909374508abe9d0252fd88d6274a0f729) )
	ROM_LOAD( "136076-1032.14p",  0x0e0000, 0x10000, CRC(f639161a) SHA1(cc2549f7fdd251fa44735a6cd5fdb8ffb97948be) )
	ROM_LOAD( "136076-1036.16p",  0x0f0000, 0x10000, CRC(b342443d) SHA1(fa7865f8a90c0e761e1cc5e155931d0574f2d81c) )

	ROM_REGION( 0x100000, "sprites_tiles2", ROMREGION_INVERT )
	ROM_LOAD( "136076-1037.2s",   0x000000, 0x10000, CRC(07addba6) SHA1(4a3286ee570bf4263944854bf959c8ef114cc123) )
	ROM_LOAD( "136076-1041.2r",   0x010000, 0x10000, CRC(1e9c29e4) SHA1(e4afa2c469bfa22504cba5dfd23704c5c2bb33c4) )
	ROM_LOAD( "136076-1045.34s",  0x020000, 0x10000, CRC(e7235876) SHA1(0bb03baec1de3e520dc270a3ed44bec953e08c00) )
	ROM_LOAD( "136076-1049.34r",  0x030000, 0x10000, CRC(a6eb8265) SHA1(f9b5fbe69b973327ebe5ea6e9fa782cb6db010d6) )
	ROM_LOAD( "136076-1038.6s",   0x040000, 0x10000, CRC(2ea543f9) SHA1(e5dafe023b1dc5068f367293da5774ab98ec617c) )
	ROM_LOAD( "136076-1042.6r",   0x050000, 0x10000, CRC(efabdc2b) SHA1(449fa8f229d901328aa14c1d093d10ba1c7e7cd9) )
	ROM_LOAD( "136076-1046.7s",   0x060000, 0x10000, CRC(6692151f) SHA1(294fb66f4a25ce0282b8f3b032df7a7103842540) )
	ROM_LOAD( "136076-1050.7r",   0x070000, 0x10000, CRC(ad7bb5f3) SHA1(1e7083cafd4c06397991cb873eeeb4b4c8a81d9d) )
	ROM_LOAD( "136076-1039.11s",  0x080000, 0x10000, CRC(cb563a40) SHA1(086619ae47c1c0b5fb1913b3e657216e021a3713) )
	ROM_LOAD( "136076-1043.11r",  0x090000, 0x10000, CRC(b7565eee) SHA1(610439f8e08fb88a646e76180bcd72ddfec8c06a) )
	ROM_LOAD( "136076-1047.12s",  0x0a0000, 0x10000, CRC(60877136) SHA1(ac7154b9324782c5c22e18c1d325f58c88938170) )
	ROM_LOAD( "136076-1051.12r",  0x0b0000, 0x10000, CRC(d4715ff0) SHA1(f0c2c7057d84b337ab1f44667e6048885922698e) )
	ROM_LOAD( "136076-1040.15s",  0x0c0000, 0x10000, CRC(6e910fc2) SHA1(29b811e0d8283dd00e2d79c2cbe120faa6834008) )
	ROM_LOAD( "136076-1044.15r",  0x0d0000, 0x10000, CRC(ff67a17a) SHA1(17f3572b526a14bdf3a6da5711c4d96feefc25aa) )
	ROM_LOAD( "136076-1048.16s",  0x0e0000, 0x10000, CRC(200d45b3) SHA1(72f1b3deaebc6266140fb79a97154f80f2cadf33) )
	ROM_LOAD( "136076-1052.16r",  0x0f0000, 0x10000, CRC(74711ef1) SHA1(c1429d6b54dc4352defdd6cf83f1a5734784e703) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "136076-1020.4m",   0x000000, 0x10000, CRC(65470354) SHA1(9895d26fa9e01c254a3d15e657152cac717c68a3) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   /* 256k for ADPCM */
	ROM_LOAD( "136076-1016.7k",  0x00000, 0x10000, CRC(c10bdf73) SHA1(a0371c6ddef2a95193c68879044b3338d481fc96) )
	ROM_LOAD( "136076-1017.7j",  0x10000, 0x10000, CRC(4e5e25e8) SHA1(373c946abd24ce8dd5221f1a0409af4537610d3d) )
	ROM_LOAD( "136076-1018.7e",  0x20000, 0x10000, CRC(ec81895d) SHA1(56acffb0700d3b70ca705fba9d240a82950fd320) )
	ROM_LOAD( "136076-1019.7d",  0x30000, 0x10000, CRC(a4009037) SHA1(01cd3f4cf510f4956258f39f3ddbb42628bc2b9a) )

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "gal16v8a-136076-1070.12e", 0x0000, 0x0117, CRC(0e96e51e) SHA1(bf23d0089e5e34ad579cb56ff869353545c29792) )
	ROM_LOAD( "gal16v8a-136076-1071.13j", 0x0200, 0x0117, CRC(ad7beb34) SHA1(d50fa0526fd83b6a9a79adcacef56a64ebafa448) )
	ROM_LOAD( "gal16v8a-136076-1072.5n",  0x0400, 0x0117, CRC(2827620a) SHA1(25c5dfae3d8f4b73936833031b95fafd2ba0ae5d) )
	ROM_LOAD( "gal16v8a-136076-1073.2h",  0x0600, 0x0117, CRC(a2fe4402) SHA1(ef93eee062d51885504dcc58c34cad656e5b1ec4) )
	ROM_LOAD( "gal16v8a-136076-2074.3l",  0x0800, 0x0117, CRC(0e8bc5d7) SHA1(815ecd2cbb3469501f29172a4425bee930852c11) )
	ROM_LOAD( "gal20v8a-136076-1075.2l",  0x0a00, 0x0157, CRC(e75e1e86) SHA1(26d09b614a24bd736fd17a3145f9cc4d61a2d1a9) )
	ROM_LOAD( "gal20v8a-136076-1076.1l",  0x0c00, 0x0157, CRC(f36f873a) SHA1(63e6cc6e50370e89ebe33aa8d4ff9c6e1faeb9a6) )
ROM_END

ROM_START( thunderja )
	ROM_REGION( 0xa0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136076-2001.14e",   0x00000, 0x10000, CRC(f6a71532) SHA1(b1c55968d7da9b64bde737d66aa8f0ddcdcfee27) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2002.14c",   0x00001, 0x10000, CRC(173ec10d) SHA1(e32eca9194336f3d7e289b2a187ed125ed03688c) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2003.15e",   0x20000, 0x10000, CRC(6e155469) SHA1(ba87d0a510304fd8a0f91c81580c4f09fc4d1886) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2004.15c",   0x20001, 0x10000, CRC(e9ff1e42) SHA1(416dd68ec2ae174a5b14fa6fb4fb88bc1afdda66) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2005.16e",   0x40000, 0x10000, CRC(a40242e7) SHA1(ea2311f064885912054ae2e50a60664b216a6253) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-2006.16c",   0x40001, 0x10000, CRC(aa18b94c) SHA1(867fd1d5485eacb705ed4ec736ee8c2e78aa9bf4) ) // Rev 2 program ROM
	ROM_LOAD16_BYTE( "136076-1009.15h",   0x60000, 0x10000, CRC(05474ebb) SHA1(74a32dba5ffe2953c81ad9639d99ed01b31b0dba) )
	ROM_LOAD16_BYTE( "136076-1010.16h",   0x60001, 0x10000, CRC(ccff21c8) SHA1(7df8facf563cc1bb8de6cac6f2ddcc58ae0aa8b4) )
	ROM_LOAD16_BYTE( "136076-1007.17e",   0x80000, 0x10000, CRC(9c2a8aba) SHA1(10e4fc04e64bb6a5083a56f630224b5d1af241b2) )
	ROM_LOAD16_BYTE( "136076-1008.17c",   0x80001, 0x10000, CRC(22109d16) SHA1(8725696271c4a617f9f050d9d483fe4141bf1e00) )

	ROM_REGION( 0x80000, "extra", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136076-1011.17l",    0x00000, 0x10000, CRC(bbbbca45) SHA1(977e785e0272a84c8d7e28e25f45064d1b37aad1) )
	ROM_LOAD16_BYTE( "136076-1012.17n",    0x00001, 0x10000, CRC(53e5e638) SHA1(75593e5d328ede105b8db64005dd5d1c5cae11ed) )
	ROM_COPY( "maincpu", 0x60000, 0x60000, 0x20000 )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136076-2015.1b", 0x00000, 0x10000, CRC(d8feb7fb) SHA1(684ebf2f0c0df742c98e7f45f74de86a11c8d6e8) ) // Rev 2 program ROM

	ROM_REGION( 0x100000, "sprites_tiles1", ROMREGION_INVERT )
	ROM_LOAD( "136076-1021.5s",   0x000000, 0x10000, CRC(d8432766) SHA1(04e7d820974c0890fde1257b4710cf7b520d7d48) ) // graphics, plane 0 */
	ROM_LOAD( "136076-1025.5r",   0x010000, 0x10000, CRC(839feed5) SHA1(c683ef5b78f8fd63dd557a630544f1e21aebe665) )
	ROM_LOAD( "136076-1029.3p",   0x020000, 0x10000, CRC(fa887662) SHA1(5d19022e8d40be86b85d0bcc28c97207ab9ec403) )
	ROM_LOAD( "136076-1033.6p",   0x030000, 0x10000, CRC(2addda79) SHA1(5a04c718055a5637b7549598ec39ca3cc9883698) )
	ROM_LOAD( "136076-1022.9s",   0x040000, 0x10000, CRC(dcf50371) SHA1(566e71e1dcb8e0266ca870af04b11f7bbee21b18) ) // graphics, plane 1 */
	ROM_LOAD( "136076-1026.9r",   0x050000, 0x10000, CRC(216e72c8) SHA1(b6155584c8760c4dee3cf2a6320c53ea2161464b) )
	ROM_LOAD( "136076-1030.10s",  0x060000, 0x10000, CRC(dc51f606) SHA1(aa401808d915b2e6cdb17a1d58814a753648c9bb) )
	ROM_LOAD( "136076-1034.10r",  0x070000, 0x10000, CRC(f8e35516) SHA1(dcb23ed69f5a70ac842c6004039ec403bac68d72) )
	ROM_LOAD( "136076-1023.13s",  0x080000, 0x10000, CRC(b6dc3f13) SHA1(c3369b58012e02ad2fd85f1c9643ee5792f4b3de) ) // graphics, plane 2 */
	ROM_LOAD( "136076-1027.13r",  0x090000, 0x10000, CRC(621cc2ce) SHA1(15db80d61f1c624c09085ed86341f8577bfac168) )
	ROM_LOAD( "136076-1031.14s",  0x0a0000, 0x10000, CRC(4682ceb5) SHA1(609ccd20f654982e01bcc6aea89801c01afe083e) )
	ROM_LOAD( "136076-1035.14r",  0x0b0000, 0x10000, CRC(7a0e1b9e) SHA1(b9a2270ee7e3b3dcf05a47085890d87bf5b3e167) )
	ROM_LOAD( "136076-1024.17s",  0x0c0000, 0x10000, CRC(d84452b5) SHA1(29bc994e37bc08fa40326b811339e7aa3290302c) ) // graphics, plane 3 */
	ROM_LOAD( "136076-1028.17r",  0x0d0000, 0x10000, CRC(0cc20245) SHA1(ebdcb47909374508abe9d0252fd88d6274a0f729) )
	ROM_LOAD( "136076-1032.14p",  0x0e0000, 0x10000, CRC(f639161a) SHA1(cc2549f7fdd251fa44735a6cd5fdb8ffb97948be) )
	ROM_LOAD( "136076-1036.16p",  0x0f0000, 0x10000, CRC(b342443d) SHA1(fa7865f8a90c0e761e1cc5e155931d0574f2d81c) )

	ROM_REGION( 0x100000, "sprites_tiles2", ROMREGION_INVERT )
	ROM_LOAD( "136076-1037.2s",   0x000000, 0x10000, CRC(07addba6) SHA1(4a3286ee570bf4263944854bf959c8ef114cc123) )
	ROM_LOAD( "136076-1041.2r",   0x010000, 0x10000, CRC(1e9c29e4) SHA1(e4afa2c469bfa22504cba5dfd23704c5c2bb33c4) )
	ROM_LOAD( "136076-1045.34s",  0x020000, 0x10000, CRC(e7235876) SHA1(0bb03baec1de3e520dc270a3ed44bec953e08c00) )
	ROM_LOAD( "136076-1049.34r",  0x030000, 0x10000, CRC(a6eb8265) SHA1(f9b5fbe69b973327ebe5ea6e9fa782cb6db010d6) )
	ROM_LOAD( "136076-1038.6s",   0x040000, 0x10000, CRC(2ea543f9) SHA1(e5dafe023b1dc5068f367293da5774ab98ec617c) )
	ROM_LOAD( "136076-1042.6r",   0x050000, 0x10000, CRC(efabdc2b) SHA1(449fa8f229d901328aa14c1d093d10ba1c7e7cd9) )
	ROM_LOAD( "136076-1046.7s",   0x060000, 0x10000, CRC(6692151f) SHA1(294fb66f4a25ce0282b8f3b032df7a7103842540) )
	ROM_LOAD( "136076-1050.7r",   0x070000, 0x10000, CRC(ad7bb5f3) SHA1(1e7083cafd4c06397991cb873eeeb4b4c8a81d9d) )
	ROM_LOAD( "136076-1039.11s",  0x080000, 0x10000, CRC(cb563a40) SHA1(086619ae47c1c0b5fb1913b3e657216e021a3713) )
	ROM_LOAD( "136076-1043.11r",  0x090000, 0x10000, CRC(b7565eee) SHA1(610439f8e08fb88a646e76180bcd72ddfec8c06a) )
	ROM_LOAD( "136076-1047.12s",  0x0a0000, 0x10000, CRC(60877136) SHA1(ac7154b9324782c5c22e18c1d325f58c88938170) )
	ROM_LOAD( "136076-1051.12r",  0x0b0000, 0x10000, CRC(d4715ff0) SHA1(f0c2c7057d84b337ab1f44667e6048885922698e) )
	ROM_LOAD( "136076-1040.15s",  0x0c0000, 0x10000, CRC(6e910fc2) SHA1(29b811e0d8283dd00e2d79c2cbe120faa6834008) )
	ROM_LOAD( "136076-1044.15r",  0x0d0000, 0x10000, CRC(ff67a17a) SHA1(17f3572b526a14bdf3a6da5711c4d96feefc25aa) )
	ROM_LOAD( "136076-1048.16s",  0x0e0000, 0x10000, CRC(200d45b3) SHA1(72f1b3deaebc6266140fb79a97154f80f2cadf33) )
	ROM_LOAD( "136076-1052.16r",  0x0f0000, 0x10000, CRC(74711ef1) SHA1(c1429d6b54dc4352defdd6cf83f1a5734784e703) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "136076-1020.4m",   0x000000, 0x10000, CRC(65470354) SHA1(9895d26fa9e01c254a3d15e657152cac717c68a3) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )
	ROM_LOAD( "136076-1016.7k",  0x00000, 0x10000, CRC(c10bdf73) SHA1(a0371c6ddef2a95193c68879044b3338d481fc96) )
	ROM_LOAD( "136076-1017.7j",  0x10000, 0x10000, CRC(4e5e25e8) SHA1(373c946abd24ce8dd5221f1a0409af4537610d3d) )
	ROM_LOAD( "136076-1018.7e",  0x20000, 0x10000, CRC(ec81895d) SHA1(56acffb0700d3b70ca705fba9d240a82950fd320) )
	ROM_LOAD( "136076-1019.7d",  0x30000, 0x10000, CRC(a4009037) SHA1(01cd3f4cf510f4956258f39f3ddbb42628bc2b9a) )

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "gal16v8a-136076-1070.12e", 0x0000, 0x0117, CRC(0e96e51e) SHA1(bf23d0089e5e34ad579cb56ff869353545c29792) )
	ROM_LOAD( "gal16v8a-136076-1071.13j", 0x0200, 0x0117, CRC(ad7beb34) SHA1(d50fa0526fd83b6a9a79adcacef56a64ebafa448) )
	ROM_LOAD( "gal16v8a-136076-1072.5n",  0x0400, 0x0117, CRC(2827620a) SHA1(25c5dfae3d8f4b73936833031b95fafd2ba0ae5d) )
	ROM_LOAD( "gal16v8a-136076-1073.2h",  0x0600, 0x0117, CRC(a2fe4402) SHA1(ef93eee062d51885504dcc58c34cad656e5b1ec4) )
	ROM_LOAD( "gal16v8a-136076-2074.3l",  0x0800, 0x0117, CRC(0e8bc5d7) SHA1(815ecd2cbb3469501f29172a4425bee930852c11) )
	ROM_LOAD( "gal20v8a-136076-1075.2l",  0x0a00, 0x0157, CRC(e75e1e86) SHA1(26d09b614a24bd736fd17a3145f9cc4d61a2d1a9) )
	ROM_LOAD( "gal20v8a-136076-1076.1l",  0x0c00, 0x0157, CRC(f36f873a) SHA1(63e6cc6e50370e89ebe33aa8d4ff9c6e1faeb9a6) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, thunderj,         0, thunderj, thunderj, thunderj_state, empty_init, ROT0, "Atari Games", "ThunderJaws (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, thunderja, thunderj, thunderj, thunderj, thunderj_state, empty_init, ROT0, "Atari Games", "ThunderJaws (rev 2)", MACHINE_SUPPORTS_SAVE )
