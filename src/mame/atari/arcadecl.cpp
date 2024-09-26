// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Atari Arcade Classics hardware (prototypes)

    driver by Aaron Giles

    Games supported:
        * Arcade Classics (1992)
        * Sparkz (1992)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-0FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    200000-21FFFF   R/W   xxxxxxxx xxxxxxxx   Playfield RAM (512x256 pixels)
                    R/W   xxxxxxxx --------      (Left pixel)
                    R/W   -------- xxxxxxxx      (Right pixel)
    3C0000-3C01FF   R/W   xxxxxxxx xxxxxxxx   Playfield palette RAM (256 entries)
                    R/W   x------- --------      (RGB 1 LSB)
                    R/W   -xxxxx-- --------      (Red 5 MSB)
                    R/W   ------xx xxx-----      (Green 5 MSB)
                    R/W   -------- ---xxxxx      (Blue 5 MSB)
    3C0200-3C03FF   R/W   xxxxxxxx xxxxxxxx   Motion object palette RAM (256 entries)
    3C0400-3C07FF   R/W   xxxxxxxx xxxxxxxx   Extra palette RAM (512 entries)
    3E0000-3E07FF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (256 entries x 4 words)
                    R/W   -------- xxxxxxxx      (0: Link to next object)
                    R/W   x------- --------      (1: Horizontal flip)
                    R/W   -xxxxxxx xxxxxxxx      (1: Tile index)
                    R/W   xxxxxxxx x-------      (2: X position)
                    R/W   -------- ----xxxx      (2: Palette select)
                    R/W   xxxxxxxx x-------      (3: Y position)
                    R/W   -------- -xxx----      (3: Number of X tiles - 1)
                    R/W   -------- -----xxx      (3: Number of Y tiles - 1)
    3E0800-3EFFFF   R/W   xxxxxxxx xxxxxxxx   Extra sprite RAM
    640000          R     xxxxxxxx --------   Input port 1
    640002          R     xxxxxxxx --------   Input port 2
    640010          R     -------- xx------   Status port
                    R     -------- x-------      (VBLANK)
                    R     -------- -x------      (Self test)
    640012          R     -------- --xx--xx   Coin inputs
                    R     -------- --xx----      (Service coins)
                    R     -------- ------xx      (Coin switches)
    640020-640027   R     -------- xxxxxxxx   Analog inputs
    640040            W   -------- x--xxxxx   Sound control
                      W   -------- x-------      (ADPCM bank select)
                      W   -------- ---xxxxx      (Volume)
    640060            W   -------- --------   EEPROM enable
    641000-6413FF   R/W   -------- xxxxxxxx   EEPROM
    642000-642001   R/W   xxxxxxxx --------   MSM6295 communications
    646000            W   -------- --------   32V IRQ acknowledge
    647000            W   -------- --------   Watchdog reset
    ========================================================================
    Interrupts:
        IRQ4 = 32V
    ========================================================================

****************************************************************************/


#include "emu.h"

#include "atarimo.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sparkz_state : public driver_device
{
public:
	sparkz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_oki(*this, "oki")
		, m_bitmap(*this, "bitmap")
	{ }

	void sparkz(machine_config &config);

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<okim6295_device> m_oki;
	required_shared_ptr<uint16_t> m_bitmap;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data);
	void latch_w(uint8_t data);
	void main_map(address_map &map) ATTR_COLD;
};


class arcadecl_state : public sparkz_state
{
public:
	arcadecl_state(const machine_config &mconfig, device_type type, const char *tag)
		: sparkz_state(mconfig, type, tag)
		, m_mob(*this, "mob")
	{ }

	void arcadecl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	required_device<atari_motion_objects_device> m_mob;

	static const atari_motion_objects_config s_mob_config;
};


/***************************************************************************

    Note: this video hardware has some similarities to Shuuz & company
    The sprite offset registers are stored to 3EFF80

****************************************************************************/


/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config arcadecl_state::s_mob_config =
{
	0,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	0,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0x100,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0x00ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }}, // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles*/
	{{ 0,0,0,0x0007 }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0 }},            // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};

void arcadecl_state::video_start()
{
	m_mob->set_scroll(-12, 0x110);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t arcadecl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	sparkz_state::screen_update(screen, bitmap, cliprect);

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
					// not yet verified
					pf[x] = mo[x];
				}
		}

	return 0;
}



/*************************************
 *
 *  Bitmap rendering
 *
 *************************************/

uint32_t sparkz_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update any dirty scanlines
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const uint16_t *const src = &m_bitmap[256 * y];
		uint16_t *const dst = &bitmap.pix(y);

		// regenerate the line
		for (int x = cliprect.left() & ~1; x <= cliprect.right(); x += 2)
		{
			int const bits = src[(x - 8) / 2];
			dst[x + 0] = bits >> 8;
			dst[x + 1] = bits & 0xff;
		}
	}

	return 0;
}


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(sparkz_state::scanline_interrupt)
{
	// generate 32V signals
	if ((param & 32) == 0)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void sparkz_state::scanline_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}


