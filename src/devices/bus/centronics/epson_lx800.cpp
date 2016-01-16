// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

   Epson LX-800 dot matrix printer

    Skeleton driver

    - CPU type uPD7810HG
    - CPU PORTD and PORTF are connected to the Gate Array
    - processing gets stuck in a loop, and never gets to scan the
      input buttons and switches.
    - CPU disassembly doesn't seem to indicate conditional JR or RET.

**********************************************************************/

#include "epson_lx800.h"
#include "lx800.lh"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EPSON_LX800 = &device_creator<epson_lx800_t>;


//-------------------------------------------------
//  ROM( lx800 )
//-------------------------------------------------

ROM_START( lx800 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("lx800.ic3c", 0x0000, 0x8000, CRC(da06c45b) SHA1(9618c940dd10d5b43cd1edd5763b90e6447de667) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *epson_lx800_t::device_rom_region() const
{
	return ROM_NAME( lx800 );
}


//-------------------------------------------------
//  ADDRESS_MAP( lx800_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( lx800_mem, AS_PROGRAM, 8, epson_lx800_t )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* 32k firmware */
	AM_RANGE(0x8000, 0x9fff) AM_RAM /* 8k external RAM */
	AM_RANGE(0xa000, 0xbfff) AM_NOP /* not used */
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x1ff8) AM_DEVREADWRITE("ic3b", e05a03_device, read, write)
	AM_RANGE(0xe000, 0xfeff) AM_NOP /* not used */
	AM_RANGE(0xff00, 0xffff) AM_RAM /* internal CPU RAM */
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( lx800_io )
//-------------------------------------------------

static ADDRESS_MAP_START( lx800_io, AS_IO, 8, epson_lx800_t )
	AM_RANGE(UPD7810_PORTA, UPD7810_PORTA) AM_READWRITE(porta_r, porta_w)
	AM_RANGE(UPD7810_PORTB, UPD7810_PORTB) AM_READ_PORT("DIPSW1")
	AM_RANGE(UPD7810_PORTC, UPD7810_PORTC) AM_READWRITE(portc_r, portc_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( epson_lx800 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( epson_lx800 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", UPD7810, XTAL_14_7456MHz)
	MCFG_CPU_PROGRAM_MAP(lx800_mem)
	MCFG_CPU_IO_MAP(lx800_io)
	MCFG_UPD7810_AN0(READLINE(epson_lx800_t, an0_r))
	MCFG_UPD7810_AN1(READLINE(epson_lx800_t, an1_r))
	MCFG_UPD7810_AN2(READLINE(epson_lx800_t, an2_r))
	MCFG_UPD7810_AN3(READLINE(epson_lx800_t, an3_r))
	MCFG_UPD7810_AN4(READLINE(epson_lx800_t, an4_r))
	MCFG_UPD7810_AN5(READLINE(epson_lx800_t, an5_r))

	MCFG_DEFAULT_LAYOUT(layout_lx800)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)

	/* gate array */
	MCFG_DEVICE_ADD("ic3b", E05A03, 0)
	MCFG_E05A03_PE_LP_CALLBACK(WRITELINE(epson_lx800_t, paperempty_led_w))
	MCFG_E05A03_RESO_CALLBACK(WRITELINE(epson_lx800_t, reset_w))
	MCFG_E05A03_PE_CALLBACK(WRITELINE(epson_lx800_t, centronics_pe_w))
	MCFG_E05A03_DATA_CALLBACK(READ8(epson_lx800_t, centronics_data_r))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor epson_lx800_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( epson_lx800 );
}


//-------------------------------------------------
//  INPUT_PORTS( epson_lx800 )
//-------------------------------------------------

INPUT_PORTS_START( epson_lx800 )
	PORT_START("ONLINE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_O)

	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_F)

	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_L)

	PORT_START("DIPSW1")
	PORT_DIPNAME(0x01, 0x00, "Typeface")
	PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(0x01, "Condensed")
	PORT_DIPSETTING(0x00, DEF_STR(Normal))
	PORT_DIPNAME(0x02, 0x00, "ZERO font")
	PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(0x02, "0")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPNAME(0x04, 0x00, "Character Table")
	PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(0x04, "Graphic")
	PORT_DIPSETTING(0x00, "Italic")
	PORT_DIPNAME(0x08, 0x00, "Paper-out detection")
	PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x08, "Valid")
	PORT_DIPSETTING(0x00, "Invalid")
	PORT_DIPNAME(0x10, 0x00, "Printing quality")
	PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x10, "NLQ")
	PORT_DIPSETTING(0x00, "Draft")
	PORT_DIPNAME(0xe0, 0xe0, "International character set")
	PORT_DIPLOCATION("DIP:3,2,1")
	PORT_DIPSETTING(0xe0, "U.S.A.")
	PORT_DIPSETTING(0x60, "France")
	PORT_DIPSETTING(0xa0, "Germany")
	PORT_DIPSETTING(0x20, "U.K.")
	PORT_DIPSETTING(0xc0, "Denmark")
	PORT_DIPSETTING(0x40, "Sweden")
	PORT_DIPSETTING(0x80, "Italy")
	PORT_DIPSETTING(0x00, "Spain")

	PORT_START("DIPSW2")
	PORT_DIPNAME(0x01, 0x00, "Page length")
	PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x01, "12\"")
	PORT_DIPSETTING(0x00, "11\"")
	PORT_DIPNAME(0x02, 0x00, "Cut sheet feeder mode")
	PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(0x02, "Valid")
	PORT_DIPSETTING(0x00, "Invalid")
	PORT_DIPNAME(0x04, 0x00, "1\" skip over perforation")
	PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x04, "Valid")
	PORT_DIPSETTING(0x00, "Invalid")
	PORT_DIPNAME(0x08, 0x00, "AUTO FEED XT control")
	PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x08, "Fix to LOW")
	PORT_DIPSETTING(0x00, "Depends on external signal")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_lx800_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_lx800 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_lx800_t - constructor
