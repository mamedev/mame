// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Robbbert
/******************************************************************************
*
*  Votrax Personal Speech System Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with help from Kevin 'kevtris' Horton
*  Special thanks to Professor Nicholas Gessler for loaning several PSS units
*
*  The votrax PSS was sold from around 35th week of 1982 until october, 1990 (LONG product life)

<kevtris> timer0 = baud clock
<kevtris> timer1 = pitch (duty cycle output to modify RC)
<kevtris> timer2 = volume (duty cycle output to control trans. gate which does vol. control)
<kevtris> portb: pin 6 through pin 13 of parallel port
<kevtris> portc 0 = NC, 1 = GND, 2 = pin 5, 3 = /RXINTEN, 4 = pin 15, 5 = pin 14 through inverter, 6 = 8910 enable, 7 = from pin 4 through inverter (I believe that's for the parallel port)
<kevtris> porta: pin 16 through 23 of parallel port
<kevtris> that's the 8255
<kevtris> on the AY-3-8910:
<kevtris> IOA0-A5 = phoneme #
<kevtris> IOA6 = strobe (SC-01)
<kevtris> IOA7 = vochord control, 0 = off, 1 = on
<kevtris> IOB0-IOB7 = dip switches
<kevtris> there ya go, complete IO port map
<LordNLptp> cool :)
<kevtris> I pinned the IO to the serial port from the 8251 but I don't think it's too useful
<LordNLptp> eh, its useful
<kevtris> I drew out the schematic for the analog section
<LordNLptp> oh that will be useful
<kevtris> but I didn't draw out the digital part, just made an I/O and memory map but I can't find them
<LordNLptp> i also see the ay is running at 2 MHz
<kevtris> sounds about right
<kevtris> got it
<kevtris> I drew out the clocking section too
<kevtris> 8MHz xtal
<kevtris> 4MHz for the Z80
<kevtris> 2MHz for the 8253 and '8910
<kevtris> then the dividers also generate the system reset signals for the 8251
<kevtris> and a periodic IRQ
<LordNLptp> on the z80?
<LordNLptp> ok how does that work?
<kevtris> IRQ rate is umm
<kevtris> 122Hz
<kevtris> (8MHz / 65536)
<kevtris> oh the 8251 is not reset by the counters
<kevtris> it just takes a positive reset off an inverter that resets the counters
<kevtris> PIT2 is gated by 8MHz / 256
<kevtris> PIT1 is gated by 8MHz / 4096
<kevtris> PIT0 is not gated
<kevtris> (i.e. it always runs)
<LordNLptp> oh boy, this is sounding more fun by the minute
<kevtris> aaand your luck is about to run out
<kevtris> that's all I got
<LordNLptp> thats good enough, saved me a lot of work
<kevtris> but that's everything you need
<LordNLptp> yeah


Things to be looked at:
- Serial doesn't work, so has been disabled.
- Bottom 3 dips must be off/serial/off, or else nothing works.
- No sound at all.
- volume and pitch should be controlled by ppi outputs
- pit to be hooked up
- bit 0 of portc is not connected according to text above, but it
  completely changes the irq operation.

******************************************************************************/

/* Core includes */
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
//#include "votrpss.lh"

/* Components */
#include "sound/ay8910.h"
#include "sound/votrax.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"

/* For testing */
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class votrpss_state : public driver_device
{
public:
	votrpss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_ppi(*this, "ppi"),
		m_uart(*this, "uart")
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(ppi_pa_r);
	DECLARE_READ8_MEMBER(ppi_pb_r);
	DECLARE_READ8_MEMBER(ppi_pc_r);
	DECLARE_WRITE8_MEMBER(ppi_pa_w);
	DECLARE_WRITE8_MEMBER(ppi_pb_w);
	DECLARE_WRITE8_MEMBER(ppi_pc_w);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	IRQ_CALLBACK_MEMBER(irq_ack);
private:
	UINT8 m_term_data;
	UINT8 m_porta;
	UINT8 m_portb;
	UINT8 m_portc;
	virtual void machine_start();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8255_device> m_ppi;
	required_device<i8251_device> m_uart;
};


/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(votrpss_mem, AS_PROGRAM, 8, votrpss_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM /* main roms (in potted module) */
	AM_RANGE(0x4000, 0x7fff) AM_NOP /* open bus/space for expansion rom (reads as 0xFF) */
	AM_RANGE(0x8000, 0x8fff) AM_RAM /* onboard memory (2x 6116) */
	AM_RANGE(0x9000, 0xbfff) AM_NOP /* open bus (space for memory expansion, checked by main roms, will be used if found)*/
	AM_RANGE(0xc000, 0xdfff) AM_ROM /* 'personality rom', containing self-test code and optional user code */
	AM_RANGE(0xe000, 0xffff) AM_NOP /* open bus (space for more personality rom, not normally used) */
ADDRESS_MAP_END

static ADDRESS_MAP_START(votrpss_io, AS_IO, 8, votrpss_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3e) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x41, 0x41) AM_MIRROR(0x3e) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x3e) AM_DEVREADWRITE("ay", ay8910_device, data_r, address_w)
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0x3e) AM_DEVREADWRITE("ay", ay8910_device, data_r, data_w)
ADDRESS_MAP_END


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
	PORT_DIPNAME( 0x40, 0x00, "Default Communications Port" )   PORT_DIPLOCATION("SW1:7")
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
	UINT8 ret = m_term_data;
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
	UINT8 data = 0;

	if (m_term_data)
	{
		m_ppi->pc4_w(0); // send a strobe pulse
		data |= 0x20;
	}

	return (m_portc & 0xdb) | data;
	//return data;
}

WRITE8_MEMBER( votrpss_state::ppi_pa_w )
{
	m_porta = data;
}

WRITE8_MEMBER( votrpss_state::ppi_pb_w )
{
	m_portb = data;
	m_terminal->write(space, offset, data&0x7f);
}

WRITE8_MEMBER( votrpss_state::ppi_pc_w )
{
	m_portc = data;
}

WRITE8_MEMBER( votrpss_state::kbd_put )
{
	m_term_data = data;
}

DECLARE_WRITE_LINE_MEMBER( votrpss_state::write_uart_clock )
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( votrpss, votrpss_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/2)  /* 4.000 MHz, verified */
	MCFG_CPU_PROGRAM_MAP(votrpss_mem)
	MCFG_CPU_IO_MAP(votrpss_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(votrpss_state,irq_ack)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT(layout_votrpss)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay", AY8910, XTAL_8MHz/4) /* 2.000 MHz, verified */
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))        // port B read
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("votrax", votrax_sc01_device, write))     // port A write
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000) /* 720 kHz? needs verify */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(votrpss_state, kbd_put))

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_8MHz) /* Timer 0: baud rate gen for 8251 */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(votrpss_state, write_uart_clock))
	MCFG_PIT8253_CLK1(XTAL_8MHz / 256) /* Timer 1: Pitch */
	MCFG_PIT8253_CLK2(XTAL_8MHz / 4096) /* Timer 2: Volume */

	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(votrpss_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(votrpss_state, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(votrpss_state, ppi_pb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(votrpss_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(votrpss_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(votrpss_state, ppi_pc_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", votrpss_state, irq_timer, attotime::from_msec(10))
MACHINE_CONFIG_END



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

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                            FLAGS */
COMP( 1982, votrpss,   0,          0,      votrpss,   votrpss, driver_device, 0,      "Votrax",        "Personal Speech System", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
