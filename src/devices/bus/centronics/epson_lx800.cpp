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

#include "emu.h"
#include "epson_lx800.h"
#include "speaker.h"
#include "lx800.lh"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_LX800, epson_lx800_device, "lx800", "Epson LX-800")


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

const tiny_rom_entry *epson_lx800_device::device_rom_region() const
{
	return ROM_NAME( lx800 );
}


//-------------------------------------------------
//  ADDRESS_MAP( lx800_mem )
//-------------------------------------------------

void epson_lx800_device::lx800_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom(); /* 32k firmware */
	map(0x8000, 0x9fff).ram(); /* 8k external RAM */
	map(0xa000, 0xbfff).noprw(); /* not used */
	map(0xc000, 0xc007).mirror(0x1ff8).rw("ic3b", FUNC(e05a03_device::read), FUNC(e05a03_device::write));
	map(0xe000, 0xfeff).noprw(); /* not used */
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_lx800_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	upd7810_device &upd(UPD7810(config, m_maincpu, 14.7456_MHz_XTAL));
	upd.set_addrmap(AS_PROGRAM, &epson_lx800_device::lx800_mem);
	upd.pa_in_cb().set(FUNC(epson_lx800_device::porta_r));
	upd.pa_out_cb().set(FUNC(epson_lx800_device::porta_w));
	upd.pb_in_cb().set_ioport("DIPSW1");
	upd.pc_in_cb().set(FUNC(epson_lx800_device::portc_r));
	upd.pc_out_cb().set(FUNC(epson_lx800_device::portc_w));
	upd.an0_func().set(FUNC(epson_lx800_device::an0_r));
	upd.an1_func().set(FUNC(epson_lx800_device::an1_r));
	upd.an2_func().set(FUNC(epson_lx800_device::an2_r));
	upd.an3_func().set(FUNC(epson_lx800_device::an3_r));
	upd.an4_func().set(FUNC(epson_lx800_device::an4_r));
	upd.an5_func().set(FUNC(epson_lx800_device::an5_r));

	config.set_default_layout(layout_lx800);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 4000); // ?
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* gate array */
	e05a03_device &ic3b(E05A03(config, "ic3b", 0));
	ic3b.pe_lp_wr_callback().set(FUNC(epson_lx800_device::paperempty_led_w));
	ic3b.reso_wr_callback().set(FUNC(epson_lx800_device::reset_w));
	ic3b.pe_wr_callback().set(FUNC(epson_lx800_device::centronics_pe_w));
	ic3b.data_rd_callback().set(FUNC(epson_lx800_device::centronics_data_r));
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

ioport_constructor epson_lx800_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_lx800 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_lx800_device - constructor
//-------------------------------------------------

epson_lx800_device::epson_lx800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	epson_lx800_device(mconfig, EPSON_LX800, tag, owner, clock)
{
}

epson_lx800_device::epson_lx800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_beep(*this, "beeper")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_lx800_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_lx800_device::device_reset()
{
	m_beep->set_state(0);
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
READ8_MEMBER( epson_lx800_device::porta_r )
{
	uint8_t result = 0;

	logerror("%s: lx800_porta_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("LINEFEED")->read() << 3;
	result |= ioport("FORMFEED")->read() << 4;
	result |= 1 << 5;

	result |= 1 << 7;

	return result;
}

WRITE8_MEMBER( epson_lx800_device::porta_w )
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
READ8_MEMBER( epson_lx800_device::portc_r )
{
	uint8_t result = 0;

	logerror("%s: lx800_portc_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("ONLINE")->read() << 3;

	return result;
}

WRITE8_MEMBER( epson_lx800_device::portc_w )
{
	logerror("%s: lx800_portc_w(%02x): %02x\n", machine().describe_context(), offset, data);
	logerror("--> err: %d, ack: %d, fire: %d, buzzer: %d\n", BIT(data, 4), BIT(data, 5), BIT(data, 6), BIT(data, 7));

	machine().output().set_value("online_led", !BIT(data, 2));
	m_beep->set_state(!BIT(data, 7));
}

READ_LINE_MEMBER( epson_lx800_device::an0_r )
{
	return BIT(ioport("DIPSW2")->read(), 0);
}

READ_LINE_MEMBER( epson_lx800_device::an1_r )
{
	return BIT(ioport("DIPSW2")->read(), 1);
}

READ_LINE_MEMBER( epson_lx800_device::an2_r )
{
	return BIT(ioport("DIPSW2")->read(), 2);
}

READ_LINE_MEMBER( epson_lx800_device::an3_r )
{
	return BIT(ioport("DIPSW2")->read(), 3); // can also read an external line AUTO_FEED_XT
}

READ_LINE_MEMBER( epson_lx800_device::an4_r )
{
	return 0; // Printer select line (0=always selected)
}

READ_LINE_MEMBER( epson_lx800_device::an5_r )
{
	return 1; // Monitors 24v line, should return 4.08 volts
}


/***************************************************************************
    GATE ARRAY
***************************************************************************/

READ8_MEMBER( epson_lx800_device::centronics_data_r )
{
	logerror("centronics: data read\n");
	return 0x55;
}

WRITE_LINE_MEMBER( epson_lx800_device::centronics_pe_w )
{
	logerror("centronics: pe = %d\n", state);
}

WRITE_LINE_MEMBER( epson_lx800_device::paperempty_led_w )
{
	logerror("setting paperout led: %d\n", state);
	machine().output().set_value("paperout_led", state);
}

WRITE_LINE_MEMBER( epson_lx800_device::reset_w )
{
	logerror("cpu reset");
	m_maincpu->reset();
}