//-------------------------------------------------

epson_lx800_t::epson_lx800_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_LX800, "Epson LX-800", tag, owner, clock, "lx800", __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_beep(*this, "beeper")
{
}

epson_lx800_t::epson_lx800_t(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_beep(*this, "beeper")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_lx800_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_lx800_t::device_reset()
{
	m_beep->set_state(0);
	m_beep->set_frequency(4000); /* ? */
}


/***************************************************************************
    I/O PORTS
***************************************************************************/

/* PA0   W  CRCOM  carriage motor, 0 = holding voltage, 1 = drive voltage
 * PA1             not used
 * PA2   W  PFCOM  paper feed motor, 0 = holding voltage, 1 = drive voltage
 * PA3  R   LF SW  line feed switch
 * PA4  R   FF SW  form feed switch
 * PA5  R   PE SW  paper end sensor, 0 = no paper, 1 = paper
 * PA6             not used
 * PA7  R   P/S    P/S signal from the optional interface
 */
READ8_MEMBER( epson_lx800_t::porta_r )
{
	UINT8 result = 0;

	logerror("%s: lx800_porta_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("LINEFEED")->read() << 3;
	result |= ioport("FORMFEED")->read() << 4;
	result |= 1 << 5;

	result |= 1 << 7;

	return result;
}

WRITE8_MEMBER( epson_lx800_t::porta_w )
{
	logerror("%s: lx800_porta_w(%02x): %02x\n", machine().describe_context(), offset, data);
	logerror("--> carriage: %d, paper feed: %d\n", BIT(data, 0), BIT(data, 2));
}

/* PC0   W  TXD        serial i/o txd
 * PC1  R   RXD        serial i/o rxd
 * PC2   W  ONLINE LP  online led
 * PC3  R   ONLINE SW  online switch
 * PC4   W  ERR        centronics error
 * PC5   W  ACK        centronics acknowledge
 * PC6   W  FIRE       drive pulse width signal
 * PC7   W  BUZZER     buzzer signal
 */
READ8_MEMBER( epson_lx800_t::portc_r )
{
	UINT8 result = 0;

	logerror("%s: lx800_portc_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("ONLINE")->read() << 3;

	return result;
}

WRITE8_MEMBER( epson_lx800_t::portc_w )
{
	logerror("%s: lx800_portc_w(%02x): %02x\n", machine().describe_context(), offset, data);
	logerror("--> err: %d, ack: %d, fire: %d, buzzer: %d\n", BIT(data, 4), BIT(data, 5), BIT(data, 6), BIT(data, 7));

	machine().output().set_value("online_led", !BIT(data, 2));
	m_beep->set_state(!BIT(data, 7));
}

READ_LINE_MEMBER( epson_lx800_t::an0_r )
{
	return BIT(ioport("DIPSW2")->read(), 0);
}

READ_LINE_MEMBER( epson_lx800_t::an1_r )
{
	return BIT(ioport("DIPSW2")->read(), 1);
}

READ_LINE_MEMBER( epson_lx800_t::an2_r )
{
	return BIT(ioport("DIPSW2")->read(), 2);
}

READ_LINE_MEMBER( epson_lx800_t::an3_r )
{
	return BIT(ioport("DIPSW2")->read(), 3); // can also read an external line AUTO_FEED_XT
}

READ_LINE_MEMBER( epson_lx800_t::an4_r )
{
	return 0; // Printer select line (0=always selected)
}

READ_LINE_MEMBER( epson_lx800_t::an5_r )
{
	return 1; // Monitors 24v line, should return 4.08 volts
}


/***************************************************************************
    GATE ARRAY
***************************************************************************/

READ8_MEMBER( epson_lx800_t::centronics_data_r )
{
	logerror("centronics: data read\n");
	return 0x55;
}

WRITE_LINE_MEMBER( epson_lx800_t::centronics_pe_w )
{
	logerror("centronics: pe = %d\n", state);
}

WRITE_LINE_MEMBER( epson_lx800_t::paperempty_led_w )
{
	logerror("setting paperout led: %d\n", state);
	machine().output().set_value("paperout_led", state);
}

WRITE_LINE_MEMBER( epson_lx800_t::reset_w )
{
	logerror("cpu reset");
	m_maincpu->reset();
}
