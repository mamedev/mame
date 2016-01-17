// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Schachcomputer SC1

        12/05/2009 Skeleton driver.

ToDo:
- speaker
- LEDs
- 7seg sometimes flashes

Port 80-83 could be a device

This happens at the start:
'maincpu' (04EF): unmapped i/o memory write to 0081 = 0F & FF
'maincpu' (04F3): unmapped i/o memory write to 0083 = CF & FF
'maincpu' (04F7): unmapped i/o memory write to 0083 = BB & FF
'maincpu' (04FB): unmapped i/o memory write to 0082 = 01 & FF
'maincpu' (0523): unmapped i/o memory write to 00FC = 02 & FF  **
'maincpu' (0523): unmapped i/o memory write to 00FC = 04 & FF  **

** These two happen for a while (making a tone from a speaker?)

Then:
'maincpu' (0523): unmapped i/o memory write to 00FC = 00 & FF
'maincpu' (0075): unmapped i/o memory write to 0080 = 02 & FF
'maincpu' (0523): unmapped i/o memory write to 00FC = 20 & FF

Then this happens continuously:
Port 80 out - 00, 02, FF
Port FC out - 00, 01, 02, 04, 08, 10, 20, 40, 80 (selecting rows?)
Port 80 in - upper byte = 20 thru 26
Port 82 in - upper byte = 0 thru 7

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "machine/z80pio.h"
#include "sc1.lh"


class sc1_state : public driver_device
{
public:
	sc1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_WRITE8_MEMBER( matrix_w );
	DECLARE_WRITE8_MEMBER( pio_port_a_w );
	DECLARE_READ8_MEMBER( pio_port_b_r );

	UINT8 m_matrix;
};

/***************************************************************************

    Display

***************************************************************************/

WRITE8_MEMBER( sc1_state::pio_port_a_w )
{
	UINT8 digit = BITSWAP8( data,3,4,6,0,1,2,7,5 );

	if (m_matrix & 0x04)
		output().set_digit_value(3, digit & 0x7f);
	if (m_matrix & 0x08)
		output().set_digit_value(2, digit & 0x7f);
	if (m_matrix & 0x10)
		output().set_digit_value(1, digit & 0x7f);
	if (m_matrix & 0x20)
		output().set_digit_value(0, digit & 0x7f);
}


/***************************************************************************

    Keyboard

***************************************************************************/

WRITE8_MEMBER( sc1_state::matrix_w )
{
	m_matrix = data;
}

READ8_MEMBER( sc1_state::pio_port_b_r )
{
	UINT8 data = 0;

	if (m_matrix & 0x01)
		data |= ioport("LINE1")->read();
	if (m_matrix & 0x02)
		data |= ioport("LINE2")->read();
	if (m_matrix & 0x04)
		data |= ioport("LINE3")->read();
	if (m_matrix & 0x08)
		data |= ioport("LINE4")->read();
	if (m_matrix & 0x10)
		data |= ioport("LINE5")->read();
	if (m_matrix & 0x20)
		data |= ioport("LINE6")->read();
	if (m_matrix & 0x40)
		data |= ioport("LINE7")->read();
	if (m_matrix & 0x80)
		data |= ioport("LINE8")->read();

	return data;
}


static ADDRESS_MAP_START(sc1_mem, AS_PROGRAM, 8, sc1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x4000, 0x43ff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sc1_io, AS_IO, 8, sc1_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0xfc, 0xfc) AM_WRITE(matrix_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sc1 )
	PORT_START("LINE1")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_D)

	PORT_START("LINE2")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_B)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F)

	PORT_START("LINE3")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_C)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_G)

	PORT_START("LINE4")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_A)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_E)

	PORT_START("LINE5")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_H)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE") PORT_CODE(KEYCODE_DEL)

	PORT_START("LINE6")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE7")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W/S") PORT_CODE(KEYCODE_W)

	PORT_START("LINE8")
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END


static MACHINE_CONFIG_START( sc1, sc1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(sc1_mem)
	MCFG_CPU_IO_MAP(sc1_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_sc1)

	/* devices */
	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(sc1_state, pio_port_a_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(sc1_state, pio_port_b_r))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                           FULLNAME       FLAGS */
COMP( 1989, sc1,    0,      0,       sc1,       sc1, driver_device,     0,  "VEB Mikroelektronik Erfurt", "Schachcomputer SC1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
