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


namespace {

class ipc_state : public driver_device
{
public:
	ipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ipc(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};


void ipc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).ram();
	map(0xe800, 0xffff).rom().region("roms", 0);
}

void ipc_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf4, 0xf5).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf6, 0xf7).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( ipc )
INPUT_PORTS_END


void ipc_state::machine_reset()
{
	m_maincpu->set_state_int(i8085a_cpu_device::I8085_PC, 0xE800);
}


void ipc_state::ipc(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(19'660'800) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ipc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ipc_state::io_map);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(XTAL(19'660'800) / 16);
	pit.set_clk<1>(XTAL(19'660'800) / 16);
	pit.set_clk<2>(XTAL(19'660'800) / 16);
	pit.out_handler<0>().set("uart1", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("uart1", FUNC(i8251_device::write_rxc));
	pit.out_handler<1>().set("uart2", FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append("uart2", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0)); // 8 data bits, no parity, 1 stop bit, 9600 baud
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 0)); // 8 data bits, no parity, 2 stop bits, 2400 baud
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( ipb )
	ROM_REGION( 0x1800, "roms", 0 )
	ROM_LOAD( "ipb_e8_v1.3.bin", 0x0000, 0x0800, CRC(fc9d4703) SHA1(2ce078e1bcd8b24217830c54bcf04c5d146d1b76) )
	ROM_LOAD( "ipb_f8_v1.3.bin", 0x1000, 0x0800, CRC(966ba421) SHA1(d6a904c7d992a05ed0f451d7d34c1fc8de9547ee) )
ROM_END

ROM_START( ipc )
	ROM_REGION( 0x1800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "ipc_v1.3_104584-001.u82", 0x0000, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2) )
	ROM_LOAD( "ipc_f8_v1.3.bin", 0x1000, 0x0800, NO_DUMP )   // rom name unknown
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS */
COMP( 19??, ipb,  0,      0,      ipc,     ipc,   ipc_state, empty_init, "Intel", "iPB",    MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 19??, ipc,  ipb,    0,      ipc,     ipc,   ipc_state, empty_init, "Intel", "iPC",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
