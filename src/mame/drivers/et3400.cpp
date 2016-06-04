// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    ET-3400

    2009-05-12 Skeleton driver.
    2016-04-29 Added Accessory.

    ETA-3400 Memory I/O Accessory
    - Provides Tiny Basic, a Terminal, a Serial Interface, a Cassette
      interface, and 1k to 4k of expansion RAM. All parts are working.
    - The roms are U105 (Monitor), U106 (Tiny Basic), both type NMOS2316E,
      and U108 (address decoder PROM).
    - Navigating:
           LED to Monitor: D1400
           Monitor to Basic: G 1C00
           Monitor to LED: G FC00
           Basic to Monitor: BYE
    - All commands in Basic and Monitor are UPPERCASE only.
    - Terminal is defaulted to 9600 baud, 7 bits, 2 stop bits.


****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "et3400.lh"


class et3400_state : public driver_device
{
public:
	et3400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
		, m_rs232(*this, "rs232")
		, m_cass(*this, "cassette")
	{ }

	DECLARE_READ8_MEMBER(keypad_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_READ8_MEMBER(pia_ar);
	DECLARE_WRITE8_MEMBER(pia_aw);
	DECLARE_READ8_MEMBER(pia_br);
	DECLARE_WRITE8_MEMBER(pia_bw);
private:
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cass;
};



READ8_MEMBER( et3400_state::keypad_r )
{
	UINT8 data = 0xff;

	if (~offset & 4)
		data &= ioport("X2")->read();
	if (~offset & 2)
		data &= ioport("X1")->read();
	if (~offset & 1)
		data &= ioport("X0")->read();

	return data;
}

WRITE8_MEMBER( et3400_state::display_w )
{
/* This computer sets each segment, one at a time. */

	static const UINT8 segments[8]={0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80};
	UINT8 digit = (offset >> 4) & 7;
	UINT8 segment = segments[offset & 7];
	UINT8 segdata = output().get_digit_value(digit);

	if (data & 1)
		segdata |= segment;
	else
		segdata &= ~segment;

	output().set_digit_value(digit, segdata);
}

// d1,2,3 = Baud rate
// d4 = gnd
// d7 = rs232 in
READ8_MEMBER( et3400_state::pia_ar )
{
	return ioport("BAUD")->read() | (m_rs232->rxd_r() << 7);
}

// d0 = rs232 out
WRITE8_MEMBER( et3400_state::pia_aw )
{
	m_rs232->write_txd(BIT(data, 0));
}

// d7 = cass in
READ8_MEMBER( et3400_state::pia_br )
{
	return (m_cass->input() > +0.0) << 7;
}

// d0 = cass out
WRITE8_MEMBER( et3400_state::pia_bw )
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}


static ADDRESS_MAP_START(et3400_mem, AS_PROGRAM, 8, et3400_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_RAM
	AM_RANGE( 0x1000, 0x1003 ) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE( 0x1400, 0x23ff ) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0xc000, 0xc0ff ) AM_READ(keypad_r)
	AM_RANGE( 0xc100, 0xc1ff ) AM_WRITE(display_w)
	AM_RANGE( 0xfc00, 0xffff ) AM_ROM AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( et3400 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D DO") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A AUTO") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 RTI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 INDEX") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 ACCA") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E EXAM") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B BACK") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 SS") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 CC") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 ACCB") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F FWD") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C CHAN") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 BR") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 SP") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 PC") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )

	PORT_START("BAUD")
	PORT_DIPNAME( 0x0E, 0x02, "Baud Rate" )
	PORT_DIPSETTING(    0x0E, "110" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x0A, "600" )
	PORT_DIPSETTING(    0x08, "1200" )
	PORT_DIPSETTING(    0x06, "2400" )
	PORT_DIPSETTING(    0x04, "4800" )
	PORT_DIPSETTING(    0x02, "9600" )
INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( et3400, et3400_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_4MHz / 4 ) // 1MHz with memory i/o accessory, or 500khz without it
	MCFG_CPU_PROGRAM_MAP(et3400_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_et3400)

	// Devices
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(et3400_state, pia_aw))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(et3400_state, pia_bw))
	MCFG_PIA_READPA_HANDLER(READ8(et3400_state, pia_ar))
	MCFG_PIA_READPB_HANDLER(READ8(et3400_state, pia_br))
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( et3400 )
	ROM_REGION( 0x1420, "roms", 0 )
	ROM_LOAD( "monitor.u105",      0x0000, 0x0800, CRC(e4142682) SHA1(785966018dd6eb097ed9bd5c7def2354ab4347db) )
	ROM_LOAD( "basic.u106",        0x0800, 0x0800, CRC(bbd6a801) SHA1(088da24bd4d923d4f196b993154c538835d10605) )
	ROM_LOAD( "et3400.ic12",       0x1000, 0x0400, CRC(2eff1f58) SHA1(38b655de7393d7a92b08276f7c14a99eaa2a4a9f) )
	ROM_LOAD_OPTIONAL("prom.u108", 0x1400, 0x0020, CRC(273025c3) SHA1(136c1cdce2a4a796c1c46e8ea4f798cdee4b549b) ) // not used
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT    COMPANY       FULLNAME       FLAGS */
COMP( 1976, et3400,  0,     0,       et3400,    et3400, driver_device,  0,    "Heath Inc", "Heathkit ET-3400", 0 )
