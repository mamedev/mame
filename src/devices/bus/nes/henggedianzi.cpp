// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
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
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_HENGG_SRICH, nes_hengg_srich_device, "nes_hengg_srich", "NES Cart Henggedianzi Super Rich PCB")
DEFINE_DEVICE_TYPE(NES_HENGG_XHZS,  nes_hengg_xhzs_device,  "nes_hengg_xhzs",  "NES Cart Henggedianzi Xing He Zhan Shi PCB")


nes_hengg_srich_device::nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_HENGG_SRICH, tag, owner, clock)
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

 Writes to 0x8000-0xffff set prg32. Moreover, data&0x20 sets
 NT mirroring.

 iNES: mapper 177

 In MESS: Supported.

 -------------------------------------------------*/

void nes_hengg_srich_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("hengg_srich write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
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

 In MESS: Supported.

 -------------------------------------------------*/

void nes_hengg_xhzs_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("hengg_xhzs write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if (offset & 0x5000)
		prg32(data >> 1);
}

void nes_hengg_xhzs_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("hengg_xhzs write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}
