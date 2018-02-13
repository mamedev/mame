// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

QT Computer Systems SBC +2/4

2009-12-11 Skeleton driver.

This looks like the same system as the Compu/Time SBC-880 Processor Board.

Currently it crashes. There's a memory move routine at 50A4, and after
a few turns it is told to move E603 bytes which corrupts everything.

Chips: P8251, D8253C, MK3880N-4 (Z80). 3x 6-sw dips. Unmarked crystal.
       Doesn't appear to be any ram?

There's a blue jumper marked 4M and 2M. Assumed to be selectable CPU clock.
Also assumed this is what the "2/4" in the name refers to.

Feature list from QT ad:
- 1K RAM (which can be located at any 1K boundary) plus one each
  Parallel and Serial I/O ports on board
- Power on jump to onboard EPROM (2708 or 2716)
- EPROM addressable on any 1K or 2K boundary
- Full 64K use of RAM allowed in shadow mode
- Programmable Baud rate selection, 110-9600
- 2 or 4MHz switch selectable
- DMA capability allows MWRT signal generation on CPU board or elsewhere
  in system under DMA logic or front panel control
- Two programmable timers available for use by programs run with the
  SBC+2/4 (timer output and controls available at parallel I/O connector;
  parallel input and output ports available for use on CPU board).

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


class qtsbc_state : public driver_device
{
public:
	qtsbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
	{ }

	DECLARE_READ8_MEMBER( qtsbc_43_r );

	void qtsbc(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_p_ram;
};


READ8_MEMBER( qtsbc_state::qtsbc_43_r )
{
	return 0; // this controls where the new ram program gets built at. 0 = 0xE000.
}

ADDRESS_MAP_START(qtsbc_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("ram") AM_REGION("maincpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(qtsbc_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x07, 0x07) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x43, 0x43) AM_READ(qtsbc_43_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( qtsbc )
INPUT_PORTS_END


void qtsbc_state::machine_reset()
{
	uint8_t* bios = memregion("maincpu")->base()+0x10000;
	memcpy(m_p_ram, bios, 0x800);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(qtsbc_state::qtsbc)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000)) // Mostek MK3880
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	/* video hardware */
	MCFG_DEVICE_ADD("pit", PIT8253, 0) // U9
	MCFG_PIT8253_CLK0(XTAL(4'000'000) / 2) /* Timer 0: baud rate gen for 8251 */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart", I8251, 0) // U8
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal) // must be exactly here
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( qtsbc )
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "qtsbc.u23", 0x10000, 0x0800, CRC(823fd942) SHA1(64c4f74dd069ae4d43d301f5e279185f32a1efa0))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT  COMPANY                  FULLNAME       FLAGS
COMP( 19??, qtsbc,  0,      0,      qtsbc,   qtsbc, qtsbc_state, 0,    "QT Computer Systems Inc.", "SBC + 2/4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
