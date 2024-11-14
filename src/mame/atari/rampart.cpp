// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari Rampart hardware

    driver by Aaron Giles

    Games supported:
        * Rampart (1990) [4 sets]

    Known bugs:
        * none at this time

    Note:
        P3 buttons 1 and 2 are mapped twice. THIS IS NOT A BUG!

    bp 548,a0==6c0007 && (d0&ffff)!=0,{print d0&ffff; g} <- what's this for?

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarimo.h"
#include "slapstic.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_COLBANK     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_COLBANK)

#include "logmacro.h"

#define LOGCOLBANK(...)     LOGMASKED(LOG_COLBANK,     __VA_ARGS__)


namespace {

class rampart_state : public driver_device
{
public:
	rampart_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slapstic(*this, "slapstic"),
		m_slapstic_bank(*this, "slapstic_bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_mob(*this, "mob"),
		m_oki(*this, "oki"),
		m_ym2413(*this, "ymsnd"),
		m_bitmap(*this, "bitmap")
	{ }

	void rampart(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<atari_slapstic_device> m_slapstic;
	required_memory_bank m_slapstic_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_motion_objects_device> m_mob;
	required_device<okim6295_device> m_oki;
	required_device<ym2413_device> m_ym2413;

	required_shared_ptr<u16> m_bitmap;

	static const atari_motion_objects_config s_mob_config;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(u16 data);
	void latch_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config rampart_state::s_mob_config =
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

void rampart_state::video_start()
{
	// set the initial scroll offset
	m_mob->set_xscroll(-12);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t rampart_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	bitmap_render(bitmap, cliprect);

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
					// the PCB supports more complex priorities, but the PAL is not stuffed, so we get the default
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

void rampart_state::bitmap_render(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update any dirty scanlines
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t const *const src = &m_bitmap[256 * y];
		uint16_t *const dst = &bitmap.pix(y);

		// regenerate the line
		for (int x = cliprect.left() & ~1; x <= cliprect.right(); x += 2)
		{
			int const bits = src[(x - 8) / 2];
			dst[x + 0] = bits >> 8;
			dst[x + 1] = bits & 0xff;
		}
	}
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void rampart_state::machine_start()
{
	m_slapstic_bank->configure_entries(0, 4, memregion("maincpu")->base() + 0x40000, 0x2000);
}

/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(rampart_state::scanline_interrupt)
{
	// generate 32V signals
	if ((param & 32) == 0)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void rampart_state::scanline_int_ack_w(u16 data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}



/*************************************
 *
 *  Latch write
 *
 *************************************/

void rampart_state::latch_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* bit layout in this register:

	    0x8000 == VCR ???
	    0x2000 == LETAMODE1 (controls right trackball)
	    0x1000 == CBANK (color bank -- is it ever set to non-zero?)
	    0x0800 == LETAMODE0 (controls center and left trackballs)
	    0x0400 == LETARES (reset LETA analog control reader)
	    0x0200 == COINCTRL
	    0x0100 == COINCTRR

	    0x0020 == PMIX0 (ADPCM mixer level)
	    0x0010 == /PCMRES (ADPCM reset)
	    0x000E == YMIX2-0 (YM2413 mixer level)
	    0x0001 == /YAMRES (YM2413 reset)
	*/

	// upper byte being modified?
	if (ACCESSING_BITS_8_15)
	{
		if (data & 0x1000)
			LOGCOLBANK("Color bank set to 1!\n");
		machine().bookkeeping().coin_counter_w(0, (data >> 9) & 1);
		machine().bookkeeping().coin_counter_w(1, (data >> 8) & 1);
	}

	// lower byte being modified?
	if (ACCESSING_BITS_0_7)
	{
		m_oki->set_output_gain(ALL_OUTPUTS, (data & 0x0020) ? 1.0f : 0.0f);
		if (!(data & 0x0010))
			m_oki->reset();
		m_ym2413->set_output_gain(ALL_OUTPUTS, ((data >> 1) & 7) / 7.0f);
		if (!(data & 0x0001))
			m_ym2413->reset();
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full memory map deduced from schematics and GALs
void rampart_state::main_map(address_map &map)
{
	map.global_mask(0x7fffff);
	map(0x000000, 0x0fffff).rom();
	map(0x140000, 0x141fff).mirror(0x43e000).bankr(m_slapstic_bank);
	map(0x200000, 0x21ffff).ram().share(m_bitmap);
	map(0x220000, 0x3bffff).nopw();    // the code blasts right through this when initializing
	map(0x3c0000, 0x3c07ff).mirror(0x019800).rw("palette", FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0xff00).share("palette");
	map(0x3e0000, 0x3e07ff).mirror(0x010000).ram().share("mob");
	map(0x3e0800, 0x3e3f3f).mirror(0x010000).ram();
	map(0x3e3f40, 0x3e3f7f).mirror(0x010000).ram().share("mob:slip");
	map(0x3e3f80, 0x3effff).mirror(0x010000).ram();
	map(0x460000, 0x460000).mirror(0x019ffe).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x480000, 0x480003).mirror(0x019ffc).w(m_ym2413, FUNC(ym2413_device::write)).umask16(0xff00);
	map(0x500000, 0x500fff).mirror(0x019000).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x5a6000, 0x5a6001).mirror(0x019ffe).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x640000, 0x640001).mirror(0x019ffe).w(FUNC(rampart_state::latch_w));
	map(0x640000, 0x640001).mirror(0x019ffc).portr("IN0");
	map(0x640002, 0x640003).mirror(0x019ffc).portr("IN1");
	map(0x6c0000, 0x6c0001).mirror(0x019ff8).portr("TRACK0");
	map(0x6c0002, 0x6c0003).mirror(0x019ff8).portr("TRACK1");
	map(0x6c0004, 0x6c0005).mirror(0x019ff8).portr("TRACK2");
	map(0x6c0006, 0x6c0007).mirror(0x019ff8).portr("TRACK3");
	map(0x726000, 0x726001).mirror(0x019ffe).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x7e6000, 0x7e6001).mirror(0x019ffe).w(FUNC(rampart_state::scanline_int_ack_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( rampart )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) // alternate button1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) // alternate button2
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK0")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("TRACK1")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("TRACK2")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK3")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ramprt2p )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) // alternate button1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Players ) )
	PORT_DIPSETTING(    0x0000, "2")
	PORT_DIPSETTING(    0x0004, "3")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) // alternate button2
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("TRACK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rampartj )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("TRACK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_rampart )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb,  256, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void rampart_state::rampart(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(14'318'181);

	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rampart_state::main_map);

