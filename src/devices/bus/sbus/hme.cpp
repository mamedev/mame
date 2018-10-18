// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun SunSwift 10/100 + Fast Wide SCSI "Colossus" skeleton

    Notable parts on board:
    - 1x 32-pin PLCC ROM, label 525 / 1409 / -08 on separate lines
    - 1x Sun STP2002QFP, marked 100-4156-05 / 609-0392458 / DP03972
    - 1x National Semiconductor DP83840AVCE-1 Ethernet Physical Layer
    - 1x National Semiconductor DP83223V Twisted Pair Transceiver

***************************************************************************/

#include "emu.h"
#include "hme.h"

DEFINE_DEVICE_TYPE(SBUS_HME, sbus_hme_device, "sbus_hme", "Sun 10/100 + Fast Wide SCSI")

void sbus_hme_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_hme_device::unknown_r), FUNC(sbus_hme_device::unknown_w));
	map(0x00000000, 0x0000ffff).r(FUNC(sbus_hme_device::rom_r));
}

ROM_START( sbus_hme )
	ROM_REGION32_BE(0x10000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "525 1409 -08.bin", 0x00000, 0x10000, CRC(10f0b28f) SHA1(b54bb0f01c45accdbc58c3a86f8de34949374880))
ROM_END

const tiny_rom_entry *sbus_hme_device::device_rom_region() const
{
	return ROM_NAME( sbus_hme );
}

void sbus_hme_device::device_add_mconfig(machine_config &config)
{
}


sbus_hme_device::sbus_hme_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_HME, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
{
}

void sbus_hme_device::device_start()
{
}

void sbus_hme_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_hme_device::mem_map);
}

READ32_MEMBER(sbus_hme_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_hme_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

READ32_MEMBER(sbus_hme_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}
