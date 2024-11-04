// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NEC EWS4800 systems.
 *
 * Sources:
 *  - http://www.jira-net.or.jp/vm/data/1993090101/1993090101knr/4-1-14.pdf
 *  - http://wiki.netbsd.org/ports/ews4800mips/
 *
 * TODO:
 *  - everything
 */

#include "emu.h"

// processors and memory
#include "cpu/mips/r4000.h"
#include "machine/ram.h"

// i/o devices
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/timekpr.h"
#include "machine/ncr53c90.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class ews4800_state : public driver_device
{
public:
	ews4800_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc%u", 0U)
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr53c96")
		, m_net(*this, "net")
	{
	}

	// machine config
	void ews4800_310(machine_config &config);

	void init();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;

	u16 lance_r(offs_t offset, u16 mem_mask = 0xffff);
	void lance_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

private:
	// processors and memory
	required_device<r4000_device> m_cpu;
	required_device<ram_device> m_ram;

	// i/o devices
	required_device<m48t02_device> m_rtc;
	required_device_array<z80scc_device, 2> m_scc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_scsi;
	required_device<am7990_device> m_net;
};

void ews4800_state::machine_start()
{
}

void ews4800_state::machine_reset()
{
}

void ews4800_state::init()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}


void ews4800_state::cpu_map(address_map &map)
{
	map(0x1fc00000, 0x1fcfffff).rom().region("eprom", 0);
}

u16 ews4800_state::lance_r(offs_t offset, u16 mem_mask)
{
	return 0;
}

void ews4800_state::lance_w(offs_t offset, u16 data, u16 mem_mask)
{
}

static void ews4800_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

/*
 * irq  function
 *  1   fdd, printer
 *  2   ethernet, scsi
 *  3   vme?
 *  4   serial
 *  5   clock
 */
void ews4800_state::ews4800_310(machine_config &config)
{
	R4000(config, m_cpu, 40_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &ews4800_state::cpu_map);

	// 8 SIMM slots
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("80M,144M");
	m_ram->set_default_value(0);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", ews4800_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", ews4800_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", ews4800_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", ews4800_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", ews4800_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", ews4800_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", ews4800_scsi_devices, nullptr);

	// scsi host adapter (NCR53C96)
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c96", NCR53C94).clock(24_MHz_XTAL).machine_config(
		[](device_t *device)
		{
			ncr53c94_device &adapter = downcast<ncr53c94_device &>(*device);

			adapter.set_busmd(ncr53c94_device::busmd_t::BUSMD_1);
			//adapter.irq_handler_cb()
			//adapter.drq_handler_cb()
		});

	// ethernet
	AM7990(config, m_net);
	//m_net->intr_out()
	m_net->dma_in().set(FUNC(ews4800_state::lance_r));
	m_net->dma_out().set(FUNC(ews4800_state::lance_w));

	// mouse on channel A, keyboard on channel B?
	SCC85230(config, m_scc[0], 4.915200_MHz_XTAL); // TODO: clock unconfirmed
	SCC85230(config, m_scc[1], 4.915200_MHz_XTAL); // TODO: clock unconfirmed

	M48T02(config, m_rtc);
}

ROM_START(ews4800_310)
	ROM_REGION64_BE(0x100000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ews4800_310", "ews4800_310")
	ROMX_LOAD("g8ppg__0100.a01f2", 0x00000, 0x80000, CRC(a1e25ce7) SHA1(cfc5e2b203bf6018b04980deeee43afa202dea7c), ROM_BIOS(0))
	ROMX_LOAD("g8ppg__0200.a01f",  0x80000, 0x80000, CRC(d610f20d) SHA1(f8476bf91111b8023ff7984e5e9a8575e48ed5df), ROM_BIOS(0))
ROM_END

} // anonymous namespace

/*   YEAR   NAME         PARENT  COMPAT  MACHINE      INPUT  CLASS          INIT  COMPANY  FULLNAME       FLAGS */
COMP(1993,  ews4800_310, 0,      0,      ews4800_310, 0,     ews4800_state, init, "NEC",   "EWS4800/310", MACHINE_IS_SKELETON)
