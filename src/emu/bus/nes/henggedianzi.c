// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Henggedianzi PCBs


 Here we emulate the following PCBs

 * Henggedianzi Super Rich [mapper 177]
 * Henggedianzi Xing He Zhan Shi [mapper 179]
 * Henggedianzi Shen Hua Jian Yun III


 TODO:
 - investigate relation with some TXC & Waixing boards

 ***********************************************************************************************************/


#include "emu.h"
#include "henggedianzi.h"

#include "cpu/m6502/m6502.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_HENGG_SRICH = &device_creator<nes_hengg_srich_device>;
const device_type NES_HENGG_XHZS = &device_creator<nes_hengg_xhzs_device>;
const device_type NES_HENGG_SHJY3 = &device_creator<nes_hengg_shjy3_device>;


nes_hengg_srich_device::nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_HENGG_SRICH, "NES Cart Henggedianzi Super Rich PCB", tag, owner, clock, "nes_hengg_srich", __FILE__)
{
}

nes_hengg_xhzs_device::nes_hengg_xhzs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_HENGG_XHZS, "NES Cart Henggedianzi Xing He Zhan Shi PCB", tag, owner, clock, "nes_hengg_xhzs", __FILE__)
{
}

nes_hengg_shjy3_device::nes_hengg_shjy3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_HENGG_SHJY3, "NES Cart Henggedianzi Shen Hua Jian Yun III PCB", tag, owner, clock, "nes_hengg_shjy3", __FILE__)
{
}




void nes_hengg_srich_device::device_start()
{
	common_start();
}

void nes_hengg_srich_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_hengg_xhzs_device::device_start()
{
	common_start();
}

void nes_hengg_xhzs_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_hengg_shjy3_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_chr_mode));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_mmc_extra_bank));
}

void nes_hengg_shjy3_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;

	m_chr_mode = 0;
	memset(m_mmc_prg_bank, 0, sizeof(m_mmc_prg_bank));
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
	memset(m_mmc_extra_bank, 0, sizeof(m_mmc_extra_bank));
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

WRITE8_MEMBER(nes_hengg_srich_device::write_h)
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

WRITE8_MEMBER(nes_hengg_xhzs_device::write_l)
{
	LOG_MMC(("hengg_xhzs write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if (offset & 0x5000)
		prg32(data >> 1);
}

WRITE8_MEMBER(nes_hengg_xhzs_device::write_h)
{
	LOG_MMC(("hengg_xhzs write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 UNL-SHJY3

 -------------------------------------------------*/

/* I think the IRQ should only get fired if enough CPU cycles have passed, but we don't implement (yet) this part */
void nes_hengg_shjy3_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (m_irq_enable & 0x02)
	{
		if (m_irq_count == 0xff)
		{
			m_irq_count = m_irq_count_latch;
			m_irq_enable = m_irq_enable | ((m_irq_enable & 0x01) << 1);
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			m_irq_count++;
	}
}

void nes_hengg_shjy3_device::update_banks()
{
	prg8_89(m_mmc_prg_bank[0]);
	prg8_ab(m_mmc_prg_bank[1]);

	for (int i = 0; i < 8; i++)
	{
		UINT8 chr_bank = m_mmc_vrom_bank[i] | (m_mmc_extra_bank[i] << 4);
		if (m_mmc_vrom_bank[i] == 0xc8)
		{
			m_chr_mode = 0;
			continue;
		}
		else if (m_mmc_vrom_bank[i] == 0x88)
		{
			m_chr_mode = 1;
			continue;
		}
		if ((m_mmc_vrom_bank[i] == 4 || m_mmc_vrom_bank[i] == 5) && !m_chr_mode)
			chr1_x(i, chr_bank & 1, CHRRAM);
		else
			chr1_x(i, chr_bank, CHRROM);
	}
}

WRITE8_MEMBER(nes_hengg_shjy3_device::write_h)
{
	LOG_MMC(("shjy3 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3000 && offset <= 0x600c)
	{
		UINT8 shift = offset & 4;
		UINT8 mmc_helper = ((offset & 8) | (offset >> 8)) >> 3;
		mmc_helper += 2;
		mmc_helper &= 7;

		m_mmc_vrom_bank[mmc_helper] = (m_mmc_vrom_bank[mmc_helper] & (0xf0 >> shift)) | ((data & 0x0f) << shift);
		if (shift)
			m_mmc_extra_bank[mmc_helper] = data >> 4;
	}
	else
	{
		switch (offset)
		{
			case 0x0010:
				m_mmc_prg_bank[0] = data;
				break;
			case 0x2010:
				m_mmc_prg_bank[1] = data;
				break;
			case 0x1400:
				switch (data & 0x03)
				{
					case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
					case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
					case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
					case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
				break;
			case 0x7000:
				m_irq_count_latch = (m_irq_count_latch & 0xf0) | (data & 0x0f);
				break;
			case 0x7004:
				m_irq_count_latch = (m_irq_count_latch & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x7008:
				m_irq_enable = data & 0x03;
				if (m_irq_enable & 0x02)
					m_irq_count = m_irq_count_latch;
				break;
		}
	}
	update_banks();
}
