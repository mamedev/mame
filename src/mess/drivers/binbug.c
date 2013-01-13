/***************************************************************************

        BINBUG

        2013-01-14 Driver created

        All input must be in uppercase.

        Commands:
        A - See and alter memory
        B - Set breakpoint (2 permitted)
        C - Clear breakpoint
        D - Dump memory to paper tape
        G - Go to address, run
        L - Load memory from paper tape
        S - See and alter registers

        BINBUG is an alternate bios to PIPBUG, however it uses its own
        video output. Method of output is through a DG640 board (sold by
        Applied Technology) which one can suppose a character generator
        rom would be used; therefore undumped.

        Keyboard input, like PIPBUG, is via a serial device.
        The baud rate is 300, 8N1.

        The terminal output (via D command) seems to contain binary
        data, so it is disabled for now. It is most likely the tape
        output mentioned below.

        There are 3 versions of BINBUG:

        - 3.6 300 baud tape routines, 300 baud keyboard, memory-mapped VDU

        - 4.4 300 baud keyboard, ACOS tape system, advanced video routines

        - 5.2 ACOS tape system, 1200 baud terminal


        ToDo:
        - Need dumps of 4.4 and 5.2, also the DG640 CGEN.
        - Add cassette

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/terminal.h"
#include "machine/keyboard.h"
#include "imagedev/snapquik.h"


class binbug_state : public driver_device
{
public:
	binbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_keyboard(*this, KEYBOARD_TAG),
//	m_terminal(*this, TERMINAL_TAG),
	m_p_videoram(*this, "videoram") { }

	DECLARE_WRITE8_MEMBER(binbug_ctrl_w);
	DECLARE_READ8_MEMBER(binbug_serial_r);
	DECLARE_WRITE8_MEMBER(binbug_serial_w);
	const UINT8 *m_p_chargen;
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<serial_keyboard_device> m_keyboard;
//	required_device<serial_terminal_device> m_terminal;
	required_shared_ptr<const UINT8> m_p_videoram;
};

WRITE8_MEMBER( binbug_state::binbug_ctrl_w )
{
}

READ8_MEMBER( binbug_state::binbug_serial_r )
{
	return m_keyboard->tx_r();
}

WRITE8_MEMBER( binbug_state::binbug_serial_w )
{
//	m_terminal->rx_w(data);
}

static ADDRESS_MAP_START(binbug_mem, AS_PROGRAM, 8, binbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff) AM_ROM
	AM_RANGE( 0x0400, 0x77ff) AM_RAM
	AM_RANGE( 0x7800, 0x7fff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(binbug_io, AS_IO, 8, binbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_WRITE(binbug_ctrl_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(binbug_serial_r,binbug_serial_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( binbug )
INPUT_PORTS_END

static const serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};

void binbug_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 binbug_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x];
					gfx = m_p_chargen[(chr<<4) | ra ] ^ (BIT(chr, 7) ? 0xff : 0);
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
		ma+=64;
	}
	return 0;
}

QUICKLOAD_LOAD( binbug )
{
	address_space &space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;
	int quick_addr = 0x0440;
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

	if (quick_data[0] != 0xc4)
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

	if (quick_length < 0x444)
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

	for (i = quick_addr; i < quick_length; i++)
	{
		space.write_byte(i, quick_data[i]);
	}

	/* display a message about the loaded quickload */
	image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

	// Start the quickload
	image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( binbug, binbug_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(binbug_mem)
	MCFG_CPU_IO_MAP(binbug_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(binbug_state, screen_update)
	MCFG_SCREEN_SIZE(512, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 159)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(monochrome_amber)

	MCFG_SERIAL_KEYBOARD_ADD(KEYBOARD_TAG, terminal_intf, 300)
//	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, 300)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", binbug, "pgm", 1)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( binbug )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "binbug.rom", 0x0000, 0x0400, CRC(2cb1ac6e) SHA1(a969883fc767484d6b0fa103cfa4b4129b90441b) )

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
	ROM_FILL(0, 16, 0)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS         INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1980, binbug, pipbug,   0,     binbug,    binbug, driver_device, 0,  "MicroByte", "BINBUG 3.6", GAME_NO_SOUND_HW )
