// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    Atari "Round" hardware

    driver by Aaron Giles

    Games supported:
        * Off the Wall (1991) [2 sets]

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


// configurable logging
#define LOG_SOUNDCTRL     (1U << 1)
#define LOG_BANKSW        (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_SOUNDCTRL | LOG_BANKSW)

#include "logmacro.h"

#define LOGSOUNDCTRL(...)     LOGMASKED(LOG_SOUNDCTRL,     __VA_ARGS__)
#define LOGBANKSW(...)        LOGMASKED(LOG_BANKSW,        __VA_ARGS__)


namespace {

class offtwall_state : public driver_device
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_mainram(*this, "mainram"),
		m_bankrom_base(*this, "maincpu")
	{ }

	void offtwall(machine_config &config);

	void init_offtwall();
	void init_offtwalc();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_shared_ptr<uint16_t> m_mainram;

	uint16_t *m_bankswitch_base = nullptr;
	required_region_ptr<uint16_t> m_bankrom_base;
	uint32_t m_bank_offset = 0;

	uint16_t *m_spritecache_count = nullptr;
	uint16_t *m_unknown_verify_base = nullptr;

	static const atari_motion_objects_config s_mob_config;

	void io_latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bankswitch_r(offs_t offset);
	uint16_t bankrom_r(address_space &space, offs_t offset);
	uint16_t spritecache_count_r(offs_t offset);
	uint16_t unknown_verify_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(offtwall_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield().extmem_read(tile_index) >> 8;
	int const code = data1 & 0x7fff;
	int const color = 0x10 + (data2 & 0x0f);
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config offtwall_state::s_mob_config =
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



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t offtwall_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
 *  Initialization
 *
 *************************************/

void offtwall_state::machine_start()
{
	save_item(NAME(m_bank_offset));
}

/*************************************
 *
 *  I/O handling
 *
 *************************************/

void offtwall_state::io_latch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// lower byte
	if (ACCESSING_BITS_0_7)
	{
		// bit 4 resets the sound CPU
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
		if (!(data & 0x10))
			m_jsa->reset();
	}

	LOGSOUNDCTRL("sound control = %04X\n", data);
}



/*************************************
 *
 *  SLOOP workarounds
 *
 *************************************/


/*-------------------------------------------------------------------------

    Bankswitching

    Like the slapstic, the SoS bankswitches memory using A13 and A14.
    Unlike the slapstic, the exact addresses to trigger the bankswitch
    are unknown.

    Fortunately, Off the Wall uses a common routine for the important
    bankswitching. The playfield data is stored in the banked area of
    ROM, and by comparing the playfields to a real system, a mechanism
    to bankswitch at the appropriate time was discovered. Fortunately,
    it's really basic.

    OtW looks up the address to read playfield data from a table which
    is 58 words long. Word 0 assumes the bank is 0, word 1 assumes the
    bank is 1, etc. So we just trigger off of the table read and cause
    the bank to switch then.

    In addition, there is code which checksums longs from $40000 down to
    $3e000. The code wants that checksum to be $aaaa5555, but there is
    no obvious way for this to happen. To work around this, we watch for
    the final read from $3e000 and tweak the value such that the checksum
    will come out the $aaaa5555 magically.

-------------------------------------------------------------------------*/



uint16_t offtwall_state::bankswitch_r(offs_t offset)
{
	// this is the table lookup; the bank is determined by the address that was requested
	m_bank_offset = (offset & 3) * 0x1000;
	LOGBANKSW("Bankswitch index %d -> %04X\n", offset, m_bank_offset);

	return m_bankswitch_base[offset];
}


uint16_t offtwall_state::bankrom_r(address_space &space, offs_t offset)
{
	// this is the banked ROM read
	logerror("Banked ROM read: %06X: %04X\n", m_maincpu->pcbase(), offset);

	/* if the values are $3e000 or $3e002 are being read by code just below the
	    ROM bank area, we need to return the correct value to give the proper checksum */
	if ((offset == 0x3000 || offset == 0x3001) && m_maincpu->pcbase() > 0x37000)
	{
		uint32_t const checksum = (space.read_word(0x3fd210) << 16) | space.read_word(0x3fd212);
		uint32_t const us = 0xaaaa5555 - checksum;
		if (offset == 0x3001)
			return us & 0xffff;
		else
			return us >> 16;
	}

	return m_bankrom_base[(0x38000 / 2) | ((m_bank_offset + offset) & 0x3fff)];
}



/*-------------------------------------------------------------------------

    Sprite Cache

    Somewhere in the code, if all the hardware tests are met properly,
    some additional dummy sprites are added to the sprite cache before
    they are copied to sprite RAM. The sprite RAM copy routine computes
    the total width of all sprites as they are copied and if the total
    width is less than or equal to 38, it adds a "HARDWARE ERROR" sprite
    to the end.

    Here we detect the read of the sprite count from within the copy
    routine, and add some dummy sprites to the cache ourself if there
    isn't enough total width.

-------------------------------------------------------------------------*/


uint16_t offtwall_state::spritecache_count_r(offs_t offset)
{
	int const prevpc = m_maincpu->pcbase();

	// if this read is coming from $99f8 or $9992, it's in the sprite copy loop
	if (prevpc == 0x99f8 || prevpc == 0x9992)
	{
		uint16_t *data = &m_spritecache_count[-0x100];
		int const oldword = m_spritecache_count[0];
		int count = oldword >> 8;
		int width = 0;

		// compute the current total width
		for (int i = 0; i < count; i++)
			width += 1 + ((data[i * 4 + 1] >> 4) & 7);

		// if we're less than 39, keep adding dummy sprites until we hit it
		if (width <= 38)
		{
			while (width <= 38)
			{
				data[count * 4 + 0] = (42 * 8) << 7;
				data[count * 4 + 1] = ((30 * 8) << 7) | (7 << 4);
				data[count * 4 + 2] = 0;
				width += 8;
				count++;
			}

			// update the final count in memory
			m_spritecache_count[0] = (count << 8) | (oldword & 0xff);
		}
	}

	// and then read the data
	return m_spritecache_count[offset];
}



/*-------------------------------------------------------------------------

    Unknown Verify

    In several places, the value 1 is stored to the byte at $3fdf1e. A
    fairly complex subroutine is called, and then $3fdf1e is checked to
    see if it was set to zero. If it was, "HARDWARE ERROR" is displayed.

    To avoid this, we just return 1 when this value is read within the
    range of PCs where it is tested.

-------------------------------------------------------------------------*/



uint16_t offtwall_state::unknown_verify_r(offs_t offset)
{
	int const prevpc = m_maincpu->pcbase();
	if (prevpc < 0x5c5e || prevpc > 0xc432)
		return m_unknown_verify_base[offset];
	else
		return m_unknown_verify_base[offset] | 0x100;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void offtwall_state::main_map(address_map &map)
{
	map(0x000000, 0x037fff).rom();
	map(0x038000, 0x03ffff).r(FUNC(offtwall_state::bankrom_r));
	map(0x120000, 0x120fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x260000, 0x260001).portr("260000");
	map(0x260002, 0x260003).portr("260002");
	map(0x260010, 0x260011).portr("260010");
	map(0x260012, 0x260013).portr("260012");
	map(0x260020, 0x260021).portr("260020");
	map(0x260022, 0x260023).portr("260022");
	map(0x260024, 0x260025).portr("260024");
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_iii_device::main_response_r));
	map(0x260041, 0x260041).w(m_jsa, FUNC(atari_jsa_iii_device::main_command_w));
	map(0x260050, 0x260051).w(FUNC(offtwall_state::io_latch_w));
	map(0x260060, 0x260061).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x2a0000, 0x2a0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x3e0000, 0x3e0fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x3effc0, 0x3effff).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x3f4000, 0x3f5eff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_msb_w)).share("vad:playfield");
	map(0x3f5f00, 0x3f5f7f).ram().share("vad:eof");
	map(0x3f5f80, 0x3f5fff).ram().share("vad:mob:slip");
	map(0x3f6000, 0x3f7fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x3f8000, 0x3fcfff).ram();
	map(0x3fd000, 0x3fd7ff).ram().share("vad:mob");
	map(0x3fd800, 0x3fffff).ram().share("mainram");
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( offtwall )
	PORT_START("260000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260002")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0000, "Whirly-gigs" )   // this is official Atari terminology!
	PORT_DIPSETTING(      0x0002, "Joysticks" )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )   // tested at a454
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )   // tested at a466
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")  // tested before writing to 260040
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260012")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260020")
	PORT_BIT( 0xff, 0, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260022")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260024")
	PORT_BIT( 0xff, 0, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)
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
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static GFXDECODE_START( gfx_offtwall )
	GFXDECODE_ENTRY( "sprites", 0, pfmolayout,  256, 32 )      // sprites & playfield
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void offtwall_state::offtwall(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &offtwall_state::main_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_offtwall);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	ATARI_VAD(config, m_vad, 0, "screen");
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	TILEMAP(config, "vad:playfield", "gfxdecode", 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(offtwall_state::get_playfield_tile_info));
	ATARI_MOTION_OBJECTS(config, "vad:mob", 0, "screen", offtwall_state::s_mob_config).set_gfxdecode("gfxdecode");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived
	   the board uses a VAD chip to generate video signals */
	screen.set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	screen.set_screen_update(FUNC(offtwall_state::screen_update));
	screen.set_palette("palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_III(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(6);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 1.0);
	config.device_remove("jsa:oki1");
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( offtwall )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136090-2012.17e", 0x00000, 0x20000, CRC(d08d81eb) SHA1(5a72aa2e4fc6455b94aa59a7719d0ddc8bcc80f2) )
	ROM_LOAD16_BYTE( "136090-2013.17j", 0x00001, 0x20000, CRC(61c2553d) SHA1(343d39f9b75fd236e9769ec21ab65310f85e31ca) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136090-1020.12c", 0x00000, 0x10000, CRC(488112a5) SHA1(55e84855daacfa303d1031de8c9adb992a846e21) )

	ROM_REGION( 0xc0000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136090-1014.14s", 0x000000, 0x20000, CRC(4d64507e) SHA1(cb2ac41aecd2702cd57c746a6f5986cd753bc29e) )
	ROM_LOAD( "136090-1016.14p", 0x020000, 0x20000, CRC(f5454f3a) SHA1(87d82bd227f7fcfd13b6f4ad88a573d1b96a4fc1) )
	ROM_LOAD( "136090-1018.14m", 0x040000, 0x20000, CRC(17864231) SHA1(22f93fcb5d413281157ab8545647f3713f98c135) )
	ROM_LOAD( "136090-1015.18s", 0x060000, 0x20000, CRC(271f7856) SHA1(928bc5e7dc589ceb5f55e536b5a05c3866116a24) )
	ROM_LOAD( "136090-1017.18p", 0x080000, 0x20000, CRC(7f7f8012) SHA1(1123ea3c6cd2c73617a87d6a5bbb26fca8941af3) )
	ROM_LOAD( "136090-1019.18m", 0x0a0000, 0x20000, CRC(9efe511b) SHA1(db1f1d8792bf497bc9ad652b0b7d78c3abf0e817) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "offtwall-eeprom.17l", 0x0000, 0x800, CRC(5eaf2d5b) SHA1(934a76a23960e6ed2cc33c359f9735caee762145) )

	ROM_REGION(0x022f, "jsa:plds", 0)
	ROM_LOAD("136085-1038.17c.bin", 0x0000, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected
	ROM_LOAD("136085-1039.20c.bin", 0x0118, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected

	ROM_REGION(0x2591, "main:plds", 0)
	ROM_LOAD("136090-1001.14l.bin", 0x0000, 0x201d, NO_DUMP ) // GAL6001-35P is read protected
	ROM_LOAD("136090-1002.11r.bin", 0x201e, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected
	ROM_LOAD("136090-1003.15f.bin", 0x2135, 0x0117, CRC(5e723b46) SHA1(e686920d0af342e33f836fec15b6e8b5ef1b8be5)) // GAL16V8A-25LP
	ROM_LOAD("136090-1005.5n.bin",  0x224c, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected
	ROM_LOAD("136090-1006.5f.bin",  0x2363, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected
	ROM_LOAD("136090-1007.3f.bin",  0x247a, 0x0117, NO_DUMP ) // GAL16V8A-25LP is read protected
ROM_END


ROM_START( offtwallc )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "090-2612.rom", 0x00000, 0x20000, CRC(fc891a3f) SHA1(027815a20fbc6c0c9242768581b97362b39941c2) )
	ROM_LOAD16_BYTE( "090-2613.rom", 0x00001, 0x20000, CRC(805d79d4) SHA1(943ec9f408ba875bdf1794ce7d24803043480401) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136090-1020.12c", 0x00000, 0x10000, CRC(488112a5) SHA1(55e84855daacfa303d1031de8c9adb992a846e21) )

	ROM_REGION( 0xc0000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "090-1614.rom", 0x000000, 0x20000, CRC(307ed447) SHA1(acee15e58cd8def8e52a7586aa14240e1f8be319) )
	ROM_LOAD( "090-1616.rom", 0x020000, 0x20000, CRC(a5bd3d9b) SHA1(756d96eac2398dc68679b7641acbf0e79204eebb) )
	ROM_LOAD( "090-1618.rom", 0x040000, 0x20000, CRC(c7d9df5d) SHA1(d5e5fbb7faf42d865862b9ac60f94d20820b00f3) )
	ROM_LOAD( "090-1615.rom", 0x060000, 0x20000, CRC(ac3642c7) SHA1(bb57e039c113c4ce5702983c8e01dbe286d7b58e) )
	ROM_LOAD( "090-1617.rom", 0x080000, 0x20000, CRC(15208a89) SHA1(124484ab54959a1e6d9022a4f3ee4288a79c768b) )
	ROM_LOAD( "090-1619.rom", 0x0a0000, 0x20000, CRC(8a5d79b3) SHA1(0a202d20e6c86989ce2223e10eadf9009dd6ca8e) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "offtwall-eeprom.17l", 0x0000, 0x800, CRC(5eaf2d5b) SHA1(934a76a23960e6ed2cc33c359f9735caee762145) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void offtwall_state::init_offtwall()
{
	// install son-of-slapstic workarounds
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fde42, 0x3fde43, read16sm_delegate(*this, FUNC(offtwall_state::spritecache_count_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x037ec2, 0x037f39, read16sm_delegate(*this, FUNC(offtwall_state::bankswitch_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fdf1e, 0x3fdf1f, read16sm_delegate(*this, FUNC(offtwall_state::unknown_verify_r)));
	m_spritecache_count = m_mainram + (0x3fde42 - 0x3fd800) / 2;
	m_bankswitch_base = (uint16_t *)(memregion("maincpu")->base() + 0x37ec2);
	m_unknown_verify_base = m_mainram + (0x3fdf1e - 0x3fd800) / 2;
}


void offtwall_state::init_offtwalc()
{
	// install son-of-slapstic workarounds
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fde42, 0x3fde43, read16sm_delegate(*this, FUNC(offtwall_state::spritecache_count_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x037eca, 0x037f43, read16sm_delegate(*this, FUNC(offtwall_state::bankswitch_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fdf24, 0x3fdf25, read16sm_delegate(*this, FUNC(offtwall_state::unknown_verify_r)));
	m_spritecache_count = m_mainram + (0x3fde42 - 0x3fd800) / 2;
	m_bankswitch_base = (uint16_t *)(memregion("maincpu")->base() + 0x37eca);
	m_unknown_verify_base = m_mainram + (0x3fdf24 - 0x3fd800) / 2;
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, offtwall,  0,        offtwall, offtwall, offtwall_state, init_offtwall, ROT0, "Atari Games", "Off the Wall (2/3-player upright)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, offtwallc, offtwall, offtwall, offtwall, offtwall_state, init_offtwalc, ROT0, "Atari Games", "Off the Wall (2-player cocktail)",  MACHINE_SUPPORTS_SAVE )
