// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam 3000 3232 (V32?) processor board.
 *
 * Sources:
 *  - http://www.cpu-ns32k.net/Multi.html
 *  - https://arvutimuuseum.ut.ee/index.php?m=eksponaadid&id=223
 *
 * TODO:
 *  - skeleton only
 */

#include "emu.h"
#include "labtam_3232.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(LABTAM_3232, labtam_3232_device, "labtam_3232", "Labtam 3232")

labtam_3232_device::labtam_3232_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LABTAM_3232, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_icu(*this, "icu")
	, m_boot(*this, "boot")
	, m_installed(false)
{
}

ROM_START(labtam_3232)
	ROM_REGION32_LE(0x10000, "eprom", 0)
	ROM_LOAD32_BYTE("sash_0__g87.bin", 0x0000, 0x4000, CRC(31132de0) SHA1(39f7c1f146346a53c253284ba3437b5f9217f439))
	ROM_LOAD32_BYTE("sash_1__g87.bin", 0x0001, 0x4000, CRC(09b4e8ba) SHA1(b722386fc6e4a634472d5a0501eb41e12d89f7dc))
	ROM_LOAD32_BYTE("sash_2__g87.bin", 0x0002, 0x4000, CRC(d76457b0) SHA1(efd18e0347b0181a0d2aa80098bd4fdb60a82788))
	ROM_LOAD32_BYTE("sash_3__g87.bin", 0x0003, 0x4000, CRC(7817d1b8) SHA1(032df47e1ba34653b5837f68eefa3ecaac17a483))
ROM_END

static INPUT_PORTS_START(labtam_3232)
INPUT_PORTS_END

const tiny_rom_entry *labtam_3232_device::device_rom_region() const
{
	return ROM_NAME(labtam_3232);
}

ioport_constructor labtam_3232_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_3232);
}

void labtam_3232_device::device_start()
{
}

void labtam_3232_device::device_reset()
{
	if (!m_installed)
	{
		// TODO: what's exposed to the Multibus?
#if 0
		m_bus->int_callback<0>().set(m_icu, FUNC(ns32202_device::ir_w<0>));
		m_bus->int_callback<1>().set(m_icu, FUNC(ns32202_device::ir_w<1>));
		m_bus->int_callback<2>().set(m_icu, FUNC(ns32202_device::ir_w<3>));
		m_bus->int_callback<3>().set(m_icu, FUNC(ns32202_device::ir_w<4>));
		m_bus->int_callback<4>().set(m_icu, FUNC(ns32202_device::ir_w<6>));
		m_bus->int_callback<5>().set(m_icu, FUNC(ns32202_device::ir_w<7>));
		m_bus->int_callback<6>().set(m_icu, FUNC(ns32202_device::ir_w<8>));
		m_bus->int_callback<7>().set([this](int state) { m_s8->read() ? m_cpu->set_input_line(INPUT_LINE_NMI, !state) : m_icu->ir_w<11>(state); });
#endif
		m_installed = true;
	}

	m_boot.select(0);
}

void labtam_3232_device::device_add_mconfig(machine_config &config)
{
	NS32032(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &labtam_3232_device::cpu_map<0>);
	m_cpu->set_addrmap(4, &labtam_3232_device::cpu_map<4>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 20_MHz_XTAL / 2);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();
}

template <unsigned ST> void labtam_3232_device::cpu_map(address_map &map)
{
	if (ST == 0)
	{
		map(0x00000000, 0x007fffff).view(m_boot);

		m_boot[0](0x00000000, 0x0000ffff).rom().region("eprom", 0);
		m_boot[1](0x00000000, 0x001fffff).ram(); // 2M on-board RAM with parity (up to 8M possible?)

		// 0xd00000 - maybe multibus memory or i/o?
		map(0x00d00413, 0x00d00413).lr8([]() { return 0x18; }, "unknown_r8");
		map(0x00d00500, 0x00d00503).lr16([]() { return 0x0003; }, "unknown_r16");

		map(0x00f80000, 0x00f8ffff).rom().region("eprom", 0);

		map(0x00fff914, 0x00fff914).lw8([this](u8 data) { m_boot.select(1); }, "men_w"); // ram enable?
	}

	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask32(0x000000ff);
}
