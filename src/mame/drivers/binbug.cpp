// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        BINBUG and DG640

        2013-01-14 Driver created

        All input must be in uppercase.

        Commands:
        A - See and alter memory
        B - Set breakpoint (2 permitted)
        C - Clear breakpoint
        D - cassette save
        G - Go to address, run
        L - cassette load
        S - See and alter registers

        BINBUG is an alternate bios to PIPBUG, however it uses its own
        video output. Method of output is through a DG640 board (sold by
        Applied Technology) which uses a MCM6574 as a character generator.
        The DG640 also supports blinking, reverse-video, and LORES graphics.
        It is a S100 card, also known as ETI-640.

        Keyboard input, like PIPBUG, is via a serial device.
        The baud rate is 300, 8N1.

        The SENSE and FLAG lines are used for 300 baud cassette, in
        conjunction with unknown hardware.

        There are 3 versions of BINBUG:

        - 3.6 300 baud tape routines, 300 baud keyboard, memory-mapped VDU

        - 4.4 300 baud keyboard, ACOS tape system, advanced video routines

        - 5.2 ACOS tape system, 1200 baud terminal


        Status:
        - DG640 is completely emulated, even though BINBUG doesn't use most
          of its features.
        - BINBUG works except that the cassette interface isn't the same as
          real hardware. But you can save, and load it back.

        ToDo:
        - Need dumps of 4.4 and 5.2.
        - Fix cassette

****************************************************************************/

#include "bus/rs232/keyboard.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "imagedev/snapquik.h"

#define KEYBOARD_TAG "keyboard"

class binbug_state : public driver_device
{
public:
	binbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rs232(*this, KEYBOARD_TAG),
		m_cass(*this, "cassette"),
		m_p_videoram(*this, "videoram"),
		m_p_attribram(*this, "attribram"),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_WRITE8_MEMBER(binbug_ctrl_w);
	DECLARE_READ8_MEMBER(binbug_serial_r);
	DECLARE_WRITE_LINE_MEMBER(binbug_serial_w);
	const UINT8 *m_p_chargen;
	UINT8 m_framecnt;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	optional_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cass;
	required_shared_ptr<UINT8> m_p_videoram;
	required_shared_ptr<UINT8> m_p_attribram;
	required_device<cpu_device> m_maincpu;
	DECLARE_QUICKLOAD_LOAD_MEMBER( binbug );
};

WRITE8_MEMBER( binbug_state::binbug_ctrl_w )
{
}

READ8_MEMBER( binbug_state::binbug_serial_r )
{
	return m_rs232->rxd_r() & (m_cass->input() < 0.03);
}

WRITE_LINE_MEMBER( binbug_state::binbug_serial_w )
{
	m_cass->output(state ? -1.0 : +1.0);
}

