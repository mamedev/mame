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
        0000-7FFF Onboard RAM (IPB & IPC)
        8000-F7FF Onboard RAM (IPC only)
        E800-EFFF Bootstrap & diagnostic ROM
        F800-FFFF Monitor ROM (or other user interface)

        I/O F4/F5 main console input and output
        I/O F6/F7 alternate console input
        I/O FF    ROM control (enable/disable access to bootstrap ROM)

        ToDo:
        - Everything!

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/74259.h"
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
		, m_ipcctrl(*this, "ipcctrl")
		, m_boot(*this, "boot")
	{ }

	void ipc(machine_config &config);
	void ipb(machine_config &config);

private:
	void board_common(machine_config &config);
	void io_map(address_map &map) ATTR_COLD;
	void ipb_mem_map(address_map &map) ATTR_COLD;
	void ipc_mem_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	void ipc_control_w(uint8_t data);
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_ipcctrl;
	memory_view m_boot;
};


void ipc_state::ipb_mem_map(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x7fff).ram();

	// selectively map the boot/diagnostic segment
	map(0x0000, 0xefff).view(m_boot);

	// SEL_BOOT/ == 0 and START_UP/ == 0
	m_boot[0](0x0000, 0x07ff).rom().region("roms", 0);
	m_boot[0](0xe800, 0xefff).rom().region("roms", 0);

	// SEL_BOOT/ == 0 and START_UP/ == 1
	m_boot[1](0xe800, 0xefff).rom().region("roms", 0);

	map(0xf800, 0xffff).rom().region("roms", 0x800);
}


void ipc_state::ipc_mem_map(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0xffff).ram();

	// selectively map the boot/diagnostic segment
	map(0x0000, 0xefff).view(m_boot);

	// SEL_BOOT/ == 0 and START_UP/ == 0
	m_boot[0](0x0000, 0x07ff).rom().region("roms", 0);
	m_boot[0](0xe800, 0xefff).rom().region("roms", 0);

	// SEL_BOOT/ == 0 and START_UP/ == 1
	m_boot[1](0xe800, 0xefff).rom().region("roms", 0);
	
	map(0xf800, 0xffff).rom().region("roms", 0x800);
}

void ipc_state::io_map(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf4, 0xf5).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf6, 0xf7).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xff, 0xff).w(FUNC(ipc_state::ipc_control_w));
}

/* Input ports */
static INPUT_PORTS_START( ipc )
INPUT_PORTS_END


void ipc_state::machine_reset()
{
	m_boot.select(0);
}

void ipc_state::ipc_control_w(uint8_t data)
{
	// b3 is ~(bit to be written)
	// b2-b0 is ~(no. of bit to be written)
	m_ipcctrl->write_bit(~data & 7, BIT(~data, 3));

	// SEL_BOOT/ == 0
	if (!m_ipcctrl->q3_r())
		// START_UP/
		m_boot.select(m_ipcctrl->q5_r());
	else
		m_boot.disable();
}


void ipc_state::ipc(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ipc_state::ipc_mem_map);
	m_maincpu->set_addrmap(AS_IO, &ipc_state::io_map);
	board_common(config);
}


void ipc_state::ipb(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, XTAL(23'400'000) / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &ipc_state::ipb_mem_map);
	m_maincpu->set_addrmap(AS_IO, &ipc_state::io_map);
	board_common(config);
}


void ipc_state::board_common(machine_config &config)
{
	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(XTAL(19'660'800) / 16);
	pit.set_clk<1>(XTAL(19'660'800) / 16);
	pit.set_clk<2>(XTAL(19'660'800) / 16);
	pit.out_handler<0>().set("uart1", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("uart1", FUNC(i8251_device::write_rxc));
	pit.out_handler<1>().set("uart2", FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append("uart2", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0)); // 8 data bits, no parity, 2 stop bits, 110 baud
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

	LS259(config, m_ipcctrl);
}

/* ROM definition */
ROM_START( ipb )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_DEFAULT_BIOS("mon12")
	// 1x2732 Copyright 1979 (in IPC)
	// note: it's not clear if an IPB ever shipped with this ROM, but aftermarket ROM upgrades to 1.3 were common
	ROM_SYSTEM_BIOS(0, "mon13", "Series II Monitor v1.3")
	ROMX_LOAD( "ipc_v1.3_104584-001.u82", 0x0000, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2), ROM_BIOS(0) )
	// 2x2716 Copyright 1978
	ROM_SYSTEM_BIOS(1, "mon12", "Series II Monitor v1.2")
	ROMX_LOAD( "ipb_e8_v1.2.bin", 0x0000, 0x0800, CRC(6496efaf) SHA1(1a9c0f1b19c1807803db3f1543f51349d7fd693a), ROM_BIOS(1) )
	ROMX_LOAD( "ipb_f8_v1.2.bin", 0x0800, 0x0800, CRC(258dc9a6) SHA1(3fde993aee06d9af5093d7a2d9a8cbd71fed0951), ROM_BIOS(1) )
	// 2x2716 Copyright 1978 (overwritten in F8 ROM by new code)
	// this version was discovered in an IPB with Siemens branding, saved from e-waste - it's unofficial, but is definitely out there
	// it's an adaptation to work with a 600 baud teleprinter with slow carriage return
	// the names are derived from the chips' markings ("BOOT 600Bd" and "MON 2 600Bd")
	// note: expect checksum messages from this ROM set, as whoever made it never bothered to correct the checksums
	ROM_SYSTEM_BIOS(2, "mon12_600bd", "Series II Monitor v1.2 - 600 baud teleprinter version")
	ROMX_LOAD( "ipb_e8_v1.2_boot_600bd.bin", 0x0000, 0x0800, CRC(6bf62535) SHA1(40ac2546c816884dffdaa0ed2c6e41eb8523dcd6), ROM_BIOS(2) )
	ROMX_LOAD( "ipb_f8_v1.2_mon_2_600bd.bin", 0x0800, 0x0800, CRC(d5ace425) SHA1(d39cd3522ffd1d334f6eb128990e488ce07243ab), ROM_BIOS(2) )
	// 2x2716 Copyright 1977
	ROM_SYSTEM_BIOS(3, "mon11", "Series II Monitor v1.1")
	ROMX_LOAD( "ipb_e8_v1.1.bin", 0x0000, 0x0800, CRC(ffb7c036) SHA1(6f60cdfe20621c4b633c972adcb644a1c02eaa39), ROM_BIOS(3) )
	ROMX_LOAD( "ipb_f8_v1.1.bin", 0x0800, 0x0800, CRC(3696ff28) SHA1(38b435e10a81629430275aec051fb0a55ec1f6fd), ROM_BIOS(3) )
ROM_END

ROM_START( ipc )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "ipc_v1.3_104584-001.u82", 0x0000, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME                         FLAGS */
COMP( 1977, ipb,  0,      0,      ipb,     ipc,   ipc_state, empty_init, "Intel", "Integrated Processor Board",    MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1979, ipc,  ipb,    0,      ipc,     ipc,   ipc_state, empty_init, "Intel", "Integrated Processor Card",     MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
