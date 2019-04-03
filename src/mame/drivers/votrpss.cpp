// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Robbbert
/******************************************************************************
*
*  Votrax Personal Speech System Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with help from Kevin 'kevtris' Horton
*  Special thanks to Professor Nicholas Gessler for loaning several PSS units
*
*  The votrax PSS was sold from around 35th week of 1982 until october, 1990

Main xtal is 8MHz
AY-3-8910 and i8253 clock is running at 2 MHz (xtal/4)
Z80A is runing at 4MHz (xtal/2)
clock dividers also generate the system reset signals for the 8251 and and a periodic IRQ at 122Hz (xtal/65536)

I8253:
Timer0 = Baud Clock, not gated (constantly on)
Timer1 = output to transistor chopper on clock input to sc-01-a to control pitch; gated by xtal/256
Timer2 = output to transistor chopper on output of sc-01-a to control volume; gated by xtal/4096

I8255 ports:
PortA 0:7 = pins 16 thru 23 of parallel port
PortB 0:7 = pins 6 thru 13 of parallel port
PortC =
    0 = NC
    1 = GND
    2 = pin 5 of parallel port
    3 = /RXINTEN
    4 = pin 15 of parallel port
    5 = pin 14 of parallel port through inverter
    6 = ay-3-8910 enable (which pin? BC1?)
    7 = input from parallel port pin 4 through inverter

AY-3-8910 I/O ports:
    IOA is in output mode
        IOA0-A5 = phoneme #
        IOA6 = strobe (SC-01)
        IOA7 = vochord control, 0 = off, 1 = on
    IOB is in input mode
        IOB0-IOB7 = dip switches

I8251 UART:
    RESET is taken from the same inverter that resets the counters

Things to be looked at:
- volume and pitch should be controlled by ppi outputs
- pit to be hooked up
- bit 0 of portc is not connected according to text above, but it
  completely changes the irq operation.

Notes:
- When Serial dip is chosen, you type the commands in, but you cannot see anything.
  If you enter text via parallel, it is echoed but otherwise ignored.
- When Parallel dip is chosen, you type the commands in, but again you cannot
  see anything.
- These operations ARE BY DESIGN. Everything is working correctly.
- Commands are case-sensitive.
- Some tests...
  - Say the time: EscT. (include the period)
  - Play some notes: !T08:1234:125:129:125:130. (then press enter)

******************************************************************************/

/* Core includes */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
//#include "votrpss.lh"

/* Components */
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/votrax.h"
#include "machine/terminal.h"
#include "speaker.h"

class votrpss_state : public driver_device
{
public:
	votrpss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_ppi(*this, "ppi")
	{ }

	void votrpss(machine_config &config);

private:
	void kbd_put(u8 data);
	DECLARE_READ8_MEMBER(ppi_pa_r);
	DECLARE_READ8_MEMBER(ppi_pb_r);
	DECLARE_READ8_MEMBER(ppi_pc_r);
	DECLARE_WRITE8_MEMBER(ppi_pa_w);
	DECLARE_WRITE8_MEMBER(ppi_pb_w);
	DECLARE_WRITE8_MEMBER(ppi_pc_w);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	IRQ_CALLBACK_MEMBER(irq_ack);

	void votrpss_io(address_map &map);
	void votrpss_mem(address_map &map);

	uint8_t m_term_data;
	uint8_t m_porta;
	uint8_t m_portb;
	uint8_t m_portc;
	virtual void machine_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8255_device> m_ppi;
};


/******************************************************************************
 Address Maps
******************************************************************************/

void votrpss_state::votrpss_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom(); /* main roms (in potted module) */
	map(0x4000, 0x7fff).noprw(); /* open bus/space for expansion rom (reads as 0xFF) */
	map(0x8000, 0x8fff).ram(); /* onboard memory (2x 6116) */
	map(0x9000, 0xbfff).noprw(); /* open bus (space for memory expansion, checked by main roms, will be used if found)*/
	map(0xc000, 0xdfff).rom(); /* 'personality rom', containing self-test code and optional user code */
	map(0xe000, 0xffff).noprw(); /* open bus (space for more personality rom, not normally used) */
}

void votrpss_state::votrpss_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x3c).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).mirror(0x3e).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x80, 0x83).mirror(0x3c).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xc0, 0xc0).mirror(0x3e).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0xc1, 0xc1).mirror(0x3e).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(votrpss)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Baud Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "150" )
	PORT_DIPSETTING(    0x07, "75" )
	PORT_DIPNAME( 0x08, 0x00, "Serial Handshaking" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "RTS/CTS" )
	PORT_DIPSETTING(    0x08, "XON/XOFF" )
	PORT_DIPNAME( 0x10, 0x00, "Parity bit behavior" )   PORT_DIPLOCATION("SW1:5") /* note: only firmware 3.C (1984?) and up handle this bit; on earlier firmwares, its function is 'unused' */
	PORT_DIPSETTING(    0x00, "Bit 8 ignored/zeroed" )
	PORT_DIPSETTING(    0x10, "Bit 8 treated as data" )
	PORT_DIPNAME( 0x20, 0x20, "Startup Message" )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Default Input Port" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Serial/RS-232" )
	PORT_DIPSETTING(    0x40, "Parallel" )
	PORT_DIPNAME( 0x80, 0x00, "Self Test Mode" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR ( On )  )
