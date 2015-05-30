// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Hanimex Pencil II
    Manufactured by Soundic, Hong Kong.

    2012-11-06 [Robbbert]

    Computer kindly donated for MESS by Ian Farquhar.

    Accessories:
    - PEN-216 : 16k RAM expansion
    - PEN-264 : 64k RAM expansion
    - PEN-511 : Data Cassette Recorder
    - ???     : Printer
    - ???     : Floppy Disk Drive (5.25)
    - ???     : Floppy Disk Controller
    - ???     : RS-232C Serial Interface
    - ???     : Coleco Adapter*
    - PEN-8xx : Various software on Cassette or Floppy Disk
    - ???     : Game Controller (joystick and 14 buttons)
    - PEN-7xx : Various software in Cartridge format
    - PEN-430 : Modem
    - PEN-902 : Computer power supply
    - PEN-962 : Monitor cable

    * The cart slot is the same as that found on the Colecovision console. By plugging the
      Coleco Adapter into the expansion slot, Colecovision cartridges can be plugged into the
      cart slot and played.

Information found by looking inside the computer
------------------------------------------------
Main Board PEN-002 11-50332-10

J1 Expansion slot
J2 Cart slot
J3 Memory expansion slot
J4 Printer slot
J5,J6 Joystick ports

XTAL 10.738 MHz

Output is to a TV on Australian Channel 1.

U1     uPD780C-1 (Z80A)
U2     Video chip with heatsink stuck on top, TMS9929
U3     SN76489AN
U4     2764 bios rom
U5     uPD4016C-2 (assumed to be equivalent of 6116 2K x 8bit static RAM)
U6     74LS04
U7     74LS74A
U8-10  74LS138
U11    74LS00
U12    74LS273
U13    74LS74A
U14-21 TMM416P-3 (4116-3 16k x 1bit dynamic RAM)
U22    74LS05
U23-24 SN74LS541


SD-BASIC usage:
All commands must be in uppercase, which is the default at boot.
The 'capslock' is done by pressing Shift and Esc together, and the
cursor will change to a checkerboard pattern.


MEMORY MAP
0000-1FFF bios rom
2000-5FFF available for expansion
6000-7FFF static RAM (2K mirrored)
8000-FFFF cart slot

The 16k dynamic RAM holds the BASIC program and the video/gfx etc
but is banked out of view of a BASIC program.


ToDo:
- Joysticks (no info)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class pencil2_state : public driver_device
{
public:
	pencil2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_centronics(*this, "centronics")
		, m_cass(*this, "cassette")
		, m_cart(*this, "cartslot")
	{}

	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(port30_w);
	DECLARE_WRITE8_MEMBER(port80_w);
	DECLARE_WRITE8_MEMBER(portc0_w);
	DECLARE_READ8_MEMBER(porte2_r);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_CUSTOM_INPUT_MEMBER(printer_ready_r);
	DECLARE_CUSTOM_INPUT_MEMBER(printer_ack_r);
private:
	virtual void machine_start();
	int m_centronics_busy;
	int m_centronics_ack;
	bool m_cass_state;
	required_device<cpu_device> m_maincpu;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cass;
	required_device<generic_slot_device> m_cart;
};

static ADDRESS_MAP_START(pencil2_mem, AS_PROGRAM, 8, pencil2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x5fff) AM_WRITENOP  // stop error log filling up
	AM_RANGE(0x6000, 0x67ff) AM_MIRROR(0x1800) AM_RAM
	//AM_RANGE(0x8000, 0xffff)      // mapped by the cartslot
ADDRESS_MAP_END

static ADDRESS_MAP_START(pencil2_io, AS_IO, 8, pencil2_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x10, 0x1f) AM_WRITE(port10_w)
	AM_RANGE(0x30, 0x3f) AM_WRITE(port30_w)
	AM_RANGE(0x80, 0x9f) AM_WRITE(port80_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xdf) AM_WRITE(portc0_w)
	AM_RANGE(0xe0, 0xff) AM_DEVWRITE("sn76489a", sn76489a_device, write)
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("E0")
	AM_RANGE(0xe1, 0xe1) AM_READ_PORT("E1")
	AM_RANGE(0xe2, 0xe2) AM_READ(porte2_r)
	AM_RANGE(0xe3, 0xe3) AM_READ_PORT("E3")
	AM_RANGE(0xe4, 0xe4) AM_READ_PORT("E4")
	AM_RANGE(0xe6, 0xe6) AM_READ_PORT("E6")
	AM_RANGE(0xe8, 0xe8) AM_READ_PORT("E8")
	AM_RANGE(0xea, 0xea) AM_READ_PORT("EA")
	AM_RANGE(0xf0, 0xf0) AM_READ_PORT("F0")
	AM_RANGE(0xf2, 0xf2) AM_READ_PORT("F2")
ADDRESS_MAP_END

READ8_MEMBER( pencil2_state::porte2_r)
{
	return (m_cass->input() > 0.1) ? 0xff : 0x7f;
}

WRITE8_MEMBER( pencil2_state::port10_w )
{
	m_centronics->write_strobe(BIT(data, 0));
}

WRITE8_MEMBER( pencil2_state::port30_w )
{
	m_cass_state ^= 1;
	m_cass->output( m_cass_state ? -1.0 : +1.0);
}

WRITE8_MEMBER( pencil2_state::port80_w )
{
}

WRITE8_MEMBER( pencil2_state::portc0_w )
{
}

WRITE_LINE_MEMBER( pencil2_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

CUSTOM_INPUT_MEMBER( pencil2_state::printer_ready_r )
{
	return m_centronics_busy;
}

WRITE_LINE_MEMBER( pencil2_state::write_centronics_ack )
{
	m_centronics_ack = state;
}

CUSTOM_INPUT_MEMBER( pencil2_state::printer_ack_r )
{
	return m_centronics_ack;
}


/* Input ports */
static INPUT_PORTS_START( pencil2 )
	PORT_START("E0")
	// port_custom MUST be ACTIVE_HIGH to work
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, pencil2_state, printer_ready_r, " ")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, pencil2_state, printer_ack_r, " ")

	PORT_START("E1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR('/')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("F0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("F2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void pencil2_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xffff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
}

static MACHINE_CONFIG_START( pencil2, pencil2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)
	MCFG_CPU_PROGRAM_MAP(pencil2_mem)
	MCFG_CPU_IO_MAP(pencil2_io)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A, XTAL_10_738635MHz/3) // guess
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pencil2_cart")

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(pencil2_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(pencil2_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "pencil2")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pencil2 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "mt.u4", 0x0000, 0x2000, CRC(338d7b59) SHA1(2f89985ac06971e00210ff992bf1e30a296d10e7) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     STATE         INIT  COMPANY    FULLNAME       FLAGS */
COMP( 1983, pencil2,   0,     0,     pencil2,   pencil2, driver_device,  0,  "Hanimex", "Pencil II", 0 )
