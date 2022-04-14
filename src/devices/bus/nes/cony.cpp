// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Cony/Yoko PCBs


 Here we emulate the following PCBs

 * UNL-CONY [mapper 83]
 * UNL-YOKO [mapper 264]

 ***********************************************************************************************************/


#include "emu.h"
#include "cony.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_CONY,   nes_cony_device,   "nes_cony",   "NES Cart Cony PCB")
DEFINE_DEVICE_TYPE(NES_CONY1K, nes_cony1k_device, "nes_cony1k", "NES Cart Cony 1K PCB")
DEFINE_DEVICE_TYPE(NES_YOKO,   nes_yoko_device,   "nes_yoko",   "NES Cart Yoko PCB")


nes_cony_device::nes_cony_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 extra_addr, u8 mask)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_extra_addr(extra_addr), m_mask(mask), m_mode_reg(0), m_outer_reg(0)
{
}

nes_cony_device::nes_cony_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_cony_device(mconfig, NES_CONY, tag, owner, clock, 0x1100, 0x1f)
{
}

nes_cony1k_device::nes_cony1k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_cony_device(mconfig, NES_CONY1K, tag, owner, clock, 0x1100, 0x1f)
{
}

nes_yoko_device::nes_yoko_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_cony_device(mconfig, NES_YOKO, tag, owner, clock, 0x1400, 0x0f)
{
}




void nes_cony_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));

	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_extra_ram));
	save_item(NAME(m_mode_reg));
	save_item(NAME(m_outer_reg));
}

void nes_cony_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);

	m_irq_enable = 0;
	m_irq_count = 0;

	m_mode_reg = 0;
	m_outer_reg = 0;

	std::fill(std::begin(m_extra_ram), std::end(m_extra_ram), 0x00);
	std::fill(std::begin(m_mmc_prg_bank), std::end(m_mmc_prg_bank), 0x00);
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Cony Bootleg Board

 Games: Dragon Ball Party, Fatal Fury 2, Street Blaster II
 Pro, World Heroes 2

 iNES: mapper 83

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cony_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count += BIT(m_mode_reg, 6) ? -1 : 1;
			if (!m_irq_count)
			{
				set_irq_line(ASSERT_LINE);
				m_irq_enable = 0;
			}
		}
	}
}

void nes_cony_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("cony write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (offset >= m_extra_addr) // scratch ram from 0x5100 or 0x5400
		m_extra_ram[offset & 0x03] = data;
}

u8 nes_cony_device::read_l(offs_t offset)
{
	LOG_MMC(("cony read_l, offset: %04x\n", offset));

	offset += 0x100;
	if (offset >= m_extra_addr) // scratch ram from 0x5100 or 0x5400
		return m_extra_ram[offset & 0x03];
	else if (offset >= 0x1000)   // 0x5000
		return m_extra_addr >> 10; // FIXME: this should be 2-bit jumper settings; certain games glitch with certain values here
	else
		return get_open_bus();
}

void nes_cony_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("cony write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery.empty())
		m_battery[((m_outer_reg >> 6) * 0x2000 + offset) & (m_battery.size() - 1)] = data;
}

u8 nes_cony_device::read_m(offs_t offset)
{
	LOG_MMC(("cony read_m, offset: %04x\n", offset));

	if (!m_battery.empty())
		return m_battery[((m_outer_reg >> 6) * 0x2000 + offset) & (m_battery.size() - 1)];
	else if (BIT(m_mode_reg, 5))
		return m_prg[(m_mmc_prg_bank[3] * 0x2000 + offset) & (m_prg_size - 1)];
	else
		return get_open_bus();
}

void nes_cony_device::set_prg()
{
	switch (m_mode_reg & 0x18)
	{
		case 0x00:
			prg16_89ab(m_outer_reg);
			prg16_cdef(m_outer_reg | m_mask >> 1);
			break;
		case 0x08:
			prg32(m_outer_reg >> 1);
			break;
		case 0x10:
		case 0x18:
		{
			int base = (m_outer_reg << 1) & ~m_mask;
			prg8_89(base | (m_mmc_prg_bank[0] & m_mask));
			prg8_ab(base | (m_mmc_prg_bank[1] & m_mask));
			prg8_cd(base | (m_mmc_prg_bank[2] & m_mask));
			prg8_ef(base | m_mask);
			break;
		}
	}
}

void nes_cony_device::set_chr()
{
	chr2_0(m_mmc_vrom_bank[0], CHRROM);
	chr2_2(m_mmc_vrom_bank[1], CHRROM);
	chr2_4(m_mmc_vrom_bank[6], CHRROM);
	chr2_6(m_mmc_vrom_bank[7], CHRROM);
}

void nes_cony_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("cony write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0300)
	{
		case 0x0000:
			m_outer_reg = data;
			set_prg();
			set_chr();
			break;
		case 0x0100:
			m_mode_reg = data;
			set_prg();
			switch (data & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x0200:
			if (offset & 1)
			{
				m_irq_enable = BIT(m_mode_reg, 7);
				m_irq_count = data << 8 | (m_irq_count & 0x00ff);
			}
			else
			{
				m_irq_count = (m_irq_count & 0xff00) | data;
				set_irq_line(CLEAR_LINE);
			}
			break;
		case 0x0300:
			switch (offset & 0x18)
			{
				case 0x00:
				case 0x08:
					m_mmc_prg_bank[offset & 3] = data;
					set_prg();
					break;
				case 0x10:
					m_mmc_vrom_bank[offset & 7] = data;
					set_chr();
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 Cony Bootleg Board alternate 1K CHR banking

 -------------------------------------------------*/

void nes_cony1k_device::set_chr()
{
	for (int i = 0; i < 8; i++)
		chr1_x(i, m_mmc_vrom_bank[i] | (m_outer_reg & 0x30) << 4, CHRROM);
}

/*-------------------------------------------------

 Yoko Bootleg Board

 Games: Mortal Kombat II, Master Fighter VI'

 Seems to be the same as the Cony board (2K CHR
 banking version) but with address lines A10/A11
 swapped in for Cony address lines A8/A9.

 NES 2.0: mapper 264

 In MAME: Supported.

 -------------------------------------------------*/

void nes_yoko_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("yoko write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & 0xf0ff) | (offset & 0x0c00) >> 2;
	nes_cony_device::write_h(offset, data);
}