INPUT_PORTS_END

void votrpss_state::machine_start()
{
}

TIMER_DEVICE_CALLBACK_MEMBER( votrpss_state::irq_timer )
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER( votrpss_state::irq_ack )
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x38;
}

READ8_MEMBER( votrpss_state::ppi_pa_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( votrpss_state::ppi_pb_r )
{
	return m_portb;
}

// Bit 0 controls what happens at interrupt time. See code around 518.
READ8_MEMBER( votrpss_state::ppi_pc_r )
{
	uint8_t data = 0;

	if (m_term_data)
	{
		m_ppi->pc4_w(0); // send a strobe pulse
		data |= 0x20;
	}

	return (m_portc & 0xdb) | data;
}

WRITE8_MEMBER( votrpss_state::ppi_pa_w )
{
	m_porta = data;
}

WRITE8_MEMBER( votrpss_state::ppi_pb_w )
{
	m_portb = data;
	m_terminal->write(data&0x7f);
}

WRITE8_MEMBER( votrpss_state::ppi_pc_w )
{
	m_portc = data;
}

void votrpss_state::kbd_put(u8 data)
{
	m_term_data = data;
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

void votrpss_state::votrpss(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000)/2);  /* 4.000 MHz, verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &votrpss_state::votrpss_mem);
	m_maincpu->set_addrmap(AS_IO, &votrpss_state::votrpss_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(votrpss_state::irq_ack));

	/* video hardware */
	//config.set_default_layout(layout_votrpss);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay", XTAL(8'000'000)/4)); /* 2.000 MHz, verified */
	ay.port_b_read_callback().set_ioport("DSW1");
	ay.port_a_write_callback().set("votrax", FUNC(votrax_sc01_device::write));
	ay.add_route(ALL_OUTPUTS, "mono", 0.25);
	VOTRAX_SC01(config, "votrax", 720000).add_route(ALL_OUTPUTS, "mono", 1.00); /* 720 kHz? needs verify */

	/* Devices */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(votrpss_state::kbd_put));

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	// when serial is chosen, and you select terminal, nothing shows (by design). You can only type commands in.
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(8_MHz_XTAL); // Timer 0: baud rate gen for 8251
	pit.out_handler<0>().set("uart", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("uart", FUNC(i8251_device::write_rxc));
	pit.set_clk<1>(8_MHz_XTAL / 256); // Timer 1: Pitch
	pit.set_clk<2>(8_MHz_XTAL / 4096); // Timer 2: Volume

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(votrpss_state::ppi_pa_r));
	m_ppi->in_pb_callback().set(FUNC(votrpss_state::ppi_pb_r));
	m_ppi->in_pc_callback().set(FUNC(votrpss_state::ppi_pc_r));
	m_ppi->out_pa_callback().set(FUNC(votrpss_state::ppi_pa_w));
	m_ppi->out_pb_callback().set(FUNC(votrpss_state::ppi_pb_w));
	m_ppi->out_pc_callback().set(FUNC(votrpss_state::ppi_pc_w));

	TIMER(config, "irq_timer").configure_periodic(FUNC(votrpss_state::irq_timer), attotime::from_msec(10));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrpss)
	ROM_REGION(0x10000, "maincpu", 0)
	/* old logo PSS, version 3.A (1982), selftest 3.0? (1982) */
	//ROM_LOAD("u-2.3.A.bin",   0x0000, 0x2000, NO_DUMP )) /* 3.A 1982 */
	//ROM_LOAD("u-3.3.A.bin",   0x2000, 0x2000, NO_DUMP )) /* 3.A 1982 */
	//ROM_LOAD("u-4.3.0.bin", 0xc000, 0x2000, NO_DUMP )) /* 3.0? */

	/* old logo PSS, version 3.B (late 82/early83), selftest 3.0? (1982) */
	//ROM_LOAD("u-2.3.B.bin",   0x0000, 0x2000, NO_DUMP )) /* 3.B 1983? */
	//ROM_LOAD("u-3.3.B.bin",   0x2000, 0x2000, NO_DUMP )) /* 3.B 1983? */
	//ROM_LOAD("u-4.3.0.bin", 0xc000, 0x2000, NO_DUMP )) /* 3.0? */

	/* old or new logo PSS, Version 3.C (1984?), selftest 3.1 (1985?) */
	ROM_LOAD("u-2.1985.bin",   0x0000, 0x2000, CRC(410c58cf) SHA1(6e181e61ab9c268e3772fbeba101302fd40b09a2)) /* 3.C 1984?; The 1987/1988 version marked "U-2 // 090788" matches this rom */
	ROM_LOAD("u-3.1985.bin",   0x2000, 0x2000, CRC(1439492e) SHA1(46af8ccac6fdb93cbeb8a6d57dce5898e0e0d623)) /* 3.C 1984? */
	ROM_LOAD("u-4.100985.bin", 0xc000, 0x2000, CRC(0b7c4260) SHA1(56f0b6b1cd7b1104e09a9962583121c112337984)) /* 3.1 10/09/85 */

ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE    INPUT    CLASS          INIT        COMPANY   FULLNAME                  FLAGS
COMP( 1982, votrpss, 0,      0,      votrpss,   votrpss, votrpss_state, empty_init, "Votrax", "Personal Speech System", 0 )
