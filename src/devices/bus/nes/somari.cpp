// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Somari Team PCBs


 Here we emulate the Somari Team PCBs [mapper 116]


 ***********************************************************************************************************/


#include "emu.h"
#include "somari.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)

#define SOMARI_VRC2_MODE 0
#define SOMARI_MMC3_MODE 1
#define SOMARI_MMC1_MODE 2


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SOMARI, nes_somari_device, "nes_somari", "NES Cart Team Somari PCB")
DEFINE_DEVICE_TYPE(NES_HUANG2, nes_huang2_device, "nes_huang2", "NES Cart Huang-2 PCB")


nes_somari_device::nes_somari_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 mmc1_prg_shift)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_board_mode(0), m_mmc1_count(0), m_mmc1_latch(0), m_mmc1_prg_shift(mmc1_prg_shift), m_vrc_mirror(0)
{
}

nes_somari_device::nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_somari_device(mconfig, NES_SOMARI, tag, owner, clock, 0)
{
}

nes_huang2_device::nes_huang2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_somari_device(mconfig, NES_HUANG2, tag, owner, clock, 1)
{
}



void nes_somari_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_board_mode));

	// MMC1
	save_item(NAME(m_mmc1_count));
	save_item(NAME(m_mmc1_latch));
	save_item(NAME(m_mmc1_reg));

	// VRC2
	save_item(NAME(m_vrc_prg_bank));
	save_item(NAME(m_vrc_vrom_bank));
	save_item(NAME(m_vrc_mirror));
}

void nes_somari_device::pcb_reset()
{
	m_board_mode = SOMARI_MMC3_MODE;

	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0xff, 0xff, 0);

	// MMC1 regs
	m_mmc1_count = 0;
	m_mmc1_latch = 0;
	m_mmc1_reg[0] = 0x0c;
	m_mmc1_reg[1] = 0x00;
	m_mmc1_reg[2] = 0x00;
	m_mmc1_reg[3] = 0x00;

	// VRC2 regs
	m_vrc_prg_bank[0] = m_vrc_prg_bank[1] = 0;
	m_vrc_mirror = 0;
	std::fill(std::begin(m_vrc_vrom_bank), std::end(m_vrc_vrom_bank), 0xff);  // Somari (original release) glitches on "Somari Team Presents" screen without this
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 SOMERI TEAM

 Games: Somari, Kart Fighter, Garou Densetsu Special,
 AV Bishoujo Senshi, AV Kyuukyoku Mahjong 2

 Like Rex Soft's SL-1632 these boards use Huang chips
 which simulate the behavior of MMC1, MMC3, and VRC2.
 There is a second revision, Huang-2, that appears to
 only have been used for AV Kyuukyoku Mahjong 2. Its
 MMC1 mode has nonstandard PRG banking.

 iNES: mapper 116


 Emulation note about regs in MAME: currently,
 - m_mmc_prg_bank[n] for n=0,...,3 represent the MMC3 PRG banks (inherited from base class)
 - m_mmc_vrom_bank[n] for n=0,...,5 represent the MMC3 CHR banks (inherited from base class)

 - m_mmc1_reg[n] for n=0,1,2,3 represent the MMC1 regs
 - m_mmc1_count and m_mmc1_latch are additional variables for MMC1 (notice that MMC3 uses a diff m_latch!)

 - m_vrc_prg_bank[n] for n=0,1 represent the VRC2 PRG banks
 - m_vrc_vrom_bank[n] for n=0,...,7 represent the VRC2 CHR banks


 In MAME: Supported.

 -------------------------------------------------*/

// MMC1 Mode emulation
void nes_somari_device::mmc1_reset_latch()
{
	m_mmc1_count = 0;
	m_mmc1_reg[0] |= 0x0c;
	update_prg();
}