	SLAPSTIC(config, m_slapstic, 118);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0x140000, 0x147fff, 0x438000);
	m_slapstic->set_bank(m_slapstic_bank);

	TIMER(config, "scantimer").configure_scanline(FUNC(rampart_state::scanline_interrupt), m_screen, 0, 32);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	// video hardware
	GFXDECODE(config, m_gfxdecode, "palette", gfx_rampart);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 512).set_membits(8);

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, rampart_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived
	   the board uses an SOS-2 chip to generate video signals */
	m_screen->set_raw(MASTER_CLOCK / 2, 456, 0+12, 336+12, 262, 0, 240);
	m_screen->set_screen_update(FUNC(rampart_state::screen_update));
	m_screen->set_palette("palette");
	//m_screen->screen_vblank().set(FUNC(rampart_state::video_int_write_line));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, MASTER_CLOCK / 4 / 3, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.60);

	YM2413(config, m_ym2413, MASTER_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( rampart )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136082-1033.13l", 0x00000, 0x80000, CRC(5c36795f) SHA1(2f3dcdfd6b04d851aa1082848624687ac0cec9e2) )
	ROM_LOAD16_BYTE( "136082-1032.13j", 0x00001, 0x80000, CRC(ec7bc38c) SHA1(72d4dbb11e92c69cb560bbb39d7bbd5e845b1e4d) )
	ROM_LOAD16_BYTE( "136082-2031.13l", 0x00000, 0x10000, CRC(07650c7e) SHA1(0a8eec76aefd4fd1515c1a0d5b96f71c674cdce7) )
	ROM_LOAD16_BYTE( "136082-2030.13h", 0x00001, 0x10000, CRC(e2bf2a26) SHA1(be15b3b0e302382518436441875a1b72954a589a) )

	ROM_REGION( 0x20000, "gfx", ROMREGION_INVERT )
	ROM_LOAD( "136082-1009.2n",   0x000000, 0x20000, CRC(23b95f59) SHA1(cec8523eaf83d4c9bb0055f34024a6e9c52c4c0c) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "136082-1007.2d", 0x00000, 0x20000, CRC(c96a0fc3) SHA1(6e7e242d0afa4714ca31d77ccbf8ee487bbdb1e4) )
	ROM_LOAD( "136082-1008.1d", 0x20000, 0x20000, CRC(518218d9) SHA1(edf1b11579dcfa9a872fa4bd866dc2f95fac767d) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "rampart-eeprom.bin", 0x0000, 0x800, CRC(0be57615) SHA1(bd1f9eef410c78c091d2c925d6275427c77c7ecd) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136082-1000.1j",  0x0000, 0x0117, CRC(18f82b38) SHA1(2ffd43a143396617704ced51da78fec2cf12cced) )
	ROM_LOAD( "gal16v8-136082-1001.4l",  0x0200, 0x0117, CRC(74d75d68) SHA1(dc3ee765ec48a76af6433026243284437958a39a) )
	ROM_LOAD( "gal16v8-136082-1002.7k",  0x0400, 0x0117, CRC(f593401f) SHA1(fbc258cd389f397a005a522812d412f4ed9bf407) )
	ROM_LOAD( "gal20v8-136082-1003.8j",  0x0600, 0x0157, CRC(67bb9705) SHA1(65bb31421f1303fce546781a463cc76921e58b25) )
	ROM_LOAD( "gal20v8-136082-1004.8m",  0x0800, 0x0157, CRC(0001ed7d) SHA1(c16a695361ee17d7508f6fb46854a9189549e3a3) )
	ROM_LOAD( "gal16v8-136082-1006.12c", 0x0a00, 0x0117, CRC(1f3b735d) SHA1(d3243cb3565e32e25637987de6044fe6e453d2f0) )
ROM_END


ROM_START( rampart2p )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136082-1033.13l",  0x00000, 0x80000, CRC(5c36795f) SHA1(2f3dcdfd6b04d851aa1082848624687ac0cec9e2) )
	ROM_LOAD16_BYTE( "136082-1032.13j",  0x00001, 0x80000, CRC(ec7bc38c) SHA1(72d4dbb11e92c69cb560bbb39d7bbd5e845b1e4d) )
	ROM_LOAD16_BYTE( "136082-2051.13kl", 0x00000, 0x20000, CRC(d4e26d0f) SHA1(5106549e6d003711bfd390aa2e19e6e5f33f2cf9) )
	ROM_LOAD16_BYTE( "136082-2050.13h",  0x00001, 0x20000, CRC(ed2a49bd) SHA1(b97ee41b7f930ba7b8b113c1b19c7729a5880b1f) )

	ROM_REGION( 0x20000, "gfx", ROMREGION_INVERT )
	ROM_LOAD( "136082-1019.2n",   0x000000, 0x20000, CRC(efa38bef) SHA1(d38448138134e7a0be2a75c3cd6ab0729da5b83b) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "136082-1007.2d", 0x00000, 0x20000, CRC(c96a0fc3) SHA1(6e7e242d0afa4714ca31d77ccbf8ee487bbdb1e4) )
	ROM_LOAD( "136082-1008.1d", 0x20000, 0x20000, CRC(518218d9) SHA1(edf1b11579dcfa9a872fa4bd866dc2f95fac767d) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "rampart-eeprom.bin", 0x0000, 0x800, CRC(0be57615) SHA1(bd1f9eef410c78c091d2c925d6275427c77c7ecd) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136082-1000.1j",  0x0000, 0x0117, CRC(18f82b38) SHA1(2ffd43a143396617704ced51da78fec2cf12cced) )
	ROM_LOAD( "gal16v8-136082-1001.4l",  0x0200, 0x0117, CRC(74d75d68) SHA1(dc3ee765ec48a76af6433026243284437958a39a) )
	ROM_LOAD( "gal16v8-136082-1002.7k",  0x0400, 0x0117, CRC(f593401f) SHA1(fbc258cd389f397a005a522812d412f4ed9bf407) )
	ROM_LOAD( "gal20v8-136082-1003.8j",  0x0600, 0x0157, CRC(67bb9705) SHA1(65bb31421f1303fce546781a463cc76921e58b25) )
	ROM_LOAD( "gal20v8-136082-1004.8m",  0x0800, 0x0157, CRC(0001ed7d) SHA1(c16a695361ee17d7508f6fb46854a9189549e3a3) )
	ROM_LOAD( "gal16v8-136082-1056.12c", 0x0a00, 0x0117, CRC(bd70bf25) SHA1(e89ed789fae0c5776a10bbebc7dda1d85fc79374) )
ROM_END


ROM_START( rampart2pa ) // original Atari PCB but with mostly hand-written labels, uses smaller ROMs for the main CPU
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "0h.13k-l",  0x00000, 0x20000, CRC(d4e26d0f) SHA1(5106549e6d003711bfd390aa2e19e6e5f33f2cf9) )
	ROM_LOAD16_BYTE( "0l.13h",    0x00001, 0x20000, CRC(ed2a49bd) SHA1(b97ee41b7f930ba7b8b113c1b19c7729a5880b1f) )
	ROM_LOAD16_BYTE( "1h.13l",    0x40000, 0x20000, CRC(b232b807) SHA1(1e405371595d97d44dec97387a0dedc6bc1ad1d2) )
	ROM_LOAD16_BYTE( "1l.13h-j",  0x40001, 0x20000, CRC(a2db78b1) SHA1(586ee0b5901b685ac3c2bfad1d8ce1cd51def292) )
	ROM_LOAD16_BYTE( "2h.13m",    0x80000, 0x20000, CRC(37b32b7e) SHA1(3f6c969829b5ca866e8e162ccecdf9ec7c17d808) )
	ROM_LOAD16_BYTE( "2l.13j",    0x80001, 0x20000, CRC(00cd567b) SHA1(1d6ee16dd5af3328365dafb2fc771396f53dbc44) )
	ROM_LOAD16_BYTE( "3h.13n",    0xc0000, 0x20000, CRC(c23b1c98) SHA1(abd8e2738bb945476dc9f848290880d7ece92081) )
	ROM_LOAD16_BYTE( "3l.13k",    0xc0001, 0x20000, CRC(0a12ca83) SHA1(2f44420b94b981af1d65cb775e4799bc43898041) )

	ROM_REGION( 0x20000, "gfx", ROMREGION_INVERT )
	ROM_LOAD( "atr.2n",   0x000000, 0x20000, CRC(efa38bef) SHA1(d38448138134e7a0be2a75c3cd6ab0729da5b83b) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "arom0_2_player_136082-1007.2d", 0x00000, 0x20000, CRC(c96a0fc3) SHA1(6e7e242d0afa4714ca31d77ccbf8ee487bbdb1e4) )
	ROM_LOAD( "arom1_2_player_136082-1006.1d", 0x20000, 0x20000, CRC(518218d9) SHA1(edf1b11579dcfa9a872fa4bd866dc2f95fac767d) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "rampart-eeprom.bin", 0x0000, 0x800, CRC(0be57615) SHA1(bd1f9eef410c78c091d2c925d6275427c77c7ecd) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136082-1000.1j",  0x0000, 0x0117, CRC(18f82b38) SHA1(2ffd43a143396617704ced51da78fec2cf12cced) ) // not dumped for this set but same part number as rampart2p
	ROM_LOAD( "gal16v8-136082-1001.4l",  0x0200, 0x0117, CRC(74d75d68) SHA1(dc3ee765ec48a76af6433026243284437958a39a) ) // not dumped for this set but same part number as rampart2p
	ROM_LOAD( "gal16v8-136082-1002.7k",  0x0400, 0x0117, CRC(f593401f) SHA1(fbc258cd389f397a005a522812d412f4ed9bf407) ) // not dumped for this set but same part number as rampart2p
	ROM_LOAD( "gal20v8-136082-1003.8j",  0x0600, 0x0157, CRC(67bb9705) SHA1(65bb31421f1303fce546781a463cc76921e58b25) ) // not dumped for this set but same part number as rampart2p
	ROM_LOAD( "gal20v8-136082-1004.8m",  0x0800, 0x0157, CRC(0001ed7d) SHA1(c16a695361ee17d7508f6fb46854a9189549e3a3) ) // not dumped for this set but same part number as rampart2p
	ROM_LOAD( "gal16v8-136082-1005.12c", 0x0a00, 0x0117, CRC(42c05114) SHA1(869a7f07da2d096b5a62f694db0dc1ca62d62242) ) // dumped for this set, matches rampartj (same part number)
ROM_END


ROM_START( rampartj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136082-3451.13kl", 0x00000, 0x20000, CRC(c6596d32) SHA1(3e3e0cbb3b5fc6dd9685bbc4b18c22e0858d9282) )
	ROM_LOAD16_BYTE( "136082-3450.13h",  0x00001, 0x20000, CRC(563b33cc) SHA1(8b454bc19644f1d3d76e4a13f08071cf5eab36e2) )
	ROM_LOAD16_BYTE( "136082-1463.13l",  0x40000, 0x20000, CRC(65fe3491) SHA1(3aa3b98fb7fe808ef89e100b5e1ee1c99c4312b6) )
	ROM_LOAD16_BYTE( "136082-1462.13hj", 0x40001, 0x20000, CRC(ba731652) SHA1(298adda4fd67991b5153e5316f50da79320754ee) )
	ROM_LOAD16_BYTE( "136082-1465.13m",  0x80000, 0x20000, CRC(9cb87d1b) SHA1(95f24ec2c42b39878b3680c4948bfb0d712cd60e) )
	ROM_LOAD16_BYTE( "136082-1464.13j",  0x80001, 0x20000, CRC(2ff75c40) SHA1(9c444402d237c3933219ab4872f180abc392547f) )
	ROM_LOAD16_BYTE( "136082-1467.13n",  0xc0000, 0x20000, CRC(e0cfcda5) SHA1(0a1bf083e0589260caf6dfcb4e556b8f5e1ece25) )
	ROM_LOAD16_BYTE( "136082-1466.13k",  0xc0001, 0x20000, CRC(a7a5a951) SHA1(a9a6adfa315c41cde4cca07d7e7d7f79ecba9f7a) )

	ROM_REGION( 0x20000, "gfx", ROMREGION_INVERT )
	ROM_LOAD( "136082-2419.2m",   0x000000, 0x20000, CRC(456a8aae) SHA1(f35a3dc2069e20493661cf35fc0d4f4c4e11e420) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM data
	ROM_LOAD( "136082-1007.2d", 0x00000, 0x20000, CRC(c96a0fc3) SHA1(6e7e242d0afa4714ca31d77ccbf8ee487bbdb1e4) )
	ROM_LOAD( "136082-1008.1d", 0x20000, 0x20000, CRC(518218d9) SHA1(edf1b11579dcfa9a872fa4bd866dc2f95fac767d) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "rampartj-eeprom.13f", 0x0000, 0x800, CRC(096cacdc) SHA1(48328a27ce1975a27d9a83ae05d068cee7556a90) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136082-1000.1j",  0x0000, 0x0117, CRC(18f82b38) SHA1(2ffd43a143396617704ced51da78fec2cf12cced) )
	ROM_LOAD( "gal16v8-136082-1001.4l",  0x0200, 0x0117, CRC(74d75d68) SHA1(dc3ee765ec48a76af6433026243284437958a39a) )
	ROM_LOAD( "gal16v8-136082-1002.7k",  0x0400, 0x0117, CRC(f593401f) SHA1(fbc258cd389f397a005a522812d412f4ed9bf407) )
	ROM_LOAD( "gal20v8-136082-1003.8j",  0x0600, 0x0157, CRC(67bb9705) SHA1(65bb31421f1303fce546781a463cc76921e58b25) )
	ROM_LOAD( "gal20v8-136082-1004.8m",  0x0800, 0x0157, CRC(0001ed7d) SHA1(c16a695361ee17d7508f6fb46854a9189549e3a3) )
	ROM_LOAD( "gal16v8-136082-1005.12c", 0x0a00, 0x0117, CRC(42c05114) SHA1(869a7f07da2d096b5a62f694db0dc1ca62d62242) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, rampart,    0,       rampart, rampart,  rampart_state, empty_init, ROT0, "Atari Games", "Rampart (Trackball)",              MACHINE_SUPPORTS_SAVE )
GAME( 1990, rampart2p,  rampart, rampart, ramprt2p, rampart_state, empty_init, ROT0, "Atari Games", "Rampart (Joystick, bigger ROMs)",  MACHINE_SUPPORTS_SAVE )
GAME( 1990, rampart2pa, rampart, rampart, ramprt2p, rampart_state, empty_init, ROT0, "Atari Games", "Rampart (Joystick, smaller ROMs)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, rampartj,   rampart, rampart, rampartj, rampart_state, empty_init, ROT0, "Atari Games", "Rampart (Japan, Joystick)",        MACHINE_SUPPORTS_SAVE )
