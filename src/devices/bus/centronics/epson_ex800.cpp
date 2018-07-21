// license:GPL-2.0+
// copyright-holders:Dirk Best
/******************************************************************************

    Epson EX-800 Dot Matrix printer


 --

 Main CPU is a UPD7810H6 running at 12 MHz.

 --

 On startup, the ports are initialized as follows:

 Port A lines PA0-PA7 are configured as output
 Port B lines PB0-PB7 are configured as output
 Port C lines PC2, PC4 and PC5 are configured as output
        the other lines are control lines
        PC0: TxD output
        PC1: RxD input
        PC3: TI input
        PC6: CO0 output
        PC7: CO1 output

The I/O lines are connected as follows:

PA0: \
PA1:  Carriage motor
PA2:  driving pulses
PA3: /
PA4: High when carriage assembly reaches either end
PA5: High when carriage assembly reaches either end
PA6: Bank 0
PA7: Bank 1

PB0: Line feed motor driving pulses
PB1: Line feed motor driving pulses
PB2: Line feed motor current control
PB3: LED09, Online
PB4: (used for serial port)
PB5: ERR signal
PB6: ACKNLG signal, data received at printer
PB7: LED12, Paper empty

PC0: TxD output
PC1: RxD input
PC2:
PC3: TI input
PC4:
PC5:
PC6: PWD signal of 5a
PC7: Buzzer

PD0-7: Data bus D0-D7

PF0-7: Address bus A8-A15

AN0: DSW 2
AN1: DSW 1
AN2:
AN3:
AN4: Print head operating temperature sensor
AN5: Scanner Unit, Color Home sensor
AN6:
AN7: Color Home sensor adjust

INT: Online switch


Gate array 5a (parallel port, printer head)

IN0-IN7: DATA1 to DATA8 from parallel port
HD1-HD9: Head driving pulses
STROBE: STROBE signal
PWD: PC6 of CPU
ALE: ALE of CPU

Gate array 7a (inputs)

- activated when carriage assembly reaches either end?

PA0: Draft switch
PA1: NLQ Roman switch
PA2: Pica switch
PA3: NLQ sans-serif switch
PA4: Elite switch
PA5: Proportional switch
PA6: Normal switch
PA7: Condensed switch

PB0: Form feed switch
PB1: Line feed switch
PB2: Autoload switch
PB3: Low when paper empty
PB4: Low when carriage assembly is in home position
PB5: SLCTIN (connected to parallel port), disable with DSW2-1
PB6: AUTO FEEDXT (connected to parallel port), disable with DSW2-4
       "When this signal is LOW, the paper is automatically fed 1 line after
        printing."
PB7:

PC0: Dipswitch SW1-1 and SW1-2, Draft LED
PC1: Dipswitch SW1-3 and SW1-4, Roman LED
PC2: Dipswitch SW1-5 and SW1-6, Pica LED
PC3: Dipswitch SW1-7 and SW1-8, Sans-Serif LED
PC4: Dipswitch SW2-2 and SW2-3, Elite LED
PC5: Dipswitch SW2-5 and SW2-6, Proportional LED
PC6: Normal LED
PC7: Condensed LED

--

LED state on startup:

The power light comes on, then draft, pica and normal. If there is no paper,
the paper empty LED comes on.

Power LED is green, selectype LED is orange, paper out LED is red.

--

TODO:  - The UPD7810 core is missing analog port emulation
       - Figure out the gate arrays (using trojan code?)
       - (much later) write an interface so that other drivers can hook
         into this one and use to print

******************************************************************************/

#include "emu.h"
#include "epson_ex800.h"
#include "speaker.h"
#include "ex800.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PA0 (data & 0x01)
#define PA1 (data & 0x02)
#define PA2 (data & 0x04)
#define PA3 (data & 0x08)
#define PA4 (data & 0x10)
#define PA5 (data & 0x20)
#define PA6 (data & 0x40)
#define PA7 (data & 0x80)

#define PB0 (data & 0x01)
#define PB1 (data & 0x02)
#define PB2 (data & 0x04)
#define PB3 (data & 0x08)
#define PB4 (data & 0x10)
#define PB5 (data & 0x20)
#define PB6 (data & 0x40)
#define PB7 (data & 0x80)

#define PC0 (data & 0x01)
#define PC1 (data & 0x02)
#define PC2 (data & 0x04)
#define PC3 (data & 0x08)
#define PC4 (data & 0x10)
#define PC5 (data & 0x20)
#define PC6 (data & 0x40)
#define PC7 (data & 0x80)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_EX800, epson_ex800_device, "ex800", "Epson EX-800")


//-------------------------------------------------
//  ROM( ex800 )
//-------------------------------------------------