static ADDRESS_MAP_START(binbug_mem, AS_PROGRAM, 8, binbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff) AM_ROM
	AM_RANGE( 0x0400, 0x77ff) AM_RAM
	AM_RANGE( 0x7800, 0x7bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE( 0x7c00, 0x7fff) AM_RAM AM_SHARE("attribram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(binbug_io, AS_IO, 8, binbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_WRITE(binbug_ctrl_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(binbug_serial_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( binbug )
INPUT_PORTS_END

void binbug_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 binbug_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// attributes bit 0 = flash, bit 1 = lores. Also bit 7 of the character = reverse-video (text only).
	UINT8 y,ra,chr,gfx,attr,inv,gfxbit;
	UINT16 sy=0,ma=0,x;
	bool flash;
	m_framecnt++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				attr = m_p_attribram[x];
				chr = m_p_videoram[x];
				flash = BIT(m_framecnt, 4) & BIT(attr, 0);

				if (BIT(attr, 1)) // lores gfx - can flash
				{
					if (flash) chr = 0; // blank part of flashing

					gfxbit = (ra & 0x0c)>>1;
					/* Display one line of a lores character (8 pixels) */
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					gfxbit++;
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
				}
				else
				{
					gfx = 0;

					if (!flash)
					{
						inv = BIT(chr, 7) ? 0xff : 0; // text with bit 7 high is reversed
						chr &= 0x7f;
						gfx = inv;

						// if g,j,p,q,y; lower the descender
						if ((chr==0x2c)||(chr==0x3b)||(chr==0x67)||(chr==0x6a)||(chr==0x70)||(chr==0x71)||(chr==0x79))
						{
							if (ra > 6)
								gfx = m_p_chargen[(chr<<4) | (ra-7) ] ^ inv;
						}
						else
						{
							if ((ra > 3) & (ra < 13))
								gfx = m_p_chargen[(chr<<4) | (ra-4) ] ^ inv;
						}
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
		ma+=64;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout dg640_charlayout =
{
	7, 9,                   /* 7 x 9 characters */
	128,                  /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( dg640 )
	GFXDECODE_ENTRY( "chargen", 0x0000, dg640_charlayout, 0, 1 )
GFXDECODE_END

QUICKLOAD_LOAD_MEMBER( binbug_state, binbug )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x440;
	int exec_addr;
	int quick_length;
	dynamic_buffer quick_data;
	int read_;
	int result = IMAGE_INIT_FAIL;

	quick_length = image.length();
	if (quick_length < 0x0444)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
	}
	else if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		quick_data.resize(quick_length);
		read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
			image.message(" Cannot read the file");
		}
		else if (quick_data[0] != 0xc4)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
			image.message(" Invalid header");
		}
		else
		{
			exec_addr = quick_data[1] * 256 + quick_data[2];

			if (exec_addr >= quick_length)
			{
				image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
				image.message(" Exec address beyond end of file");
			}
			else
			{
				for (i = quick_addr; i < read_; i++)
					space.write_byte(i, quick_data[i]);

				/* display a message about the loaded quickload */
				image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

				// Start the quickload
				m_maincpu->set_state_int(S2650_PC, exec_addr);

				result = IMAGE_INIT_PASS;
			}
		}
	}

	return result;
}

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( binbug, binbug_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(binbug_mem)
	MCFG_CPU_IO_MAP(binbug_io)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(binbug_state, binbug_serial_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(binbug_state, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 255)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dg640)
	MCFG_PALETTE_ADD_MONOCHROME_AMBER("palette")

	/* Keyboard */
	MCFG_RS232_PORT_ADD(KEYBOARD_TAG, default_rs232_devices, "keyboard")
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("keyboard", keyboard)

	/* Cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", binbug_state, binbug, "pgm", 1)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( binbug )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "binbug.rom", 0x0000, 0x0400, CRC(2cb1ac6e) SHA1(a969883fc767484d6b0fa103cfa4b4129b90441b) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "6574.bin", 0x0000, 0x0800, CRC(fd75df4f) SHA1(4d09aae2f933478532b7d3d1a2dee7123d9828ca) )
	ROM_FILL(0, 16, 0x00)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS         INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1980, binbug, pipbug,   0,     binbug,    binbug, driver_device, 0,  "MicroByte", "BINBUG 3.6", 0 )



/******************************************************************************************************************/

/*

DG680 (ETI-680), using the DGOS-Z80 operating system.

This is a S100 card.

In some ways, this system is the ancestor of the original Microbee.

ROM is typed in from a PDF, therefore marked a bad dump.

No schematic available, most of this is guesswork.

Port 0 is the input from an ascii keyboard.

Port 2 is the cassette interface.

Port 8 controls some kind of memory protection scheme.
The code indicates that B is the page to protect, and
A is the code (0x08 = inhibit; 0x0B = unprotect;
0x0C = enable; 0x0E = protect). There are 256 pages so
each page is 256 bytes.

The clock is controlled by the byte in D80D.

Monitor Commands:
C (compare)*
E (edit)*
F (fill)*
G - Go to address
I - Inhibit CTC
M (move)*
P (clear screen)*
R (read tape)*
S (search)*
T hhmm [ss] - Set the time
W (write tape)*
X - protection status
XC - clear ram
XD - same as X
XE - enable facilities
XF - disable facilities
XP - protect block
XU - unprotect block
Z - go to 0000.

* These commands are identical to the Microbee ones.

ToDo:
- dips
- leds
- need schematic to find out what else is missing
- cassette
- ctc / clock

*/


#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "cpu/z80/z80daisy.h"


class dg680_state : public binbug_state
{
public:
	dg680_state(const machine_config &mconfig, device_type type, const char *tag)
		: binbug_state(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ctc(*this, "z80ctc"),
	m_pio(*this, "z80pio")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	TIMER_DEVICE_CALLBACK_MEMBER(time_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(uart_tick);
	UINT8 m_pio_b;
	UINT8 m_term_data;
	UINT8 m_protection[0x100];
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
};

static ADDRESS_MAP_START(dg680_mem, AS_PROGRAM, 8, dg680_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xcfff) AM_RAM
	AM_RANGE( 0xd000, 0xd7ff) AM_ROM
	AM_RANGE( 0xd800, 0xefff) AM_RAM
	AM_RANGE( 0xf000, 0xf3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE( 0xf400, 0xf7ff) AM_RAM AM_SHARE("attribram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(dg680_io, AS_IO, 8, dg680_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x04,0x07) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x08,0x08) AM_READWRITE(port08_r,port08_w) //SWP Control and Status
	//AM_RANGE(0x09,0x09) parallel input port
	// Optional AM9519 Programmable Interrupt Controller (port c = data, port d = control)
	//AM_RANGE(0x0c,0x0d) AM_DEVREADWRITE("am9519", am9519_device, read, write)
ADDRESS_MAP_END

void dg680_state::machine_reset()
{
	m_maincpu->set_pc(0xd000);
}

// this is a guess there is no information available
static const z80_daisy_config dg680_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ 0x00 }
};


/* Input ports */
static INPUT_PORTS_START( dg680 )
INPUT_PORTS_END

WRITE8_MEMBER( dg680_state::kbd_put )
{
	m_term_data = data;
	/* strobe in keyboard data */
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

READ8_MEMBER( dg680_state::porta_r )
{
	UINT8 data = m_term_data;
	m_term_data = 0;
	return data;
}

READ8_MEMBER( dg680_state::portb_r )
{
	return m_pio_b | (m_cass->input() > 0.03);
}

// bit 1 = cassout; bit 2 = motor on
WRITE8_MEMBER( dg680_state::portb_w )
{
	m_pio_b = data & 0xfe;
	m_cass->output(BIT(data, 1) ? -1.0 : +1.0);
}

READ8_MEMBER( dg680_state::port08_r )
{
	UINT8 breg = m_maincpu->state_int(Z80_B);
	return m_protection[breg];
}

WRITE8_MEMBER( dg680_state::port08_w )
{
	UINT8 breg = m_maincpu->state_int(Z80_B);
	m_protection[breg] = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(dg680_state::time_tick)
{
// ch0 is for the clock
	m_ctc->trg0(1);
	m_ctc->trg0(0);
// no idea about ch2
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(dg680_state::uart_tick)
{
// ch3 is for cassette
	m_ctc->trg3(1);
	m_ctc->trg3(0);
}

static MACHINE_CONFIG_START( dg680, dg680_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_8MHz / 4)
	MCFG_CPU_PROGRAM_MAP(dg680_mem)
	MCFG_CPU_IO_MAP(dg680_io)
	MCFG_CPU_CONFIG(dg680_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(binbug_state, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 255)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dg640)
	MCFG_PALETTE_ADD_MONOCHROME_AMBER("palette")

	/* Keyboard */
	MCFG_DEVICE_ADD("keyb", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(dg680_state, kbd_put))

	/* Cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_8MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg1))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_8MHz / 4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(dg680_state, porta_r))
	// OUT_ARDY - this activates to ask for kbd data but not known if actually used
	MCFG_Z80PIO_IN_PB_CB(READ8(dg680_state, portb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(dg680_state, portb_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc0", dg680_state, time_tick, attotime::from_hz(200))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc3", dg680_state, uart_tick, attotime::from_hz(4800))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( dg680 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dg680.rom", 0xd000, 0x0800, BAD_DUMP CRC(c1aaef6a) SHA1(1508ca8315452edfb984718e795ccbe79a0c0b58) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "6574.bin", 0x0000, 0x0800, CRC(fd75df4f) SHA1(4d09aae2f933478532b7d3d1a2dee7123d9828ca) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin", 0x0000, 0x0020, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT   MACHINE  INPUT    CLASS       INIT    COMPANY            FULLNAME       FLAGS */
COMP( 1980, dg680, 0,      0,       dg680,   dg680, driver_device, 0,  "David Griffiths", "DG680 with DGOS-Z80 1.4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
