// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Seattle Computer SCP-300F S100 card. It has sockets on the card for
one serial and 2 parallel connections.

2013-08-14 Skeleton driver.

When started you must press Enter twice before anything happens.

All commands must be in UPPER case.

Known Commands:
B : Boot from disk?
D : Dump memory
E : Edit memory
F : Find
G : Go?
I : Input port
M : Move
O : Output port
R : Display / Modify Registers
S : Search
T : Trace

Chips on the board: 8259 x2; AM9513; 8251; 2716 ROM (MON-86 V1.5TDD)
There is a 4MHz crystal connected to the 9513.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/am9513.h"
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


namespace {

class seattle_comp_state : public driver_device
{
public:
	seattle_comp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic%u", 1U)
		, m_monitor(*this, "monitor")
	{ }

	void seattle(machine_config &config);

private:
	u8 pic_slave_ack(offs_t offset);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<pic8259_device, 2> m_pic;
	required_region_ptr<u8> m_monitor;
};


u8 seattle_comp_state::pic_slave_ack(offs_t offset)
{
	if (offset == 1)
		return m_pic[1]->acknowledge();

	return 0;
}


void seattle_comp_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xff7ff).ram();
	map(0xff800, 0xfffff).lr8([this](offs_t offset) { return m_monitor[offset]; }, "monitor_r");
}

void seattle_comp_state::io_map(address_map &map)
{
	//map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf1).rw("pic1", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf2, 0xf3).rw("pic2", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf4, 0xf5).rw("stc", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0xf6, 0xf7).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	//map(0xfc, 0xfd) Parallel data, status, serial DCD
	//map(0xfe, 0xff) Eprom disable bit, read sense switches (bank of 8 dipswitches)
}

/* Input ports */
static INPUT_PORTS_START( seattle )
INPUT_PORTS_END


// bit 7 needs to be stripped off, we do this by choosing 7 bits and even parity
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void seattle_comp_state::seattle(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 24_MHz_XTAL / 3); // 8 MHz or 4 MHz selectable
	m_maincpu->set_addrmap(AS_PROGRAM, &seattle_comp_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &seattle_comp_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic1", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic[0]);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_INT0);
	m_pic[0]->read_slave_ack_callback().set(FUNC(seattle_comp_state::pic_slave_ack));

	PIC8259(config, m_pic[1]);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir1_w));

	am9513_device &stc(AM9513(config, "stc", 4_MHz_XTAL)); // dedicated XTAL
	stc.out2_cb().set(m_pic[1], FUNC(pic8259_device::ir0_w));
	stc.out3_cb().set(m_pic[1], FUNC(pic8259_device::ir4_w));
	stc.out4_cb().set(m_pic[1], FUNC(pic8259_device::ir7_w));
	stc.out5_cb().set("uart", FUNC(i8251_device::write_txc));
	stc.out5_cb().append("uart", FUNC(i8251_device::write_rxc));
	stc.fout_cb().set("stc", FUNC(am9513_device::source1_w));
	// FOUT not shown on schematics, which inexplicably have Source 1 tied to Gate 5

	i8251_device &uart(I8251(config, "uart", 24_MHz_XTAL / 12)); // CLOCK on line 49
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.rxrdy_handler().set("pic2", FUNC(pic8259_device::ir1_w));
	uart.txrdy_handler().set("pic2", FUNC(pic8259_device::ir5_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

/* ROM definition */
ROM_START( scp300f )
	ROM_REGION( 0x800, "monitor", 0 )
	ROM_LOAD( "mon86 v1.5tdd", 0x0000, 0x0800, CRC(7db23169) SHA1(c791b02ca33a4e1f8e95eb541624a59738f378c4))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS               INIT        COMPANY            FULLNAME    FLAGS
COMP( 1986, scp300f, 0,      0,      seattle, seattle, seattle_comp_state, empty_init, "Seattle Computer", "SCP-300F", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
