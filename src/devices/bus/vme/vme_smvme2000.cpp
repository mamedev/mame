// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Signetics SMVME2000 VMEbus CPU Module
 *
 * Probably also sold as the Motorola MVME115M with different firmware, 68010 and MMU.
 *
 * Sources:
 *  - http://bitsavers.org/components/signetics/_dataBooks/1986_Signetics_Microprocessor.pdf
 *
 * TODO:
 *  - interrupts and configuration
 *  - VME interface
 *  - MMU
 */

#include "emu.h"
#include "vme_smvme2000.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_SMVME2000, vme_smvme2000_device, "smvme2000", "Signetics SMVME2000")

vme_smvme2000_device::vme_smvme2000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_SMVME2000, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	//, m_mmu(*this, "mmu")
	, m_duart(*this, "duart")
	, m_pit(*this, "pit")
	, m_serial(*this, "serial%u", 0U)
	, m_fail(*this, "fail")
	, m_eprom(*this, "eprom")
	, m_ram(*this, "ram")
{
}

ROM_START(smvme2000)
	ROM_REGION16_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v11", "SIGbug Version 1.1 Copyright 1983 Signetics")
	ROMX_LOAD("sigbug__1.bin", 0x0000, 0x4000, CRC(ae23c2c3) SHA1(da422ca03ac77d9aed2067f1099ec0fb96939fc8), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("sigbug__2.bin", 0x0001, 0x4000, CRC(e39bd1db) SHA1(2f610170f118c74477f4586e19aee0fc63803e17), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("sigbug__3.bin", 0x8000, 0x4000, CRC(182998ef) SHA1(ecf980b7dd8bd0286f002b31cd726798f84a2267), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("sigbug__4.bin", 0x8001, 0x4000, CRC(f0e25380) SHA1(78427f7b9a6f676945c2cdb90632a16a619c83f9), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(smvme2000)
INPUT_PORTS_END

const tiny_rom_entry *vme_smvme2000_device::device_rom_region() const
{
	return ROM_NAME(smvme2000);
}

ioport_constructor vme_smvme2000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(smvme2000);
}

void vme_smvme2000_device::device_start()
{
	m_fail.resolve();
}

void vme_smvme2000_device::device_reset()
{
	/*
	 * FIXME: EPROM is mapped at the start of memory during reset to allow the
	 * CPU to fetch the stack pointer and initial PC. The exact mechanism used
	 * is unknown; current emulation unmaps the EPROM and maps RAM when the last
	 * two bytes of the reset vector are fetched.
	 */
	m_cpu->space(AS_PROGRAM).install_rom(0, m_eprom.bytes() - 1, m_eprom.target());

	m_boot = m_cpu->space(AS_PROGRAM).install_read_tap(0x000006, 0x000007, "boot",
		[this](offs_t offset, u16 &data, u16 mem_mask)
		{
			if (!machine().side_effects_disabled())
			{
				m_cpu->space(AS_PROGRAM).unmap_readwrite(0, m_eprom.bytes() - 1);
				m_cpu->space(AS_PROGRAM).install_ram(0, m_ram.bytes() - 1, m_ram.target());

				m_boot.remove();
			}
		});
}

void vme_smvme2000_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_smvme2000_device::cpu_mem);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_smvme2000_device::cpu_int);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(m_pit, FUNC(pit68230_device::h3_w)); // TODO: verify destination

	PIT68230(config, m_pit, 0);
	m_pit->pc_out_callback().set(
		[this](u8 data)
		{
			m_fail = BIT(data, 4);
		});
	m_pit->port_irq_callback().set_inputline(m_cpu, M68K_IRQ_5); // TODO: verify destination

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// TODO: dsr, dtr
	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void vme_smvme2000_device::cpu_mem(address_map &map)
{
	map(0x000000, 0x000fff).ram().share("ram");

	//map(0x010000, 0xf6ffff); // VMEbus memory address space
	map(0xf70000, 0xf7ffff).rom().region("eprom", 0);
	//map(0xf80000, 0xf8003f); // MMU
	map(0xf81000, 0xf8103f).rw(m_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
	map(0xf82000, 0xf8201f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	//map(0xf90000, 0xf9ffff).mirror(0x060000); // VMEbus short I/O space
}

void vme_smvme2000_device::cpu_int(address_map &map)
{
	map(0xfffff3, 0xfffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffff5, 0xfffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffff7, 0xfffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffff9, 0xfffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffb, 0xfffffb).lr8(NAME([]() { return m68000_base_device::autovector(5); })); // TODO: m_pit->irq_piack()?
	map(0xfffffd, 0xfffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffff, 0xffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}
