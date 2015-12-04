// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Nanos

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/upd765.h"
#include "formats/nanos_dsk.h"
#include "machine/ram.h"


class nanos_state : public driver_device
{
public:
	nanos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pio(*this, "z80pio"),
	m_pio_0(*this, "z80pio_0"),
	m_pio_1(*this, "z80pio_1"),
	m_sio_0(*this, "z80sio_0"),
	m_sio_1(*this, "z80sio_1"),
	m_ctc_0(*this, "z80ctc_0"),
	m_ctc_1(*this, "z80ctc_1"),
	m_fdc(*this, "upd765"),
	m_key_t(*this, "keyboard_timer"),
	m_ram(*this, RAM_TAG),
	m_region_maincpu(*this, "maincpu"),
	m_bank1(*this, "bank1"),
	m_bank2(*this, "bank2"),
	m_bank3(*this, "bank3"),
	m_line0(*this, "LINE0"),
	m_line1(*this, "LINE1"),
	m_line2(*this, "LINE2"),
	m_line3(*this, "LINE3"),
	m_line4(*this, "LINE4"),
	m_line5(*this, "LINE5"),
	m_line6(*this, "LINE6"),
	m_linec(*this, "LINEC")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<z80pio_device> m_pio_0;
	required_device<z80pio_device> m_pio_1;
	required_device<z80sio0_device> m_sio_0;
	required_device<z80sio0_device> m_sio_1;
	required_device<z80ctc_device> m_ctc_0;
	required_device<z80ctc_device> m_ctc_1;
	required_device<upd765a_device> m_fdc;
	required_device<timer_device> m_key_t;
	required_device<ram_device> m_ram;
	const UINT8 *m_p_chargen;
	UINT8 m_key_command;
	UINT8 m_last_code;
	UINT8 m_key_pressed;
	DECLARE_WRITE8_MEMBER( nanos_tc_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	virtual void machine_reset();
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	DECLARE_WRITE_LINE_MEMBER(z80daisy_interrupt);
	DECLARE_READ8_MEMBER(nanos_port_a_r);
	DECLARE_READ8_MEMBER(nanos_port_b_r);
	DECLARE_WRITE8_MEMBER(nanos_port_b_w);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	required_ioport m_linec;
	UINT8 row_number(UINT8 code);
};



static ADDRESS_MAP_START(nanos_mem, AS_PROGRAM, 8, nanos_state)
	AM_RANGE( 0x0000, 0x0fff ) AM_READ_BANK("bank1") AM_WRITE_BANK("bank3")
	AM_RANGE( 0x1000, 0xffff ) AM_RAMBANK("bank2")
ADDRESS_MAP_END

WRITE8_MEMBER(nanos_state::nanos_tc_w)
{
	m_fdc->tc_w(BIT(data,1));
}


/* Z80-CTC Interface */

WRITE_LINE_MEMBER( nanos_state::ctc_z0_w )
{
}

WRITE_LINE_MEMBER( nanos_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( nanos_state::ctc_z2_w )
{
}

/* Z80-SIO Interface */

WRITE_LINE_MEMBER(nanos_state::z80daisy_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

/* Z80 Daisy Chain */

static const z80_daisy_config nanos_daisy_chain[] =
{
	{ "z80pio" },
	{ "z80pio_0" },
	{ "z80pio_1" },
	{ "z80sio_0" },
	{ "z80ctc_0" },
	{ "z80sio_1" },
	{ "z80ctc_1" },
	{ nullptr }
};

static ADDRESS_MAP_START( nanos_io , AS_IO, 8, nanos_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* CPU card */
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)

	/* I/O card */
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("z80pio_0", z80pio_device, read, write)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("z80sio_0", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x88, 0x8B) AM_DEVREADWRITE("z80pio_1", z80pio_device, read, write)
	AM_RANGE(0x8C, 0x8F) AM_DEVREADWRITE("z80ctc_0", z80ctc_device, read, write)

	/* FDC card */
	AM_RANGE(0x92, 0x92) AM_WRITE(nanos_tc_w)
	AM_RANGE(0x94, 0x95) AM_DEVICE("upd765", upd765a_device, map)
	/* V24+IFSS card */
	AM_RANGE(0xA0, 0xA3) AM_DEVREADWRITE("z80sio_0", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0xA4, 0xA7) AM_DEVREADWRITE("z80ctc_1", z80ctc_device, read, write)

	/* 256-k RAM card I  -  64k OS-Memory + 192k-RAM-Floppy */
	//AM_RANGE(0xC0, 0xC7)

	/* 256-k RAM card II -  64k OS-Memory + 192k-RAM-Floppy */
	//AM_RANGE(0xC8, 0xCF)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( nanos )
	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~")  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END


void nanos_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 nanos_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  static UINT8 framecnt=0;
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

//  framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				if (ra < 8)
				{
					chr = m_ram->pointer()[0xf800+ x];

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | ra ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
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
		ma+=80;
	}
	return 0;
}

READ8_MEMBER(nanos_state::nanos_port_a_r)
{
	UINT8 retVal;
	if (m_key_command==0)  {
		return m_key_pressed;
	} else {
		retVal = m_last_code;
		m_last_code = 0;
		return retVal;
	}
}

READ8_MEMBER(nanos_state::nanos_port_b_r)
{
	return 0xff;
}


WRITE8_MEMBER(nanos_state::nanos_port_b_w)
{
	m_key_command = BIT(data,1);
	if (BIT(data,7)) {
		m_bank1->set_base(m_region_maincpu->base());
	} else {
		m_bank1->set_base(m_ram->pointer());
	}
}

UINT8 nanos_state::row_number(UINT8 code)
{
	if BIT(code,0) return 0;
	if BIT(code,1) return 1;
	if BIT(code,2) return 2;
	if BIT(code,3) return 3;
	if BIT(code,4) return 4;
	if BIT(code,5) return 5;
	if BIT(code,6) return 6;
	if BIT(code,7) return 7;
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(nanos_state::keyboard_callback)
{
	ioport_port *io_ports[] = { m_line0, m_line1, m_line2, m_line3, m_line4, m_line5, m_line6 };

	int i;
	UINT8 code;
	UINT8 key_code = 0;
	UINT8 shift = m_linec->read() & 0x02 ? 1 : 0;
	UINT8 ctrl =  m_linec->read() & 0x01 ? 1 : 0;
	m_key_pressed = 0xff;
	for(i = 0; i < 7; i++)
	{
		code = io_ports[i]->read();
		if (code != 0)
		{
			if (i==0 && shift==0) {
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}
			if (i==0 && shift==1) {
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers
			}
			if (i==1 && shift==0) {
				if (row_number(code) < 4) {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i==1 && shift==1) {
				if (row_number(code) < 4) {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i>=2 && i<=4 && shift==1 && ctrl==0) {
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
			}
			if (i>=2 && i<=4 && shift==0 && ctrl==0) {
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters
			}
			if (i>=2 && i<=4 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters
			}
			if (i==5 && shift==1 && ctrl==0) {
				if (row_number(code)<7) {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
				} else {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for signs it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==0) {
				if (row_number(code)<7) {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for small letters
				} else {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for signs it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for letters + ctrl
			}
			if (i==6) {
				switch(row_number(code))
				{
					case 0: key_code = 0x11; break;
					case 1: key_code = 0x12; break;
					case 2: key_code = 0x13; break;
					case 3: key_code = 0x14; break;
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0D; break; // Enter
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0A; break; // LF
				}
			}
			m_last_code = key_code;
		}
	}
	if (key_code==0){
		m_key_pressed = 0xf7;
	}
}

void nanos_state::machine_start()
{
	m_key_pressed = 0xff;
}

void nanos_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_write_bank(0x0000, 0x0fff, "bank3");
	space.install_write_bank(0x1000, 0xffff, "bank2");

	m_bank1->set_base(m_region_maincpu->base());
	m_bank2->set_base(m_ram->pointer() + 0x1000);
	m_bank3->set_base(m_ram->pointer());

	machine().device<floppy_connector>("upd765:0")->get_device()->mon_w(false);
}

FLOPPY_FORMATS_MEMBER( nanos_state::floppy_formats )
	FLOPPY_NANOS_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( nanos_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

/* F4 Character Displayer */
static const gfx_layout nanos_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( nanos )
	GFXDECODE_ENTRY( "chargen", 0x0000, nanos_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( nanos, nanos_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(nanos_mem)
	MCFG_CPU_IO_MAP(nanos_io)
	MCFG_CPU_CONFIG(nanos_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(nanos_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0,80*8-1,0,25*10-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nanos)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* devices */
	MCFG_DEVICE_ADD("z80ctc_0", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(nanos_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(nanos_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(nanos_state, ctc_z2_w))

	MCFG_DEVICE_ADD("z80ctc_1", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(nanos_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(nanos_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(nanos_state, ctc_z2_w))

	MCFG_DEVICE_ADD("z80pio_0", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80pio_1", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("z80sio_0", XTAL_4MHz, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(WRITELINE(nanos_state, z80daisy_interrupt))

	MCFG_Z80SIO0_ADD("z80sio_1", XTAL_4MHz, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(WRITELINE(nanos_state, z80daisy_interrupt))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(nanos_state, nanos_port_a_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(nanos_state, nanos_port_b_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(nanos_state, nanos_port_b_w))

	/* UPD765 */
	MCFG_UPD765A_ADD("upd765", false, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", nanos_floppies, "525hd", nanos_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", nanos_state, keyboard_callback, attotime::from_hz(24000))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( nanos )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "k7634_1.rom", 0x0000, 0x0800, CRC(8e34e6ac) SHA1(fd342f6effe991823c2a310737fbfcba213c4fe3))
	ROM_LOAD( "k7634_2.rom", 0x0800, 0x0180, CRC(4e01b02b) SHA1(8a279da886555c7470a1afcbb3a99693ea13c237))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "zg_nanos.rom", 0x0000, 0x0800, CRC(5682d3f9) SHA1(5b738972c815757821c050ee38b002654f8da163))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY                                              FULLNAME       FLAGS */
COMP( 1985, nanos,  0,      0,       nanos,     nanos, driver_device,    0,   "Ingenieurhochschule fur Seefahrt Warnemunde/Wustrow", "NANOS", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
