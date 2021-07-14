// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Casio SX1010 and SX1050.
 *
 * Sources:
 *
 * TODO:
 *   - skeleton only
 */

/*
 * Preliminary parts list for 600-CPU-PCA CPU board:
 *
 * MC68010L10
 * D8237AC-5
 * HD63484-8
 * D8251AFC
 * D72065C
 * D8259AC-2
 * D8251AFC
 * D65021G031    gate array
 * D65030G024    gate array
 * HN62301AP AA1 128k mask ROM
 * HN62301AP AA2 128k mask ROM
 * MB89321A      CMOS Programmable CRT Controller
 * MB4108        ASSP Floppy Disk VFO
 * RP5C15        RTC
 * 48.8MHz XTAL
 *
 * D41256C-15    262144x1 DRAM, x36 == 1M with parity main RAM?
 * MB81464-12    262144x1 DRAM, x16 == 512K video RAM?
 * HM6264ALSP-12 8192-word 8-bit High Speed CMOS Static RAM, x2 == 16K non-volatile RAM?
 */
#include "emu.h"

#include "cpu/m68000/m68000.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"

// video
#include "screen.h"
//#include "video/hd63484.h"

// busses and connectors
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class sx1000_state : public driver_device
{
public:
	sx1000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
	{
	}

	void sx1010(machine_config &config);
	void init_common();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cpu_map(address_map &map);

	void common(machine_config &config);

private:
	// devices
	required_device<m68010_device> m_cpu;
	required_device<ram_device> m_ram;
};

void sx1000_state::machine_start()
{
}

void sx1000_state::machine_reset()
{
}

void sx1000_state::init_common()
{
}

void sx1000_state::cpu_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("eprom", 0x8000); // FIXME: probably mapped/unmapped during reset

	map(0xf00000, 0xf07fff).rom().region("eprom", 0x8000);
}

void sx1000_state::common(machine_config &config)
{
	M68010(config, m_cpu, 10'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &sx1000_state::cpu_map);

	// 36 x D41256C-15 (256Kb DRAM) on CPU board
	RAM(config, m_ram);
	m_ram->set_default_size("1M");
}

void sx1000_state::sx1010(machine_config &config)
{
	common(config);
}

static INPUT_PORTS_START(sx1010)
INPUT_PORTS_END

ROM_START(sx1010)
	ROM_REGION16_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "sx1010", "sx1010")
	ROMX_LOAD("pc1a.u6l8", 0x0001, 0x8000, CRC(75d0f02c) SHA1(dfcc7efc1b5e7b43fc1ee030bef5c75c23d5e742), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("pc1a.u6h8", 0x0000, 0x8000, CRC(a928c0c9) SHA1(712601ee889a0790a7579fe06df20ec7e0a4bb49), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

} // anonymous namespace

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME  FLAGS */
COMP(1987, sx1010, 0,      0,      sx1010,  sx1010, sx1000_state, init_common, "Casio", "SX1010", MACHINE_IS_SKELETON)
