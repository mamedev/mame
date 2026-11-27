// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Henggedianzi PCBs


 Here we emulate the following PCBs

 * Henggedianzi Super Rich [mapper 177]
 * Henggedianzi Xing He Zhan Shi [mapper 179]


 TODO:
 - investigate relation with some TXC & Waixing boards

 ***********************************************************************************************************/


#include "emu.h"
#include "henggedianzi.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_HENGG_SRICH, nes_hengg_srich_device, "nes_hengg_srich", "NES Cart Henggedianzi Super Rich PCB")
DEFINE_DEVICE_TYPE(NES_HENGG_XHZS,  nes_hengg_xhzs_device,  "nes_hengg_xhzs",  "NES Cart Henggedianzi Xing He Zhan Shi PCB")


nes_hengg_srich_device::nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_HENGG_SRICH, tag, owner, clock), m_ram_bank(0)
{
}

nes_hengg_xhzs_device::nes_hengg_xhzs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_HENGG_XHZS, tag, owner, clock)
{
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Henggedianzi

 Games: Mei Guo Fu Hao, Shang Gu Shen Jian , Wang Zi Fu
 Chou Ji

 NESdev INES Mapper 177: an oversize BNROM without bus
 conflicts, with battery-backed WRAM at $6000-$7FFF. A write to
 $8000-$FFFF (WWMP PPPP) selects the 32 KiB PRG-ROM bank (PPPPP),
 the nametable mirroring (M: 0 vertical, 1 horizontal) and the 8 KiB
 PRG-RAM bank (WW; it has no effect when the RAM is 8 KiB). The
 registers power up as 0 (not documented).

 TODO: submapper 1 (a 16 KiB CHR-RAM mode in place of the mirroring
 bit) and the LPC speech chip of one educational cartridge are not
 emulated.

 iNES: mapper 177

 In MAME: Supported.

 -------------------------------------------------*/

void nes_hengg_srich_device::device_start()
{
	common_start();
	save_item(NAME(m_ram_bank));
}

void nes_hengg_srich_device::pcb_reset()
{
	nes_nrom_device::pcb_reset();
	m_ram_bank = 0;
}

// the 8 KiB PRG-RAM bank at $6000-$7FFF is selected by bits 7-6 of the register
uint32_t nes_hengg_srich_device::ram_index(offs_t offset, size_t size) const
{
	return ((m_ram_bank << 13) | (offset & 0x1fff)) & (size - 1);
}

uint8_t nes_hengg_srich_device::read_m(offs_t offset)
{
	if (!m_battery.empty())
		return m_battery[ram_index(offset, m_battery.size())];
	if (!m_prgram.empty())
		return m_prgram[ram_index(offset, m_prgram.size())];
	return get_open_bus();
}

void nes_hengg_srich_device::write_m(offs_t offset, uint8_t data)
{
	if (!m_battery.empty())
		m_battery[ram_index(offset, m_battery.size())] = data;
	if (!m_prgram.empty())
		m_prgram[ram_index(offset, m_prgram.size())] = data;
}

void nes_hengg_srich_device::write_h(offs_t offset, uint8_t data)
{
	LOG("hengg_srich write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(data & 0x1f);
	m_ram_bank = data >> 6;
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Bootleg Board by Henggedianzi

 Games: Xing He Zhan Shi

 Writes to 0x5000-0x5fff set prg32 banks, writes to 0x8000-
 0xffff set NT mirroring

 Note: NEStopia marks this as Xjzb, but Xing Ji Zheng Ba
 (Phantasy Star?) runs on the other Henggedianzi board
 Is there an alt dump of Xing Ji Zheng Ba using this?

 iNES: mapper 179

 In MAME: Supported.

 -------------------------------------------------*/

void nes_hengg_xhzs_device::write_l(offs_t offset, uint8_t data)
{
	LOG("hengg_xhzs write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x4100;

	// the register answers at $5000-$5FFF only (offset & $5000 was true for every $4100-$4FFF address as well)
	if ((offset & 0xf000) == 0x5000)
		prg32(data >> 1);
}

void nes_hengg_xhzs_device::write_h(offs_t offset, uint8_t data)
{
	LOG("hengg_xhzs write_h, offset: %04x, data: %02x\n", offset, data);

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}
