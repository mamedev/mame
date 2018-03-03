// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector 4

        08/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


class vector4_state : public driver_device
{
public:
	vector4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void vector4(machine_config &config);
	void vector4_io(address_map &map);
	void vector4_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


ADDRESS_MAP_START(vector4_state::vector4_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf8ff) AM_ROM
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(vector4_state::vector4_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("uart3", i8251_device, data_r, data_w)
	AM_RANGE(0x07, 0x07) AM_DEVREADWRITE("uart3", i8251_device, status_r, control_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vector4 )
INPUT_PORTS_END


void vector4_state::machine_reset()
{
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}


MACHINE_CONFIG_START(vector4_state::vector4)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(vector4_mem)
	MCFG_CPU_IO_MAP(vector4_io)

	/* video hardware */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart3", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart3", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1", i8251_device, write_cts))

	MCFG_DEVICE_ADD("uart2", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart2", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart2", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart2", i8251_device, write_cts))

	MCFG_DEVICE_ADD("uart3", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232c", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232c", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232c", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232c", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart3", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart3", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart3", i8251_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vector4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS( 0, "v4c", "ver 4.0c" )
	ROMX_LOAD( "vg40cl_ihl.bin", 0xe000, 0x0400, CRC(dcaf79e6) SHA1(63619ddb12ff51e5862902fb1b33a6630f555ad7), ROM_BIOS(1))
	ROMX_LOAD( "vg40ch_ihl.bin", 0xe400, 0x0400, CRC(3ff97d70) SHA1(b401e49aa97ac106c2fd5ee72d89e683ebe34e34), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "v43", "ver 4.3" )
	ROMX_LOAD( "vg-em-43.bin",   0xe000, 0x1000, CRC(29a0fcee) SHA1(ca44de527f525b72f78b1c084c39aa6ce21731b5), ROM_BIOS(2))

	ROM_SYSTEM_BIOS( 2, "v5", "ver 5.0" )
	ROMX_LOAD( "vg-zcb50.bin",   0xe000, 0x1000, CRC(22d692ce) SHA1(cbb21b0acc98983bf5febd59ff67615d71596e36), ROM_BIOS(3))
	ROM_LOAD( "mfdc.bin", 0xf800, 0x0100, CRC(d82a40d6) SHA1(cd1ef5fb0312cd1640e0853d2442d7d858bc3e3b))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE          INIT  COMPANY            FULLNAME    FLAGS
COMP( 19??, vector4, 0,      0,       vector4,   vector4, vector4_state, 0,    "Vector Graphics", "Vector 4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
