// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Siemens PC-MX2
 *
 * Sources:
 *   - http://www.cpu-ns32k.net/Siemens.html
 *   - https://mx300i.narten.de/view_board.cfm?5EF287A1ABC3F4DCAFEA2BC2FAB8C4000E50392BD4C3E389826E8E68AC9F53F3BE16A107EFC767
 *
 * TODO:
 *   - skeleton only
 */

#include "emu.h"

// cpu board hardware
#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/mc146818.h"

// dueai board hardware
#include "cpu/i86/i186.h"
#include "machine/z80scc.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class pcmx2_state : public driver_device
{
public:
	pcmx2_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_rtc(*this, "rtc")
		, m_dueai_cpu(*this, "dueai_cpu")
		, m_dueai_scc(*this, "dueai_scc")
		, m_boot_view(*this, "boot_view")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map);
	void dueai_mem_map(address_map &map);
	void dueai_pio_map(address_map &map);

public:
	// machine config
	void pcmx2(machine_config &config);

protected:
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<mc146818_device> m_rtc;

	required_device<i80186_cpu_device> m_dueai_cpu;
	required_device<z80scc_device> m_dueai_scc;

private:
	memory_view m_boot_view;
};

void pcmx2_state::machine_start()
{
}

void pcmx2_state::machine_reset()
{
	m_boot_view.select(0);
}

template <unsigned ST> void pcmx2_state::cpu_map(address_map &map)
{
	if (ST == 0)
	{
		map(0x000000, 0x0fffff).view(m_boot_view);

		m_boot_view[0](0x000000, 0x007fff).rom().region("eprom", 0);
		m_boot_view[1](0x000000, 0x0fffff).ram();
	}

	map(0xfe0000, 0xfe7fff).rom().region("eprom", 0);

	map(0xff8500, 0xff8500).lw8([this](u8 data) { m_boot_view.select(data); }, "boot_view_w");

	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0x00ff);
}

void pcmx2_state::dueai_mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram();

	map(0x0fe000, 0x0fffff).rom().region("dueai", 0);
}

void pcmx2_state::dueai_pio_map(address_map &map)
{
	map(0x0000, 0x0003).rw(m_dueai_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
}

void pcmx2_state::pcmx2(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &pcmx2_state::cpu_map<0>);
	m_cpu->set_addrmap(4, &pcmx2_state::cpu_map<4>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 20_MHz_XTAL / 2);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	MC146818(config, m_rtc, 32.768_kHz_XTAL);

	I80186(config, m_dueai_cpu, 16_MHz_XTAL / 2);
	m_dueai_cpu->set_addrmap(AS_PROGRAM, &pcmx2_state::dueai_mem_map);
	m_dueai_cpu->set_addrmap(AS_IO, &pcmx2_state::dueai_pio_map);

	SCC8530N(config, m_dueai_scc, 7.3728_MHz_XTAL);
}

ROM_START(pcmx2)
	ROM_REGION16_LE(0x8000, "eprom", 0)
	ROMX_LOAD("361d0333d053__e01735_ine.d53", 0x0000, 0x4000, CRC(b5eefb64) SHA1(a71a7daf9a8f0481d564bfc4d7ed5eb955f8665f), ROM_SKIP(1))
	ROMX_LOAD("361d0333d054__e01725_ine.d54", 0x0001, 0x4000, CRC(3a3c6b6e) SHA1(5302fd79c89e0b4d164c639e2d73f4b9a279ddcb), ROM_SKIP(1))

	ROM_REGION16_LE(0x2000, "dueai", 0)
	ROM_LOAD("d277__e00_410__d3_17e4.d61", 0x0000, 0x2000, CRC(31415348) SHA1(1407dac077b1f9aec5f9063711c3ab2c517cc9e8))
ROM_END

}

/*   YEAR   NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS */
COMP(1986?, pcmx2, 0,      0,      pcmx2,   0,     pcmx2_state, empty_init, "Siemens", "PC-MX2", MACHINE_IS_SKELETON)
