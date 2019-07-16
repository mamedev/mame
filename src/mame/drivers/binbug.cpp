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

        ToDo:
        - Need dumps of 4.4 and 5.2.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/keyboard.h"
#include "cpu/s2650/s2650.h"
#include "machine/z80daisy.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class binbug_state : public driver_device
{
public:
	binbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pio(*this, "pio")
		, m_p_videoram(*this, "videoram")
		, m_p_attribram(*this, "attribram")
		, m_p_chargen(*this, "chargen")
		, m_rs232(*this, "keyboard")
		, m_clock(*this, "cass_clock")
	{ }

	void binbug_base(machine_config &config);
	void binbug(machine_config &config);

protected:
	DECLARE_WRITE_LINE_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	// needed by dg680 class
	required_device<cpu_device> m_maincpu; // S2650 or Z80
	required_device<cassette_image_device> m_cass;
	optional_device<z80pio_device> m_pio;

	uint8_t m_framecnt;
	u8 m_cass_data[4];
	bool m_cassold, m_cassinbit, m_cassoutbit;

private:
	void binbug_data(address_map &map);
	void binbug_mem(address_map &map);
	DECLARE_WRITE8_MEMBER(binbug_ctrl_w);
	DECLARE_READ_LINE_MEMBER(binbug_serial_r);
	DECLARE_WRITE_LINE_MEMBER(binbug_serial_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_attribram;
	required_region_ptr<u8> m_p_chargen;
	optional_device<rs232_port_device> m_rs232;

public:
	required_device<clock_device> m_clock;
};

WRITE8_MEMBER( binbug_state::binbug_ctrl_w )
{
}

WRITE_LINE_MEMBER( binbug_state::kansas_w )
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_RECORD)
		return;

	u8 twobit = m_cass_data[3] & 15;

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassoutbit;

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( binbug_state::kansas_r )
{
	// no tape - set to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 32)
	{
		m_cass_data[1] = 32;
		m_cassinbit = 1;
	}

	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
		return;

	/* cassette - turn 1200/2400Hz to a bit */
	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
		if (m_pio)
			m_pio->pb0_w(m_cassinbit);
	}
}

READ_LINE_MEMBER( binbug_state::binbug_serial_r )
{
	return m_rs232->rxd_r() & m_cassinbit;
}

WRITE_LINE_MEMBER( binbug_state::binbug_serial_w )
{
	m_cassoutbit = state;
}

void binbug_state::binbug_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0x77ff).ram();
	map(0x7800, 0x7bff).ram().share("videoram");
	map(0x7c00, 0x7fff).ram().share("attribram");
}

void binbug_state::binbug_data(address_map &map)
{
	map.unmap_value_high();
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).w(FUNC(binbug_state::binbug_ctrl_w));
}

/* Input ports */
static INPUT_PORTS_START( binbug )
INPUT_PORTS_END

uint32_t binbug_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// attributes bit 0 = flash, bit 1 = lores. Also bit 7 of the character = reverse-video (text only).
	uint8_t y,ra,chr,gfx,attr,inv,gfxbit;
	uint16_t sy=0,ma=0,x;
	bool flash;
	m_framecnt++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

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

static GFXDECODE_START( gfx_dg640 )
	GFXDECODE_ENTRY( "chargen", 0x0000, dg640_charlayout, 0, 1 )
GFXDECODE_END

QUICKLOAD_LOAD_MEMBER(binbug_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x440;
	int exec_addr;
	int quick_length;
	std::vector<uint8_t> quick_data;
	int read_;
	image_init_result result = image_init_result::FAIL;

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

				result = image_init_result::PASS;
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

void binbug_state::binbug_base(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(binbug_state::screen_update));
	screen.set_size(512, 256);
	screen.set_visarea(0, 511, 0, 255);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_dg640);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(binbug_state::kansas_r), attotime::from_hz(40000));

	CLOCK(config, m_clock, 4'800); // 300 baud x 16(divider) = 4800
	m_clock->signal_handler().set(FUNC(binbug_state::kansas_w));
}

