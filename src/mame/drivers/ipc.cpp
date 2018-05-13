// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Intel iPB and iPC

        17/12/2009 Skeleton driver.

        22/04/2011 Connected to a terminal, it responds. Modernised.

        --> When started, you must press Space, then it will start to work.

        Monitor commands:
        A
        Dn n - dump memory
        E
        Fn n n - fill memory
        G
        Hn n - hex arithmetic
        Mn n n - move (copy) memory block
        N
        Q
        R
        Sn - modify a byte of memory
        W - display memory in Intel? format
        X - show and modify registers


        Preliminary Memory Map
        E800-F7FF BIOS ROM area
        F800-FFFF Monitor ROM (or other user interface)

        I/O F4/F5 main console input and output
        I/O F6/F7 alternate console input

        ToDo:
        - Everything!
        - iPC - Find missing rom F800-FFFF

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"


class ipc_state : public driver_device
{
public:
	ipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ipc(machine_config &config);
	void ipc_io(address_map &map);
	void ipc_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


void ipc_state::ipc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).ram();
	map(0xe800, 0xffff).rom().region("roms", 0);
}

void ipc_state::ipc_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf4, 0xf4).rw("uart1", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf5, 0xf5).rw("uart1", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xf6, 0xf6).rw("uart2", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf7, 0xf7).rw("uart2", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
}

/* Input ports */
static INPUT_PORTS_START( ipc )
INPUT_PORTS_END


void ipc_state::machine_reset()
{
	m_maincpu->set_state_int(i8085a_cpu_device::I8085_PC, 0xE800);
}


MACHINE_CONFIG_START(ipc_state::ipc)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",I8085A, XTAL(19'660'800) / 4)
	MCFG_DEVICE_PROGRAM_MAP(ipc_mem)
	MCFG_DEVICE_IO_MAP(ipc_io)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(19'660'800) / 16)
	MCFG_PIT8253_CLK1(XTAL(19'660'800) / 16)
	MCFG_PIT8253_CLK2(XTAL(19'660'800) / 16)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart1", i8251_device, write_rxc))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE("uart2", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart2", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart1", I8251, 0) // 8 data bits, no parity, 1 stop bit, 9600 baud
	MCFG_I8251_TXD_HANDLER(WRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(WRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(WRITELINE("rs232a", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart1", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart1", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart1", i8251_device, write_cts))

	MCFG_DEVICE_ADD("uart2", I8251, 0) // 8 data bits, no parity, 2 stop bits, 2400 baud
	MCFG_I8251_TXD_HANDLER(WRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(WRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(WRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart2", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart2", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart2", i8251_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ipb )
	ROM_REGION( 0x1800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "ipb_e8_v1.3.bin", 0x0000, 0x0800, CRC(fc9d4703) SHA1(2ce078e1bcd8b24217830c54bcf04c5d146d1b76) )
	ROM_LOAD( "ipb_f8_v1.3.bin", 0x1000, 0x0800, CRC(966ba421) SHA1(d6a904c7d992a05ed0f451d7d34c1fc8de9547ee) )
ROM_END

ROM_START( ipc )
	ROM_REGION( 0x1800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "ipc_u82_v1.3_104584-001.bin", 0x0000, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2) )

	// required rom is missing. Using this one from 'ipb' for now.
	ROM_LOAD( "ipb_f8_v1.3.bin", 0x1000, 0x0800, BAD_DUMP CRC(966ba421) SHA1(d6a904c7d992a05ed0f451d7d34c1fc8de9547ee) )
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS */
COMP( 19??, ipb,  0,      0,      ipc,     ipc,   ipc_state, empty_init, "Intel", "iPB",    MACHINE_NO_SOUND_HW )
COMP( 19??, ipc,  ipb,    0,      ipc,     ipc,   ipc_state, empty_init, "Intel", "iPC",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
