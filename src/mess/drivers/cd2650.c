/***************************************************************************

        Central Data cd2650

        08/04/2010 Skeleton driver.

        No info available on this computer apart from a few newsletters.
        The system only uses 1000-14FF for videoram and 17F0-17FF for
        scratch ram. All other ram is optional.

        All commands must be in upper case. They are A,B,C,D,E,I,L,R,V.
        L,D,V appear to be commands to load, dump and verify tapes.
        The in and output lines for tapes are SENSE and FLAG, which is
        the usual with S2650 systems. The remaining commands have
        unknown functions.

        TODO
        - Lots, probably. The computer is a complete mystery. No pictures,
                manuals or schematics exist.
        - Using the terminal keyboard, as it is unknown if it has its own.
        - Cassette interface to be tested - no idea what the command syntax is for saving.
        - Need the proper chargen rom.

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/keyboard.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class cd2650_state : public driver_device
{
public:
	cd2650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_p_videoram(*this, "p_videoram"),
	m_cass(*this, CASSETTE_TAG) { }

	DECLARE_READ8_MEMBER(cd2650_keyin_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(cass_r);
	DECLARE_WRITE8_MEMBER(cass_w);
	const UINT8 *m_p_chargen;
	UINT8 m_term_data;
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<const UINT8> m_p_videoram;
	required_device<cassette_image_device> m_cass;
};


WRITE8_MEMBER( cd2650_state::cass_w )
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}

READ8_MEMBER( cd2650_state::cass_r )
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

READ8_MEMBER( cd2650_state::cd2650_keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0x80;
	if ((ret > 0x60) && (ret < 0x7b))
		ret -= 0x20; // upper case only
	return ret;
}

static ADDRESS_MAP_START(cd2650_mem, AS_PROGRAM, 8, cd2650_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff) AM_ROM
	AM_RANGE( 0x0400, 0x0fff) AM_RAM
	AM_RANGE( 0x1000, 0x17ff) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE( 0x1800, 0x7fff) AM_RAM // expansion ram needed by quickloads
ADDRESS_MAP_END

static ADDRESS_MAP_START( cd2650_io, AS_IO, 8, cd2650_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(S2650_DATA_PORT,S2650_DATA_PORT) AM_READ(cd2650_keyin_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(cass_r, cass_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cd2650 )
INPUT_PORTS_END


void cd2650_state::machine_reset()
{
	m_term_data = 0x80;
}

void cd2650_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 cd2650_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/* The video is unusual in that the characters in each line are spaced at 16 bytes in memory,
    thus line 1 starts at 1000, line 2 at 1001, etc. There are 16 lines of 80 characters.
    Further, the letters have bit 6 set low, thus the range is 01 to 1A.
    When the bottom of the screen is reached, it does not scroll, it just wraps around. */

	UINT16 offset = 0;
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,x,mem;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					mem = offset + y + (x<<4);

					if (mem > 0x4ff)
						mem -= 0x500;

					chr = m_p_videoram[mem];

					if (chr < 0x20)
						chr |= 0x40;

					gfx = m_p_chargen[(chr<<4) | ra ];
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

WRITE8_MEMBER( cd2650_state::kbd_put )
{
	if (data)
		m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(cd2650_state, kbd_put)
};

QUICKLOAD_LOAD( cd2650 )
{
	address_space &space = *image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;
	int quick_addr = 0x440;
	int exec_addr;
	int quick_length;
	UINT8 *quick_data;
	int read_;

	quick_length = image.length();
	quick_data = (UINT8*)malloc(quick_length);
	if (!quick_data)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot open file");
		image.message(" Cannot open file");
		return IMAGE_INIT_FAIL;
	}

	read_ = image.fread( quick_data, quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_data[0] != 0x40)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
		image.message(" Invalid header");
		return IMAGE_INIT_FAIL;
	}

	exec_addr = quick_data[1] * 256 + quick_data[2];

	if (exec_addr >= quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
		image.message(" Exec address beyond end of file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length < 0x0444)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
		return IMAGE_INIT_FAIL;
	}

	read_ = 0x1000;
	if (quick_length < 0x1000)
		read_ = quick_length;

	for (i = quick_addr; i < read_; i++)
		space.write_byte(i, quick_data[i]);

	read_ = 0x1780;
	if (quick_length < 0x1780)
		read_ = quick_length;

	if (quick_length > 0x157f)
		for (i = 0x1580; i < read_; i++)
			space.write_byte(i, quick_data[i]);

	if (quick_length > 0x17ff)
		for (i = 0x1800; i < quick_length; i++)
			space.write_byte(i, quick_data[i]);

	/* display a message about the loaded quickload */
	image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

	// Start the quickload
	image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( cd2650, cd2650_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(cd2650_mem)
	MCFG_CPU_IO_MAP(cd2650_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(cd2650_state, screen_update)
	MCFG_SCREEN_SIZE(640, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 159)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", cd2650, "pgm", 1)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cd2650 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2650.rom", 0x0000, 0x0400, CRC(5397328e) SHA1(7106fdb60e1ad2bc5e8e45527f348c23296e8d6a))

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY        FULLNAME       FLAGS */
COMP( 1977, cd2650, 0,      0,       cd2650,    cd2650, driver_device,  0,   "Central Data",   "CD 2650", 0 )
