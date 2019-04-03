// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Central Data 2650 Computer System

2010-04-08 Skeleton driver.

No info available on this computer apart from a few newsletters and
magazine articles. The computer was described in a series of articles
published between April and June 1977 in Radio-Electronics, which include
supposedly complete schematics and fairly detailed subsystem descriptions.
There is supposed to be a “Computer System Manual” with definitive revised
schematics, but this has not been found yet.

All signals to and from the 2650 board (including the built-in 300 baud
Kansas City standard cassette tape interface) are passed through six ribbon
cables. Central Data later produced an “extender board” that adapted the
bus signals to a S-100 backplane. This interface was missing a considerable
number of standard S-100 timing signals, though it was compatible at least
with some dynamic RAM boards released by the company.

The unusual XTAL frequency seems deliberately chosen to produce a vertical
sync rate of exactly 60 Hz.

The system only uses 1000-14FF for videoram and 17F0-17FF for
scratch ram. All other ram is optional.

Commands (must be in uppercase):
A    Examine memory; press C to alter memory
B    Set breakpoint?
C    View breakpoint?
D    Dump to tape
E    Execute
I    ?
L    Load
R    ?
V    Verify?
Press Esc to exit most commands.

TODO
- Lots, probably. The computer is a complete mystery. No manuals are known to exist.
- Cassette doesn't work.

****************************************************************************/

#define CHARACTER_WIDTH 8
#define CHARACTER_HEIGHT 8
#define CHARACTER_LINES 12

#include "emu.h"
#include "cpu/s2650/s2650.h"
//#include "bus/s100/s100.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/74259.h"
#include "machine/keyboard.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class cd2650_state : public driver_device
{
public:
	cd2650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_cass(*this, "cassette")
	{
	}

	DECLARE_READ8_MEMBER(keyin_r);
	void kbd_put(u8 data);
	DECLARE_WRITE_LINE_MEMBER(tape_deck_on_w);
	DECLARE_READ_LINE_MEMBER(cass_r);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(cd2650);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cd2650(machine_config &config);
	void cd2650_data(address_map &map);
	void cd2650_io(address_map &map);
	void cd2650_mem(address_map &map);
private:
	uint8_t m_term_data;
	virtual void machine_reset() override;
	required_device<s2650_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<cassette_image_device> m_cass;
};


WRITE_LINE_MEMBER(cd2650_state::tape_deck_on_w)
{
	// output polarity not verified
	logerror("Cassette tape deck turned %s\n", state ? "on" : "off");
}

WRITE_LINE_MEMBER(cd2650_state::cass_w)
{
	m_cass->output(state ? -1.0 : +1.0);
}

READ_LINE_MEMBER(cd2650_state::cass_r)
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

READ8_MEMBER(cd2650_state::keyin_r)
{
	uint8_t ret = m_term_data;
	m_term_data = ret | 0x80;
	return ret;
}

void cd2650_state::cd2650_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("roms", 0);
	map(0x1000, 0x7fff).ram().share("videoram");
}

void cd2650_state::cd2650_io(address_map &map)
{
	map.unmap_value_high();
	//AM_RANGE(0x80, 0x84) disk i/o
}

void cd2650_state::cd2650_data(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(cd2650_state::keyin_r)).w("outlatch", FUNC(f9334_device::write_nibble_d3));
}

/* Input ports */
static INPUT_PORTS_START( cd2650 )
INPUT_PORTS_END


void cd2650_state::machine_reset()
{
	m_term_data = 0x80;
}

uint32_t cd2650_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/* The video is unusual in that the characters in each line are spaced at 16 bytes in memory,
    thus line 1 starts at 1000, line 2 at 1001, etc. There are 16 lines of 80 characters.
    Further, the letters have bit 6 set low, thus the range is 01 to 1A.
    When the bottom of the screen is reached, it does not scroll, it just wraps around. */

	uint16_t offset = 0;
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,x,mem;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < CHARACTER_LINES; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < CHARACTER_HEIGHT)
				{
					mem = offset + y + (x<<4);

					if (mem > 0x4ff)
						mem -= 0x500;

					chr = m_p_videoram[mem] & 0x3f;

					gfx = m_p_chargen[(bitswap<8>(chr,7,6,2,1,0,3,4,5)<<3) | ra];
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout cd2650_charlayout =
{
	8, 8,                  /* 8 x 8 characters */
	192,                    /* 64 characters in char.rom + 128 characters in char2.rom */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_cd2650 )
	GFXDECODE_ENTRY( "chargen", 0x0000, cd2650_charlayout, 0, 1 )
GFXDECODE_END

void cd2650_state::kbd_put(u8 data)
{
	if (data)
		m_term_data = data;
}

QUICKLOAD_LOAD_MEMBER( cd2650_state, cd2650 )
{
	int i;
	image_init_result result = image_init_result::FAIL;

	int quick_length = image.length();
	if (quick_length < 0x1500)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
	}
	else
	if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		std::vector<uint8_t> quick_data(quick_length);
		int read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
			image.message(" Cannot read the file");
		}
		else
		if (quick_data[0] != 0x40)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
			image.message(" Invalid header");
		}
		else
		{
			int exec_addr = quick_data[1] * 256 + quick_data[2];

			if (exec_addr >= quick_length)
			{
				image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
				image.message(" Exec address beyond end of file");
			}
			else
			{
				// do not overwite system area (17E0-17FF) otherwise chess3 has problems
				read_ = 0x17e0;
				if (quick_length < 0x17e0)
					read_ = quick_length;

				for (i = 0x1500; i < read_; i++)
					m_p_videoram[i-0x1000] = quick_data[i];

				if (quick_length > 0x17ff)
					for (i = 0x1800; i < quick_length; i++)
						m_p_videoram[i-0x1000] = quick_data[i];

				/* display a message about the loaded quickload */
				image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

				// Start the quickload
				m_maincpu->set_state_int(S2650_PC, exec_addr);

				result = image_init_result::PASS;
			}
		}
	}

	return result;
}