ROM_START( ex800 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("w8_pe9.9b", 0x0000, 0x8000, CRC(6dd41e9b) SHA1(8e30ead727b9317154742efd881206e9f9bbf95b))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_ex800_device::device_rom_region() const
{
	return ROM_NAME( ex800 );
}


//-------------------------------------------------
//  ADDRESS_MAP( ex800_mem )
//-------------------------------------------------

void epson_ex800_device::ex800_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).ram(); /* external RAM */
	map(0xc000, 0xc7ff).mirror(0x1800).rw(FUNC(epson_ex800_device::devsel_r), FUNC(epson_ex800_device::devsel_w));
	map(0xe000, 0xe7ff).rw(FUNC(epson_ex800_device::gate5a_r), FUNC(epson_ex800_device::gate5a_w));
	map(0xe800, 0xefff).rw(FUNC(epson_ex800_device::iosel_r), FUNC(epson_ex800_device::iosel_w));
	map(0xf000, 0xf001).mirror(0x07fc).r(FUNC(epson_ex800_device::gate7a_r));
	map(0xf002, 0xf003).mirror(0x07fc).w(FUNC(epson_ex800_device::gate7a_w));
	map(0xf800, 0xfeff).noprw(); /* not connected */
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_ex800_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	upd7810_device &upd(UPD7810(config, m_maincpu, 12000000)); // 12 MHz?
	upd.set_addrmap(AS_PROGRAM, &epson_ex800_device::ex800_mem);
	upd.pa_in_cb().set(FUNC(epson_ex800_device::porta_r));
	upd.pa_out_cb().set(FUNC(epson_ex800_device::porta_w));
	upd.pb_in_cb().set(FUNC(epson_ex800_device::portb_r));
	upd.pb_out_cb().set(FUNC(epson_ex800_device::portb_w));
	upd.pc_in_cb().set(FUNC(epson_ex800_device::portc_r));
	upd.pc_out_cb().set(FUNC(epson_ex800_device::portc_w));

	config.set_default_layout(layout_ex800);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 4000); // measured at 4000 Hz */
	m_beeper->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/* The ON LINE switch is directly connected to the INT1 input of the CPU */
INPUT_CHANGED_MEMBER(epson_ex800_device::online_switch)
{
	if (newval)
	{
		m_maincpu->set_input_line(UPD7810_INTF1, m_irq_state);
		m_irq_state = (m_irq_state == ASSERT_LINE) ? CLEAR_LINE : ASSERT_LINE;
	}
}


//-------------------------------------------------
//  INPUT_PORTS( epson_ex800 )
//-------------------------------------------------

INPUT_PORTS_START( epson_ex800 )
	PORT_START("ONLISW")
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON LINE")   PORT_CODE(KEYCODE_F9) PORT_CHANGED_MEMBER(DEVICE_SELF, epson_ex800_device, online_switch, nullptr)

	PORT_START("FEED")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FORM FEED") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_F11)

	PORT_START("SelecType")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Draft")          PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NLQ Roman")      PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pica")           PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NLQ Sans Serif") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Elite")          PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Proportional")   PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(DEF_STR(Normal))  PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Condensed")      PORT_CODE(KEYCODE_F1)

	PORT_START("DSW_1")
	PORT_DIPNAME(0x01, 0x00, "Condensed characters")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Slashed zero")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Character table")
	PORT_DIPSETTING(   0x00, "Italics")
	PORT_DIPSETTING(   0x04, "Graphics")
	PORT_DIPNAME(0x08, 0x00, "Printer commands")
	PORT_DIPSETTING(   0x00, "ESC/P")
	PORT_DIPSETTING(   0x08, "IBM printer emulation")
	PORT_DIPNAME(0x10, 0x00, "Print quality")
	PORT_DIPSETTING(   0x00, "Draft")
	PORT_DIPSETTING(   0x10, "NLQ")
	PORT_DIPNAME(0xe0, 0x00, "Int. character set")
	PORT_DIPSETTING(   0x00, DEF_STR(USA))
	PORT_DIPSETTING(   0x20, DEF_STR(French))
	PORT_DIPSETTING(   0x30, DEF_STR(German))
	PORT_DIPSETTING(   0x40, "UK")
	PORT_DIPSETTING(   0x50, "Danish")
	PORT_DIPSETTING(   0x60, "Swedish")
	PORT_DIPSETTING(   0x70, DEF_STR(Italian))
	PORT_DIPSETTING(   0x80, DEF_STR(Spanish))

	PORT_START("DSW_2")
	PORT_DIPNAME(0x01, 0x00, "Page length")
	PORT_DIPSETTING(   0x00, "11 inch")
	PORT_DIPSETTING(   0x01, "12 inch")
	PORT_DIPNAME(0x02, 0x00, "Auto. sheet feeder")
	PORT_DIPSETTING(   0x00, "Canceled")
	PORT_DIPSETTING(   0x02, "Selected")
	PORT_DIPNAME(0x04, 0x00, "Skip-over-perforation")
	PORT_DIPSETTING(   0x00, DEF_STR(None))
	PORT_DIPSETTING(   0x04, "1 inch")
	PORT_DIPNAME(0x08, 0x00, "Add LF after CR")
	PORT_DIPSETTING(   0x00, "CR only")
	PORT_DIPSETTING(   0x08, "CR + LF")
	PORT_DIPNAME(0x30, 0x00, "Interface type")
	PORT_DIPSETTING(   0x00, "Parallel")
	PORT_DIPSETTING(   0x10, "Serial (odd parity)")
	PORT_DIPSETTING(   0x20, "Serial (even parity)")
	PORT_DIPSETTING(   0x30, "Serial (no parity)")
	PORT_DIPNAME(0xc0, 0x00, "Baud rate")
	PORT_DIPSETTING(   0x00, "9600")
	PORT_DIPSETTING(   0x40, "4800")
	PORT_DIPSETTING(   0x80, "1200")
	PORT_DIPSETTING(   0xc0, "300")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_ex800_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_ex800 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_ex800_device - constructor