void binbug_state::binbug(machine_config &config)
{
	binbug_base(config);

	/* basic machine hardware */
	s2650_device &maincpu(S2650(config, m_maincpu, XTAL(1'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &binbug_state::binbug_mem);
	maincpu.set_addrmap(AS_DATA, &binbug_state::binbug_data);
	maincpu.sense_handler().set(FUNC(binbug_state::binbug_serial_r));
	maincpu.flag_handler().set(FUNC(binbug_state::binbug_serial_w));

	/* Keyboard */
	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard").set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(binbug_state::quickload_cb), this);
}


/* ROM definition */
ROM_START( binbug )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "binbug.rom", 0x0000, 0x0400, CRC(2cb1ac6e) SHA1(a969883fc767484d6b0fa103cfa4b4129b90441b) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "6574.bin", 0x0000, 0x0800, CRC(fd75df4f) SHA1(4d09aae2f933478532b7d3d1a2dee7123d9828ca) )
	ROM_FILL(0, 16, 0x00)
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS         INIT        COMPANY      FULLNAME      FLAGS
COMP( 1980, binbug, pipbug,   0,     binbug,    binbug, binbug_state, empty_init, "MicroByte", "BINBUG 3.6", 0 )



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

*/


class dg680_state : public binbug_state
{
public:
	dg680_state(const machine_config &mconfig, device_type type, const char *tag)
		: binbug_state(mconfig, type, tag)
		, m_ctc(*this, "ctc")
	{ }

	void dg680(machine_config &config);

private:
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	void kbd_put(u8 data);

	void dg680_io(address_map &map);
	void dg680_mem(address_map &map);

	uint8_t m_pio_b;
	uint8_t m_term_data;
	uint8_t m_protection[0x100];
	virtual void machine_reset() override;
	required_device<z80ctc_device> m_ctc;
};

void dg680_state::dg680_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xcfff).ram();
	map(0xd000, 0xd7ff).rom();
	map(0xd800, 0xefff).ram();
	map(0xf000, 0xf3ff).ram().share("videoram");
	map(0xf400, 0xf7ff).ram().share("attribram");
}

void dg680_state::dg680_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x07).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x08).rw(FUNC(dg680_state::port08_r), FUNC(dg680_state::port08_w)); //SWP Control and Status
	//AM_RANGE(0x09,0x09) parallel input port
	// Optional AM9519 Programmable Interrupt Controller (port c = data, port d = control)
	//AM_RANGE(0x0c,0x0d) AM_DEVREADWRITE("am9519", am9519_device, read, write)
}

void dg680_state::machine_reset()
{
	m_maincpu->set_pc(0xd000);
	m_pio_b = 0xFF;
}

// this is a guess there is no information available
static const z80_daisy_config dg680_daisy_chain[] =
{
	{ "ctc" },
	{ "pio" },
	{ 0x00 }
};


/* Input ports */
static INPUT_PORTS_START( dg680 )
INPUT_PORTS_END

void dg680_state::kbd_put(u8 data)
{
	if (data == 8)
		data = 127;   // fix backspace
	m_term_data = data;
	/* strobe in keyboard data */
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

READ8_MEMBER( dg680_state::porta_r )
{
	uint8_t data = m_term_data;
	m_term_data = 0;
	return data;
}

READ8_MEMBER( dg680_state::portb_r )
{
	return m_pio_b | m_cassinbit;
}

// bit 1 = cassout; bit 2 = motor on
WRITE8_MEMBER( dg680_state::portb_w )
{
	if (BIT(m_pio_b ^ data, 2))
		m_cass->change_state(BIT(data, 2) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_pio_b = data & 0xfe;
	m_cassoutbit = BIT(data, 1);
}

READ8_MEMBER( dg680_state::port08_r )
{
	uint8_t breg = m_maincpu->state_int(Z80_B);
	return m_protection[breg];
}

WRITE8_MEMBER( dg680_state::port08_w )
{
	uint8_t breg = m_maincpu->state_int(Z80_B);
	m_protection[breg] = data;
}


void dg680_state::dg680(machine_config &config)
{
	binbug_base(config);
	m_clock->signal_handler().append(m_ctc, FUNC(z80ctc_device::trg2));
	m_clock->signal_handler().append(m_ctc, FUNC(z80ctc_device::trg3));

	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	/* basic machine hardware */
	z80_device& maincpu(Z80(config, m_maincpu, XTAL(8'000'000) / 4));
	maincpu.set_addrmap(AS_PROGRAM, &dg680_state::dg680_mem);
	maincpu.set_addrmap(AS_IO, &dg680_state::dg680_io);
	maincpu.set_daisy_config(dg680_daisy_chain);

	/* Keyboard */
	generic_keyboard_device &keyb(GENERIC_KEYBOARD(config, "keyb", 0));
	keyb.set_keyboard_callback(FUNC(dg680_state::kbd_put));

	/* Devices */
	Z80CTC(config, m_ctc, XTAL(8'000'000) / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(200);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));

	Z80PIO(config, m_pio, XTAL(8'000'000) / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(dg680_state::porta_r));
	// OUT_ARDY - this activates to ask for kbd data but not known if actually used
	m_pio->in_pb_callback().set(FUNC(dg680_state::portb_r));
	m_pio->out_pb_callback().set(FUNC(dg680_state::portb_w));
}

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

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY            FULLNAME                   FLAGS
COMP( 1980, dg680, 0,      0,      dg680,   dg680, dg680_state, empty_init, "David Griffiths", "DG680 with DGOS-Z80 1.4", MACHINE_NO_SOUND_HW )
