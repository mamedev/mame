// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Artecon SB-300P 3-serial 1-parallel SBus card skeleton

    The Artecon SB series of SBus cards uses up to 4 Cirrus Logic
    CL-CD1400 Four-Channel Serial/Parallel Communications Engines.

    Each chip supports up to four full-duplex serial channels, or three
    full-duplex serial channels and one high-speed bidirectional parallel
    channel.

***************************************************************************/

#include "emu.h"
#include "artecon.h"

DEFINE_DEVICE_TYPE(SBUS_SB300P, sbus_artecon_device, "sb300p", "Artecon SB-300P 3S/1P controller")

void sbus_artecon_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_artecon_device::unknown_r), FUNC(sbus_artecon_device::unknown_w));
	map(0x00000000, 0x00003fff).r(FUNC(sbus_artecon_device::rom_r));
}

ROM_START( sbus_artecon )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "artecon_sbus_port.bin", 0x0000, 0x4000, CRC(bced6981) SHA1(1c6006fb8cb555eff0cb7c2783c776d05c6797f8))
ROM_END

const tiny_rom_entry *sbus_artecon_device::device_rom_region() const
{
	return ROM_NAME( sbus_artecon );
}

void sbus_artecon_device::device_add_mconfig(machine_config &config)
{
}


sbus_artecon_device::sbus_artecon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_SB300P, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
{
}

void sbus_artecon_device::device_start()
{
}

void sbus_artecon_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_artecon_device::mem_map);
}

READ32_MEMBER(sbus_artecon_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_artecon_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

READ32_MEMBER(sbus_artecon_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}
