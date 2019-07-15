// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun SunPC 5x86 Accelerator (501-4230) skeleton

    Notable parts on board:
    - 1x AMD AM27C256 PLCC ROM
    - 1x Motorola SunPC Accelerator 100-3069-03, mfr/date AANL9732
    - 6x Cypress CY7B185-10VC 64kBit Static RAM
    - 1x AMD 5x86 (under heatsink; markings unknown)

***************************************************************************/

#include "emu.h"
#include "sunpc.h"

DEFINE_DEVICE_TYPE(SBUS_SUNPC, sbus_sunpc_device, "sbus_sunpc", "Sun SunPC accelerator")

void sbus_sunpc_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_sunpc_device::unknown_r), FUNC(sbus_sunpc_device::unknown_w));
	map(0x00000000, 0x00007fff).r(FUNC(sbus_sunpc_device::rom_r));
}

ROM_START( sbus_sunpc )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-1763-01.bin", 0x0000, 0x8000, CRC(171f50f8) SHA1(21c4c02bc5a3a0494301f19c54ba0e207568fb42))
ROM_END

const tiny_rom_entry *sbus_sunpc_device::device_rom_region() const
{
	return ROM_NAME( sbus_sunpc );
}

void sbus_sunpc_device::device_add_mconfig(machine_config &config)
{
}


sbus_sunpc_device::sbus_sunpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_SUNPC, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
{
}

void sbus_sunpc_device::device_start()
{
}

void sbus_sunpc_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_sunpc_device::mem_map);
}

READ32_MEMBER(sbus_sunpc_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_sunpc_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

READ32_MEMBER(sbus_sunpc_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}