void cd2650_state::cd2650(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(14'192'640) / 12); // 1.182720MHz according to RE schematic
	m_maincpu->set_addrmap(AS_PROGRAM, &cd2650_state::cd2650_mem);
	m_maincpu->set_addrmap(AS_IO, &cd2650_state::cd2650_io);
	m_maincpu->set_addrmap(AS_DATA, &cd2650_state::cd2650_data);
	m_maincpu->sense_handler().set(FUNC(cd2650_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(cd2650_state::cass_w));

	f9334_device &outlatch(F9334(config, "outlatch")); // IC26
	outlatch.q_out_cb<0>().set(FUNC(cd2650_state::tape_deck_on_w)); // TD ON
	outlatch.q_out_cb<7>().set("beeper", FUNC(beep_device::set_state)); // OUT6
	// Q1-Q7 = OUT 0-6, not defined in RE
	// The connection of OUT6 to a 700-1200 Hz noise generator is suggested
	// in Central Data 2650 Newsletter, Volume 1, Issue 3 for use with the
	// "Morse Code" program by Mike Durham.

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'192'640), 112 * CHARACTER_WIDTH, 0, 80 * CHARACTER_WIDTH, 22 * CHARACTER_LINES, 0, 16 * CHARACTER_LINES);
	screen.set_screen_update(FUNC(cd2650_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_cd2650);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload"));
	quickload.set_handler(snapquick_load_delegate(&QUICKLOAD_LOAD_NAME(cd2650_state, cd2650), this), "pgm", attotime::from_seconds(1));

	/* Sound */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", m_cass).add_route(ALL_OUTPUTS, "mono", 0.25);
	BEEP(config, "beeper", 950).add_route(ALL_OUTPUTS, "mono", 0.50); // guess

	/* Devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(cd2650_state::kbd_put));
	CASSETTE(config, m_cass);
}

/* ROM definition */
ROM_START( cd2650 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "cd2650.rom", 0x0000, 0x0400, CRC(5397328e) SHA1(7106fdb60e1ad2bc5e8e45527f348c23296e8d6a))

	ROM_REGION( 0x0600, "chargen", 0 )
	ROM_LOAD( "char.rom",   0x0000, 0x0200, CRC(9b75db2a) SHA1(4367c01afa503d7cba0c38078fde0b95392c6c2c))
	ROM_LOAD_OPTIONAL( "char2.rom",  0x0200, 0x0400, CRC(b450eea8) SHA1(c1bdba52c2dc5698cad03b6b884b942a083465ed)) // not used

	// various unused roms found on Amigan site
	ROM_REGION( 0xc900, "user1", 0 )
	ROM_LOAD_OPTIONAL( "01a_cd_boots.bin", 0x0000, 0x0200, CRC(5336c62f) SHA1(e94cf7be01ea806ff7c7b90aee1a4e88f4f1ba9f))
	ROM_LOAD_OPTIONAL( "01a_cd_dos.bin",   0x0200, 0x2000, CRC(3f177cdd) SHA1(01afd77ad2f065158cbe032aa26682cb20afe7d8))
	ROM_LOAD_OPTIONAL( "01a_cd_pop.bin",   0x2200, 0x1000, CRC(d8f44f11) SHA1(605ab5a045290fa5b99ff4fc8fbfa2a3f202578f))
	ROM_LOAD_OPTIONAL( "01b_cd_alp.bin",   0x3200, 0x2a00, CRC(b05568bb) SHA1(29e74633c0cd731c0be25313288cfffdae374236))
	ROM_LOAD_OPTIONAL( "01b_cd_basic.bin", 0x5c00, 0x3b00, CRC(0cf1e3d8) SHA1(3421e679c238aeea49cd170b34a6f344da4770a6))
	ROM_LOAD_OPTIONAL( "01b_cd_mon_m.bin", 0x9700, 0x0400, CRC(f6f19c08) SHA1(1984d85d57fc2a6c5a3bd51fbc58540d7129a0ae))
	ROM_LOAD_OPTIONAL( "01b_cd_mon_o.bin", 0x9b00, 0x0400, CRC(9d40b4dc) SHA1(35cffcbd983b7b37c878a15af44100568d0659d1))
	ROM_LOAD_OPTIONAL( "02b_cd_alp.bin",   0x9f00, 0x2a00, CRC(a66b7f32) SHA1(2588f9244b0ec6b861dcebe666d37d3fa88dd043))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY         FULLNAME                FLAGS
COMP( 1977, cd2650, 0,      0,      cd2650,  cd2650, cd2650_state, empty_init, "Central Data", "2650 Computer System", 0 )
