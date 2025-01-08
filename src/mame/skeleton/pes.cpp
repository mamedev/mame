// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  Pacific Educational Systems 'PES' Speech box
*  Part number VPU-1 V1
*  By Kevin 'kevtris' Horton and Jonathan Gevaryahu AKA Lord Nightmare
*
*  Pacific Educational Systems, 915 Woodhall Drive, Victoria, Canada.
*
*  RE work done by Kevin Horton and Jonathan Gevaryahu
*
*  DONE:
*  compiles correctly
*  rom loads correctly
*  interface with tms5220 is done
*  rts and cts bits are stored in struct
*  serial is attached to terminal
*
*  TODO:
*  serial receive clear should happen after delay of one cpu cycle, not ASSERT and then CLEAR immediately after
*  figure out how to attach serial to external socket
*
***********************************************************************
This is almost as simple as hardware gets from the digital side:
Hardware consists of:
10.245Mhz xtal
80c31 cpu/mcu
27c64 rom (holds less than 256 bytes of code)
unpopulated 6164 sram, which isn't used
TSP5220C speech chip (aka tms5220c)
mc145406 RS232 driver/receiver (+-24v to 0-5v converter for serial i/o)
74hc573b1 octal tri-state D-latch (part of bus interface for ROM)
74hc74b1 dual d-flipflop with set/reset, positive edge trigger (?)
74hc02b1 quad 2-input NOR gate (wired up to decode address 0, and data 0 and 1 to produce /RS and /WS)
mc14520b dual 4-bit binary up counter (used as a chopper for the analog filter)
Big messy analog section to allow voice output to be filtered extensively by a 4th order filter

Address map:
80C31 ADDR:
  0000-1FFF: ROM
  2000-3FFF: open bus (would be ram)
  4000-ffff: open bus
80C31 IO:
  00 W: d0 and d1 are the /RS and /WS bits
  port 1.x: tms5220 bus
  port 2.x: unused
  port 3.0: RxD serial receive
  port 3.1: TxD serial send
  port 3.2: read, from /INT on tms5220c
  port 3.3: read, from /READY on tms5220c
  port 3.4: read, from the serial RTS line
  port 3.5: read/write, to the serial CTS line, inverted (effectively /CTS)
  port 3.6: write, /WR (general) and /WE (pin 27) for unpopulated 6164 SRAM
  port 3.7: write, /RD (general) and /OE (pin 22) for unpopulated 6164 SRAM


Current status:
- bios 0 : working. Hold down various letters for fragments of sound. g makes a beep. Status is reflected on the screen (eg space,@,`).
- bios 1 : working as above, however it does not get the tms5220 status and it does not write to the screen.

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/tms5220.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/rs232.h"
#include "speaker.h"


namespace {

class pes_state : public driver_device
{
public:
	pes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_serial(*this, "serial")
		, m_speech(*this, "tms5220")
	{ }

	void pes(machine_config &config);

private:

	u8 m_port3 = 0U;
	virtual void machine_reset() override ATTR_COLD;
	void port3_w(u8 data);
	u8 port3_r();
	void rx_w(int state);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
	required_device<i80c31_device> m_maincpu;
	required_device<rs232_port_device> m_serial;
	required_device<tms5220_device> m_speech;
};



/* Port Handlers */
void pes_state::port3_w(u8 data)
{
	// preserve RXD
	m_port3 = (m_port3 & 0x01) | (data & ~0x01);
#if 0
	logerror("port3 write: control data written: %02X; ", data);
	logerror("RXD: %d; ", BIT(data,0));
	logerror("TXD: %d; ", BIT(data,1));
	logerror("/INT: %d; ", BIT(data,2));
	logerror("/RDY: %d; ", BIT(data,3));
	logerror("RTS: %d; ", BIT(data,4));
	logerror("CTS: %d; ", BIT(data,5));
	logerror("WR: %d; ", BIT(data,6));
	logerror("RD: %d;\n", BIT(data,7));
#endif
	m_serial->write_txd(BIT(data, 1));
}

u8 pes_state::port3_r()
{
	uint8_t data = m_port3 & 0xE3; // return last written state with rts, /rdy and /int masked out
	// check rts state; if virtual fifo is nonzero, rts is set, otherwise it is cleared
	//data |= 0x10; // set RTS bit
	data |= (m_speech->intq_r()<<2);
	data |= (m_speech->readyq_r()<<3);
	return data;
}

void pes_state::rx_w(int state)
{
	if (state)
		m_port3 |= 1;
	else
		m_port3 &= ~1;
}


/* Reset */
void pes_state::machine_reset()
{
	m_port3 = 0; // reset the openbus state of port 3
	m_speech->reset(); // reset the 5220
}


/******************************************************************************
 Address Maps
******************************************************************************/

void pes_state::prg_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom(); /* 27C64 ROM */
	// map(0x2000, 0x3fff).ram(); /* 6164 8k SRAM, not populated */
}

void pes_state::io_map(address_map &map)
{
	map(0x0000, 0x0000).w(m_speech, FUNC(tms5220_device::combined_rsq_wsq_w)); // /WS(0) and /RS(1)
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( pes )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static void serial_devices(device_slot_interface &device)
{
	device.option_add("terminal", SERIAL_TERMINAL);
}

void pes_state::pes(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, XTAL(10'245'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pes_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &pes_state::io_map);
	m_maincpu->port_in_cb<1>().set(m_speech, FUNC(tms5220_device::status_r));
	m_maincpu->port_out_cb<1>().set(m_speech, FUNC(tms5220_device::data_w));
	m_maincpu->port_in_cb<3>().set(FUNC(pes_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(pes_state::port3_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TMS5220C(config, m_speech, 720000); /* 720Khz clock, 9khz sample-rate, adjustable with 10-turn trimpot */
	m_speech->add_route(ALL_OUTPUTS, "mono", 1.0);

	RS232_PORT(config, m_serial, serial_devices, "terminal");
	m_serial->rxd_handler().set(FUNC(pes_state::rx_w));
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( pes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("kevbios")
	ROM_SYSTEM_BIOS( 0, "orig", "PES box with original firmware v2.5")
	ROMX_LOAD( "vpu_2-5.bin",   0x0000, 0x2000, CRC(b27cfdf7) SHA1(c52acf9c080823de5ef26ac55abe168ad53a7d38), ROM_BIOS(0)) // original firmware, rather buggy, 4800bps serial, buggy RTS/CTS flow control, no buffer
	ROM_SYSTEM_BIOS( 1, "kevbios", "PES box with kevtris' rewritten firmware")
	ROMX_LOAD( "pes.bin",   0x0000, 0x2000, CRC(22c1c4ec) SHA1(042e139cd0cf6ffafcd88904f1636c6fa1b38f25), ROM_BIOS(1)) // rewritten firmware by kevtris, 4800bps serial, RTS/CTS plus XON/XOFF flow control, 64 byte buffer
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME             FLAGS
COMP( 1987, pes,  0,      0,      pes,     pes,   pes_state, empty_init, "Pacific Educational Systems", "VPU-01 Speech box", 0 )
