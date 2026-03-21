// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#include "emu.h"
#include "luna_68k_cmc.h"

luna_68k_cmc_device::luna_68k_cmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LUNA_68K_CMC, tag, owner, clock)
	, m_cpu(*this, "cpu")
	, m_eth(*this, "lance")
	, m_gpib(*this, "gpib")
	, m_stc(*this, "stc")
	, m_scc1(*this, "scc1")
	, m_scc2(*this, "scc2")
	, m_boot(*this, "boot")
{
}

void luna_68k_cmc_device::device_start()
{
}

void luna_68k_cmc_device::device_reset()
{
	m_boot.select(0);
}
// There's no reset opcode in the cmc case, ram write seems to be the
// the most obvious option.  But it could also be something with
// whatever is at e00000.
void luna_68k_cmc_device::boot_w(offs_t offset, u32 data)
{
	m_boot.disable();
	m_cpu->space(AS_PROGRAM).write_dword(offset, data);
}

void luna_68k_cmc_device::cpu_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram(); // NEC D43256AC-10L x 8 (32768x8)
	map(0x00000000, 0x00000007).view(m_boot);
	m_boot[0](0x00000000, 0x00000007).rom().region("cmc", 0).w(FUNC(luna_68k_cmc_device::boot_w));

	map(0x00400000, 0x00407fff).rom().region("cmc", 0);
}

void luna_68k_cmc_device::vme_map(address_map &map)
{
}

void luna_68k_cmc_device::device_add_mconfig(machine_config &config)
{
	M68020(config, m_cpu, 25_MHz_XTAL/2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_cmc_device::cpu_map);

	AM7990(config, m_eth); // Linked to an AM7992BCD
	TMS9914(config, m_gpib, 25_MHz_XTAL/5);
	AM9513(config, m_stc, 25_MHz_XTAL/2); // FIXME: clock unknown

	SCC8530(config, m_scc1, 4.9152_MHz_XTAL); // AM8530H-6PC
	SCC8530(config, m_scc2, 4.9152_MHz_XTAL); // AM8530H-6PC
	m_scc1->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	m_scc2->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
}

ROM_START( luna_68k_cmc )
	ROM_REGION32_BE(0x8000, "cmc", 0)
	ROM_LOAD("8112_v1_1.ic54", 0x0000, 0x8000, CRC(b87e0122) SHA1(22290850761ed3dddb2369e062012679e2963fa3))
ROM_END

const tiny_rom_entry *luna_68k_cmc_device::device_rom_region() const
{
	return ROM_NAME(luna_68k_cmc);
}

DEFINE_DEVICE_TYPE(LUNA_68K_CMC, luna_68k_cmc_device, "luna_68k_cmc", "Omron Luna 68k CMC VME board")
