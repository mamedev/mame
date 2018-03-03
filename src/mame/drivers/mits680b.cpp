// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

MITS Altair 680b

2009-12-03 Skeleton driver.
2011-06-08 Connected to a terminal

Monitor Commands:
J
L switch to terminal mode
M
N modify memory in a limited way
P this does a rti and causes a momentary crash. Weird.


ToDo:


****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"


class mits680b_state : public driver_device
{
public:
	mits680b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(status_check_r);

	void mits680b(machine_config &config);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

READ8_MEMBER( mits680b_state::status_check_r )
{
	return 0; // crashes at start if bit 7 high
}


ADDRESS_MAP_START(mits680b_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAM // 1024 bytes RAM
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("acia", acia6850_device, read, write)
	AM_RANGE(0xf002, 0xf002) AM_READ(status_check_r)
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mits680b )
INPUT_PORTS_END


MACHINE_CONFIG_START(mits680b_state::mits680b)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL(1'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("acia", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("acia", acia6850_device, write_rxc))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mits680b )
	ROM_REGION( 0x100, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "mits680b.bin", 0x0000, 0x0100, CRC(397e717f) SHA1(257d3eb1343b8611dc05455aeed33615d581f29c))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT     STATE           INIT  COMPANY  FULLNAME       FLAGS
COMP( 1976, mits680b, 0,      0,      mits680b,  mits680b, mits680b_state, 0,    "MITS",  "Altair 680b", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
