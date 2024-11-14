// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macquadra605.cpp
    Mac Quadra 605 ("Aladdin", "Primus")
    Mac LC 475
    Mac LC 575 ("Optimus")

    By R. Belmont

    These machines were cost-reduced versions of the "Wombat" (Quadra/Centris 610/650/800)
    machines.  djMEMC was replaced with a cost-reduced version called MEMCjr,
    and IOSB was replaced with PrimeTime, which is mostly compatible.

****************************************************************************/

#include "emu.h"

#include "cuda.h"
#include "djmemc.h"
#include "iosb.h"
#include "macadb.h"
#include "mactoolbox.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68040.h"
#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80scc.h"

#include "softlist_dev.h"

#define C32M 31.3344_MHz_XTAL
#define C15M (C32M/2)
#define C7M (C32M/4)

namespace {

class quadra605_state : public driver_device
{
public:
	quadra605_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_memcjr(*this, "memcjr"),
		m_primetime(*this, "primetime"),
		m_macadb(*this, "macadb"),
		m_cuda(*this, "cuda"),
		m_scc(*this, "scc"),
		m_ram(*this, RAM_TAG),
		m_scsibus(*this, "scsi"),
		m_ncr1(*this, "scsi:7:ncr53c96")
	{
	}

	void macqd605(machine_config &config);
	void maclc475(machine_config &config);
	void maclc575(machine_config &config);

	void quadra605_map(address_map &map) ATTR_COLD;
	void lc475_map(address_map &map) ATTR_COLD;
	void lc575_map(address_map &map) ATTR_COLD;

	void init_macqd605();

private:
	required_device<m68040_device> m_maincpu;
	required_device<memcjr_device> m_memcjr;
	required_device<primetime_device> m_primetime;
	required_device<macadb_device> m_macadb;
	required_device<cuda_device> m_cuda;
	required_device<z80scc_device> m_scc;
	required_device<ram_device> m_ram;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c96_device> m_ncr1;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 mac_scc_r(offs_t offset)
	{
		m_primetime->via_sync();
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, u16 data) { m_primetime->via_sync();
		m_scc->dc_ab_w(offset, data >> 8); }

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}
};

void quadra605_state::machine_start()
{
	m_memcjr->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());
}

void quadra605_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void quadra605_state::init_macqd605()
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void quadra605_state::quadra605_map(address_map &map)
{
	map(0x00000000, 0xffffffff).m(m_memcjr, FUNC(memcjr_device::map));
	map(0x50000000, 0x5fffffff).m(m_primetime, FUNC(primetime_device::map));

	map(0x5000c000, 0x5000dfff).rw(FUNC(quadra605_state::mac_scc_r), FUNC(quadra605_state::mac_scc_2_w)).mirror(0x00fc0000);

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2225; }));
}

void quadra605_state::lc475_map(address_map &map)
{
	quadra605_map(map);
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2221; }));
}

void quadra605_state::lc575_map(address_map &map)
{
	quadra605_map(map);
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a222e; }));
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void quadra605_state::macqd605(machine_config &config)
{
	M68040(config, m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra605_state::quadra605_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	MEMCJR(config, m_memcjr, 25_MHz_XTAL);
	m_memcjr->set_maincpu_tag("maincpu");
	m_memcjr->set_rom_tag("bootrom");
	m_memcjr->write_irq().set(m_primetime, FUNC(primetime_device::via2_irq_w<0x40>));

	PRIMETIME(config, m_primetime, 25_MHz_XTAL);
	m_primetime->set_maincpu_tag("maincpu");
	m_primetime->set_scsi_tag("scsi:7:ncr53c96");

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(m_primetime, FUNC(primetime_device::scc_irq_w));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	// SCSI bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^primetime:lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^primetime:rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c96", NCR53C96).clock(40_MHz_XTAL).machine_config(
		[this] (device_t *device)
		{
			ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

			adapter.set_busmd(ncr53c96_device::BUSMD_1);
			adapter.irq_handler_cb().set(m_primetime, FUNC(primetime_device::scsi_irq_w));
			adapter.drq_handler_cb().set(m_primetime, FUNC(primetime_device::scsi_drq_w));
		});

	MACADB(config, m_macadb, C15M);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0788");
	m_cuda->reset_callback().set(FUNC(quadra605_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_primetime, FUNC(primetime_device::cb1_w));
	m_cuda->via_data_callback().set(m_primetime, FUNC(primetime_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	m_primetime->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_primetime->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_primetime->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_primetime->write_cb2().set(m_cuda, FUNC(cuda_device::set_via_data));

	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqe_callback().set(m_primetime, FUNC(primetime_device::via2_irq_w<0x20>));
	NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_cards, nullptr);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,96M,128M,192M,256M,320M,384M,512M,640M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");
}

void quadra605_state::maclc475(machine_config &config)
{
	macqd605(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &quadra605_state::lc475_map);
}

void quadra605_state::maclc575(machine_config &config)
{
	macqd605(config);

	M68040(config.replace(), m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra605_state::lc575_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	m_ram->set_default_size("5M");
}

ROM_START( macqd605 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "ff7439ee.bin", 0x000000, 0x100000, CRC(b8514689) SHA1(1d833125adf553a50f5994746c2c01aa5a1dbbf2) )
ROM_END

#define rom_maclc475 rom_macqd605
#define rom_maclc575 rom_macqd605

} // anonymous namespace

COMP( 1993, macqd605, 0, 0, macqd605, macadb, quadra605_state, init_macqd605,  "Apple Computer", "Macintosh Quadra 605", MACHINE_SUPPORTS_SAVE)
COMP( 1993, maclc475, macqd605, 0, maclc475, macadb, quadra605_state, init_macqd605,  "Apple Computer", "Macintosh LC/Performa 475", MACHINE_SUPPORTS_SAVE)
COMP( 1994, maclc575, macqd605, 0, maclc575, macadb, quadra605_state, init_macqd605,  "Apple Computer", "Macintosh LC/Performa 575", MACHINE_SUPPORTS_SAVE)
