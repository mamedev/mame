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


class seattle_comp_state : public driver_device
{
public:
	seattle_comp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic%u", 1)
	{ }

	DECLARE_READ8_MEMBER(pic_slave_ack);

	void seattle(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device_array<pic8259_device, 2> m_pic;
};


READ8_MEMBER(seattle_comp_state::pic_slave_ack)
{
	if (offset == 1)
		return m_pic[1]->acknowledge();

	return 0;
}


void seattle_comp_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xff7ff).ram();
	map(0xff800, 0xfffff).rom().region("user1", 0);
}

void seattle_comp_state::io_map(address_map &map)
{
	//ADDRESS_MAP_UNMAP_HIGH
	map.global_mask(0xff);
	map(0xf0, 0xf1).rw("pic1", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf2, 0xf3).rw("pic2", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf4, 0xf5).rw("stc", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0xf6, 0xf6).rw("uart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf7, 0xf7).rw("uart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	//AM_RANGE(0xfc, 0xfd) Parallel data, status, serial DCD
	//AM_RANGE(0xfe, 0xff) Eprom disable bit, read sense switches (bank of 8 dipswitches)
}

/* Input ports */
static INPUT_PORTS_START( seattle )
INPUT_PORTS_END


// bit 7 needs to be stripped off, we do this by choosing 7 bits and even parity
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(seattle_comp_state::seattle)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8086, XTAL(24'000'000) / 3) // 8 MHz or 4 MHz selectable
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("pic1", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pic1", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_INT0))
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(*this, seattle_comp_state, pic_slave_ack))

	MCFG_DEVICE_ADD("pic2", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(WRITELINE("pic1", pic8259_device, ir1_w))

	MCFG_DEVICE_ADD("stc", AM9513, XTAL(4'000'000)) // dedicated XTAL
	MCFG_AM9513_OUT2_CALLBACK(WRITELINE("pic2", pic8259_device, ir0_w))
	MCFG_AM9513_OUT3_CALLBACK(WRITELINE("pic2", pic8259_device, ir4_w))
	MCFG_AM9513_OUT4_CALLBACK(WRITELINE("pic2", pic8259_device, ir7_w))
	MCFG_AM9513_OUT5_CALLBACK(WRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart", i8251_device, write_rxc))
	MCFG_AM9513_FOUT_CALLBACK(WRITELINE("stc", am9513_device, source1_w))
	// FOUT not shown on schematics, which inexplicably have Source 1 tied to Gate 5

	MCFG_DEVICE_ADD("uart", I8251, XTAL(24'000'000) / 12) // CLOCK on line 49
	MCFG_I8251_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE("pic2", pic8259_device, ir1_w))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE("pic2", pic8259_device, ir5_w))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart", i8251_device, write_cts))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", terminal) // must be exactly here
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( scp300f )
	ROM_REGION( 0x800, "user1", 0 )
	ROM_LOAD( "mon86 v1.5tdd", 0x0000, 0x0800, CRC(7db23169) SHA1(c791b02ca33a4e1f8e95eb541624a59738f378c4))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS               INIT        COMPANY            FULLNAME    FLAGS
COMP( 1986, scp300f, 0,      0,      seattle, seattle, seattle_comp_state, empty_init, "Seattle Computer", "SCP-300F", MACHINE_NO_SOUND_HW )
