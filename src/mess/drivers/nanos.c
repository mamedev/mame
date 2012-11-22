/***************************************************************************

        Nanos

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/z80ctc.h"
#include "machine/upd765.h"
#include "formats/mfi_dsk.h"
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
	m_key_t(*this, "keyboard_timer")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<z80pio_device> m_pio_0;
	required_device<z80pio_device> m_pio_1;
	required_device<z80sio_device> m_sio_0;
	required_device<z80sio_device> m_sio_1;
	required_device<z80ctc_device> m_ctc_0;
	required_device<z80ctc_device> m_ctc_1;
	required_device<upd765a_device> m_fdc;
	required_device<device_t> m_key_t;
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

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* interrupt handler */
	DEVCB_DRIVER_LINE_MEMBER(nanos_state, ctc_z0_w),	/* ZC/TO0 callback */
	DEVCB_DRIVER_LINE_MEMBER(nanos_state, ctc_z1_w),	/* ZC/TO1 callback */
	DEVCB_DRIVER_LINE_MEMBER(nanos_state, ctc_z2_w)		/* ZC/TO2 callback */
};

/* Z80-PIO Interface */

static Z80PIO_INTERFACE( pio1_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_NULL,						/* port A read callback */
	DEVCB_NULL,						/* port A write callback */
	DEVCB_NULL,						/* portA ready active callback */
	DEVCB_NULL,						/* port B read callback */
	DEVCB_NULL,						/* port B write callback */
	DEVCB_NULL						/* portB ready active callback */
};

static Z80PIO_INTERFACE( pio2_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_NULL,						/* port A read callback */
	DEVCB_NULL,						/* port A write callback */
	DEVCB_NULL,						/* portA ready active callback */
	DEVCB_NULL,						/* port B read callback */
	DEVCB_NULL,						/* port B write callback */
	DEVCB_NULL						/* portB ready active callback */
};

/* Z80-SIO Interface */

WRITE_LINE_MEMBER(nanos_state::z80daisy_interrupt)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, state);
}

static const z80sio_interface sio_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(nanos_state, z80daisy_interrupt),	/* interrupt handler */
	DEVCB_NULL,				/* DTR changed handler */
	DEVCB_NULL,				/* RTS changed handler */
	DEVCB_NULL,				/* BREAK changed handler */
	DEVCB_NULL,				/* transmit handler */
	DEVCB_NULL				/* receive handler */
};

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
	{ NULL }
};
static ADDRESS_MAP_START( nanos_io , AS_IO, 8, nanos_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* CPU card */
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)

	/* I/O card */
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("z80pio_0", z80pio_device, read, write)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("z80sio_0", z80sio_device, read_alt, write_alt)
	AM_RANGE(0x88, 0x8B) AM_DEVREADWRITE("z80pio_1", z80pio_device, read, write)
	AM_RANGE(0x8C, 0x8F) AM_DEVREADWRITE("z80ctc_0", z80ctc_device, read, write)

	/* FDC card */
	AM_RANGE(0x92, 0x92) AM_WRITE(nanos_tc_w)
	AM_RANGE(0x94, 0x95) AM_DEVICE("upd765", upd765a_device, map)
	/* V24+IFSS card */
	AM_RANGE(0xA0, 0xA3) AM_DEVREADWRITE("z80sio_0", z80sio_device, read_alt, write_alt)
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
					chr = machine().device<ram_device>(RAM_TAG)->pointer()[0xf800+ x];

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
		membank("bank1")->set_base(memregion("maincpu")->base());
	} else {
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());
	}
}

static UINT8 row_number(UINT8 code) {
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
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6" };

	int i;
	UINT8 code;
	UINT8 key_code = 0;
	UINT8 shift = machine().root_device().ioport("LINEC")->read() & 0x02 ? 1 : 0;
	UINT8 ctrl =  machine().root_device().ioport("LINEC")->read() & 0x01 ? 1 : 0;
	m_key_pressed = 0xff;
	for(i = 0; i < 7; i++)
	{

		code =	machine().root_device().ioport(keynames[i])->read();
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
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	space.install_write_bank(0x0000, 0x0fff, "bank3");
	space.install_write_bank(0x1000, 0xffff, "bank2");

	membank("bank1")->set_base(machine().root_device().memregion("maincpu")->base());
	membank("bank2")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + 0x1000);
	membank("bank3")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());

	machine().device<floppy_connector>("upd765:0")->get_device()->mon_w(false);
}

static Z80PIO_INTERFACE( nanos_z80pio_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(nanos_state,nanos_port_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(nanos_state,nanos_port_b_r),
	DEVCB_DRIVER_MEMBER(nanos_state,nanos_port_b_w),
	DEVCB_NULL
};

FLOPPY_FORMATS_MEMBER( nanos_state::floppy_formats )
	FLOPPY_NANOS_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( nanos_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

/* F4 Character Displayer */
static const gfx_layout nanos_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8					/* every char takes 8 bytes */
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
	MCFG_GFXDECODE(nanos)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	/* devices */
	MCFG_Z80CTC_ADD( "z80ctc_0", XTAL_4MHz, ctc_intf)
	MCFG_Z80CTC_ADD( "z80ctc_1", XTAL_4MHz, ctc_intf)
	MCFG_Z80PIO_ADD( "z80pio_0", XTAL_4MHz, pio1_intf)
	MCFG_Z80PIO_ADD( "z80pio_1", XTAL_4MHz, pio2_intf)
	MCFG_Z80SIO_ADD( "z80sio_0", XTAL_4MHz, sio_intf)
	MCFG_Z80SIO_ADD( "z80sio_1", XTAL_4MHz, sio_intf)
	MCFG_Z80PIO_ADD( "z80pio", XTAL_4MHz, nanos_z80pio_intf )
	/* UPD765 */
	MCFG_UPD765A_ADD("upd765", false, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", nanos_floppies, "525hd", 0, nanos_state::floppy_formats)

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
COMP( 1985, nanos,  0,      0,       nanos,     nanos, driver_device,    0,   "Ingenieurhochschule fur Seefahrt Warnemunde/Wustrow", "NANOS", GAME_NOT_WORKING | GAME_NO_SOUND)