//-------------------------------------------------

epson_ex800_device::epson_ex800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_EX800, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_beeper(*this, "beeper"), m_irq_state(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_ex800_device::device_start()
{
	m_irq_state = ASSERT_LINE;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_ex800_device::device_reset()
{
	/* Setup beep */
	m_beeper->set_state(0);
}


READ8_MEMBER(epson_ex800_device::porta_r)
{
	logerror("PA R %s\n", machine().describe_context());
	return machine().rand();
}

READ8_MEMBER(epson_ex800_device::portb_r)
{
	logerror("PB R %s\n", machine().describe_context());
	return machine().rand();
}

READ8_MEMBER(epson_ex800_device::portc_r)
{
	logerror("PC R %s\n", machine().describe_context());
	return machine().rand();
}

WRITE8_MEMBER(epson_ex800_device::porta_w)
{
	if (PA6) logerror("BNK0 selected.\n");
	if (PA7) logerror("BNK1 selected.\n");

	logerror("PA W %x %s\n", data, machine().describe_context());
}

WRITE8_MEMBER(epson_ex800_device::portb_w)
{
	if (data & 3)
		logerror("PB0/1 Line feed %s\n", machine().describe_context());
	if (!(data & 4))
		logerror("PB2 Line feed %s\n", machine().describe_context());
	if (data & 8)
		logerror("PB3 Online LED on %s\n", machine().describe_context());
	else
		logerror("PB3 Online LED off %s\n", machine().describe_context());
	if (data & 16)
		logerror("PB4 Serial %s\n", machine().describe_context());
	if (data & 32)
		logerror("PB4 Serial %s\n", machine().describe_context());
	if (data & 64)
		logerror("PB4 Serial %s\n", machine().describe_context());
	if (data & 128)
		logerror("PB3 Paper empty LED on %s\n", machine().describe_context());
	else
		logerror("PB3 Paper empty LED off %s\n", machine().describe_context());

//  logerror("PB W %x %s\n", data, machine().describe_context());
}

WRITE8_MEMBER(epson_ex800_device::portc_w)
{
	if (data & 0x80)
		m_beeper->set_state(0);
	else
		m_beeper->set_state(1);

	logerror("PC W %x %s\n", data, machine().describe_context());
}


/* Memory mapped I/O access */

READ8_MEMBER(epson_ex800_device::devsel_r)
{
	logerror("DEVSEL R %s with offset %x\n", machine().describe_context(), offset);
	return machine().rand();
}

WRITE8_MEMBER(epson_ex800_device::devsel_w)
{
	logerror("DEVSEL W %x %s with offset %x\n", data, machine().describe_context(), offset);
}

READ8_MEMBER(epson_ex800_device::gate5a_r)
{
	logerror("GATE5A R %s with offset %x\n", machine().describe_context(), offset);
	return machine().rand();
}

WRITE8_MEMBER(epson_ex800_device::gate5a_w)
{
	logerror("GATE5A W %x %s with offset %x\n", data, machine().describe_context(), offset);
}

READ8_MEMBER(epson_ex800_device::iosel_r)
{
	logerror("IOSEL R %s with offset %x\n", machine().describe_context(), offset);
	return machine().rand();
}

WRITE8_MEMBER(epson_ex800_device::iosel_w)
{
	logerror("IOSEL W %x %s with offset %x\n", data, machine().describe_context(), offset);
}

READ8_MEMBER(epson_ex800_device::gate7a_r)
{
	logerror("GATE7A R %s with offset %x\n", machine().describe_context(), offset);
	return machine().rand();
}

WRITE8_MEMBER(epson_ex800_device::gate7a_w)
{
	logerror("GATE7A W %x %s with offset %x\n", data, machine().describe_context(), offset);
}