/*************************************
 *
 *  Latch write
 *
 *************************************/

void sparkz_state::latch_w(uint8_t data)
{
	/* bit layout in this register:

	    0x0080 == ADPCM bank
	    0x001F == volume
	*/

	m_oki->set_rom_bank((data >> 7) & 1);
	m_oki->set_output_gain(ALL_OUTPUTS, (data & 0x001f) / 31.0f);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void sparkz_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x21ffff).ram().share(m_bitmap);
	map(0x3c0000, 0x3c07ff).rw("palette", FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0xff00).share("palette");
	map(0x3e0000, 0x3e07ff).ram().share("mob");
	map(0x3e0800, 0x3effbf).ram();
	map(0x3effc0, 0x3effff).ram().share("mob:slip");
	map(0x640000, 0x640001).portr("PLAYER1");
	map(0x640002, 0x640003).portr("PLAYER2");
	map(0x640010, 0x640011).portr("STATUS");
	map(0x640012, 0x640013).portr("COIN");
	map(0x640020, 0x640021).portr("TRACKX2");
	map(0x640022, 0x640023).portr("TRACKY2");
	map(0x640024, 0x640025).portr("TRACKX1");
	map(0x640026, 0x640027).portr("TRACKY1");
	map(0x640041, 0x640041).mirror(0xe).w(FUNC(sparkz_state::latch_w));
	map(0x640060, 0x64006f).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x641000, 0x6413ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x642000, 0x642000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x646000, 0x646001).mirror(0xffe).w(FUNC(sparkz_state::scanline_int_ack_w));
	map(0x647000, 0x647fff).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( arcadecl )
	PORT_START("PLAYER1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLAYER2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STATUS")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT(  0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX2")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY2")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX1")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY1")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( sparkz )
	PORT_START("PLAYER1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("PLAYER2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("STATUS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )         // not in "test mode"
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_arcadecl )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb, 256, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void sparkz_state::sparkz(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = 14.318181_MHz_XTAL;

	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &sparkz_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(sparkz_state::scanline_interrupt), m_screen, 0, 32);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, "palette", gfx_arcadecl);
	palette_device &palette(PALETTE(config, "palette"));
	palette.set_format(palette_device::IRGB_1555, 512);
	palette.set_membits(8);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses an SOS-2 chip to generate video signals
	m_screen->set_raw(MASTER_CLOCK / 2, 456, 0+12, 336+12, 262, 0, 240);
	m_screen->set_screen_update(FUNC(sparkz_state::screen_update));
	m_screen->set_palette("palette");
	//m_screen->screen_vblank().set(FUNC(sparkz_state::video_int_write_line));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, MASTER_CLOCK / 4 / 3, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void arcadecl_state::arcadecl(machine_config &config)
{
	sparkz(config);

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, arcadecl_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( arcadecl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pgm0",  0x00000, 0x80000, CRC(b5b93623) SHA1(a2e96c0c6eceb3d8f205e28d6b8197055aeb8cc4) )
	ROM_LOAD16_BYTE( "prog1", 0x00001, 0x80000, CRC(e7efef85) SHA1(05f2119d8ecc27f6efea85f5174ea7da404d7e9b) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "atcl_mob",   0x00000, 0x80000, CRC(0e9b3930) SHA1(51a449b79235d6ca5189e671399acff72a2d3dc8) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "adpcm",      0x00000, 0x80000, CRC(03ca7f03) SHA1(87ff53599b6f0cdfa5a1779773e09cc5cfe3c2a8) )
ROM_END


ROM_START( sparkz )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sparkzpg.0", 0x00000, 0x80000, CRC(a75c331c) SHA1(855ed44bd23c1dd0ca64926cacc8be62aca82fe2) )
	ROM_LOAD16_BYTE( "sparkzpg.1", 0x00001, 0x80000, CRC(1af1fc04) SHA1(6d92edb1a881ba6b63e0144c9c3e631b654bf8ae) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 ) // Unknown size, Unpopulated

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sparkzsn",      0x00000, 0x80000, CRC(87097ce2) SHA1(dc4d199b5af692d111c087af3edc01e2ac0287a8) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, arcadecl, 0, arcadecl, arcadecl, arcadecl_state, empty_init, ROT0, "Atari Games", "Arcade Classics (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sparkz,   0, sparkz,   sparkz,   sparkz_state,   empty_init, ROT0, "Atari Games", "Sparkz (prototype)",          MACHINE_SUPPORTS_SAVE )
