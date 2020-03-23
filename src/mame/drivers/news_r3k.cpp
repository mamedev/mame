// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS R3000 systems.
 *
 * Sources:
 *
 * TODO
 *   - skeleton only
 */

#include "emu.h"

#include "cpu/mips/mips1.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/am79c90.h"
#include "machine/timekpr.h"
#include "machine/z80scc.h"

// video
#include "screen.h"

// audio
#include "sound/spkrdev.h"
#include "speaker.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

class news_r3k_state : public driver_device
{
public:
	news_r3k_state(machine_config const &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_scsibus(*this, "scsi")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);

	// machine config
	void common(machine_config &config);

public:
	void nws3260(machine_config &config);

	void init_common();

protected:
	// devices
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<m48t02_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus;
};

void news_r3k_state::machine_start()
{
}

void news_r3k_state::machine_reset()
{
}

void news_r3k_state::init_common()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void news_r3k_state::cpu_map(address_map &map)
{
	map(0x1fc00000, 0x1fc1ffff).rom().region("eprom", 0);

	map(0x1ff40000, 0x1ff407ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
}

static void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void news_r3k_state::common(machine_config &config)
{
	R3000A(config, m_cpu, 20_MHz_XTAL, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &news_r3k_state::cpu_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);

	// 12 SIMM slots
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	M48T02(config, m_rtc);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, "cdrom");
}

void news_r3k_state::nws3260(machine_config &config)
{
	common(config);
}

ROM_START(nws3260)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3260", "NWS-3260 v2.0A")
	ROMX_LOAD("nws3260.bin", 0x00000, 0x20000, CRC(61222991) SHA1(076fab0ad0682cd7dacc7094e42efe8558cbaaa1), ROM_BIOS(0))
ROM_END

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT         COMPANY  FULLNAME    FLAGS */
COMP(1991, nws3260, 0,      0,      nws3260, 0,     news_r3k_state, init_common, "Sony",  "NWS-3260", MACHINE_IS_SKELETON)