void nes_somari_device::mmc1_w(offs_t offset, u8 data)
{
	if (BIT(data, 7))
		mmc1_reset_latch();
	else
	{
		m_mmc1_latch = (data & 1) << 4 | m_mmc1_latch >> 1;

		m_mmc1_count = (m_mmc1_count + 1) % 5;
		if (!m_mmc1_count)
		{
			m_mmc1_reg[(offset & 0x6000) >> 13] = m_mmc1_latch;
			update_all_banks();
		}
	}
}

// VRC2 Mode emulation
void nes_somari_device::vrc2_w(offs_t offset, u8 data)
{
	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			m_vrc_prg_bank[BIT(offset, 13)] = data & 0x1f;
			update_prg();
			break;
		case 0x1000:
			m_vrc_mirror = data;
			update_mirror();
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			u8 bank = ((offset >> 12) - 3) * 2 + BIT(offset, 1);
			u8 shift = (offset & 1) << 2;
			u8 mask = 0x0f << shift;
			m_vrc_vrom_bank[bank] = (m_vrc_vrom_bank[bank] & ~mask) | ((data << shift) & mask);
			update_chr();
			break;
		}
	}
}

void nes_somari_device::update_prg()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			prg8_89(m_vrc_prg_bank[0]);
			prg8_ab(m_vrc_prg_bank[1]);
			prg16_cdef(m_prg_chunks - 1);
			break;
		case SOMARI_MMC3_MODE:
			set_prg(m_prg_base, m_prg_mask);
			break;
		case SOMARI_MMC1_MODE:
			switch ((m_mmc1_reg[0] >> 2) & 3)
			{
				case 0:
				case 1:
					prg32(m_mmc1_reg[3] >> 1);
					break;
				case 2:
					prg16_89ab(0);
					prg16_cdef(m_mmc1_reg[3] >> m_mmc1_prg_shift);
					break;
				case 3:
					prg16_89ab(m_mmc1_reg[3] >> m_mmc1_prg_shift);
					prg16_cdef(0x0f >> m_mmc1_prg_shift);
					break;
			}
			break;
	}
}

void nes_somari_device::update_chr()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			for (int i = 0; i < 8; i++)
				chr1_x(i, m_chr_base | m_vrc_vrom_bank[i], CHRROM);
			break;
		case SOMARI_MMC3_MODE:
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		case SOMARI_MMC1_MODE:
			if (BIT(m_mmc1_reg[0], 4))
			{
				chr4_0(m_mmc1_reg[1] & 0x1f, CHRROM);
				chr4_4(m_mmc1_reg[2] & 0x1f, CHRROM);
			}
			else
				chr8((m_mmc1_reg[1] & 0x1f) >> 1, CHRROM);
			break;
	}
}

void nes_somari_device::update_mirror()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			set_nt_mirroring(m_vrc_mirror & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case SOMARI_MMC3_MODE:
			set_nt_mirroring(m_mmc_mirror & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case SOMARI_MMC1_MODE:
			switch (m_mmc1_reg[0] & 0x03)
			{
				case 0x00: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			}
			break;
	}
}

void nes_somari_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("somari write_h, mode %d, offset: %04x, data: %02x\n", m_board_mode, offset, data));

	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE: vrc2_w(offset, data); break;
		case SOMARI_MMC3_MODE: txrom_write(offset, data); break;
		case SOMARI_MMC1_MODE: mmc1_w(offset, data); break;
	}
}

void nes_somari_device::update_all_banks()
{
	update_mirror();
	update_prg();
	update_chr();
}

void nes_somari_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("somari write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x100)
	{
		m_board_mode = std::min(data & 0x03, SOMARI_MMC1_MODE);
		if (m_board_mode == SOMARI_MMC1_MODE)
			mmc1_reset_latch();  // Garou crashes between fights without this

		m_chr_base = (data & 0x04) << 6;

		if (m_board_mode != SOMARI_MMC3_MODE)
		{
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
		}

		update_all_banks();
	}
}
