// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Somari Team PCBs


 Here we emulate the Somari Team PCBs [mapper 116]


 ***********************************************************************************************************/


#include "emu.h"
#include "somari.h"

#include "cpu/m6502/m6502.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)

#define SOMARI_VRC2_MODE 0
#define SOMARI_MMC3_MODE 1
#define SOMARI_MMC1_MODE 2
#define SOMARI_MMC1_MODE_AGAIN 3


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_SOMARI = &device_creator<nes_somari_device>;


nes_somari_device::nes_somari_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SOMARI, "NES Cart Team Somari PCB", tag, owner, clock, "nes_somari", __FILE__),
	m_board_mode(0),
	m_mmc3_mirror_reg(0),
	m_count(0),
	m_mmc1_latch(0),
	m_vrc_mirror_reg(0)
				{
}



void nes_somari_device::device_start()
{
	common_start();
	save_item(NAME(m_board_mode));

	// MMC3
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));
	save_item(NAME(m_prg_base));
	save_item(NAME(m_prg_mask));
	save_item(NAME(m_chr_base));
	save_item(NAME(m_chr_mask));
	save_item(NAME(m_mmc3_mirror_reg));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_clear));

	// MMC1
	save_item(NAME(m_count));
	save_item(NAME(m_mmc1_latch));
	save_item(NAME(m_mmc1_reg));

	// VRC2
	save_item(NAME(m_vrc_prg_bank));
	save_item(NAME(m_vrc_vrom_bank));
	save_item(NAME(m_vrc_mirror_reg));
}

void nes_somari_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_board_mode = 2; // mode

	// MMC3
	m_prg_base = m_chr_base = 0;
	m_prg_mask = 0xff;
	m_chr_mask = 0xff;
	m_latch = 0;
	m_mmc_prg_bank[0] = 0x3c;
	m_mmc_prg_bank[1] = 0x3d;
	m_mmc_prg_bank[2] = 0x3e;
	m_mmc_prg_bank[3] = 0x3f;
	m_mmc_vrom_bank[0] = 0x00;
	m_mmc_vrom_bank[1] = 0x01;
	m_mmc_vrom_bank[2] = 0x04;
	m_mmc_vrom_bank[3] = 0x05;
	m_mmc_vrom_bank[4] = 0x06;
	m_mmc_vrom_bank[5] = 0x07;
	m_mmc3_mirror_reg = 0;

	m_alt_irq = 0;
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
	m_irq_clear = 0;

	// MMC1 regs
	m_count = 0;
	m_mmc1_latch = 0;
	m_mmc1_reg[0] = 0x0c;
	m_mmc1_reg[1] = 0x00;
	m_mmc1_reg[2] = 0x00;
	m_mmc1_reg[3] = 0x00;

	// VRC2 regs
	m_vrc_prg_bank[0] = 0x00;
	m_vrc_prg_bank[1] = 0x01;
	for (int i = 0; i < 8; ++i)
		m_vrc_vrom_bank[i] = i;
	bank_update_switchmode();
	m_vrc_mirror_reg = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 SOMERI TEAM

 iNES: mapper 116


 Emulation note about regs in MESS: currently,
 - m_mmc_prg_bank[n] for n=0,...,3 represent the MMC3 PRG banks (inherited from base class)
 - m_mmc_vrom_bank[n] for n=0,...,5 represent the MMC3 CHR banks (inherited from base class)

 - m_mmc1_reg[n] for n=0,1,2,3 represent the MMC1 regs
 - m_count and m_mmc1_latch are additional variables for MMC1 (notice that MMC3 uses a diff m_latch!)

 - m_vrc_prg_bank[n] for n=0,1 represent the VRC2 PRG banks
 - m_vrc_vrom_bank[n] for n=0,...,7 represent the VRC2 CHR banks


 In MESS: Preliminary support

 -------------------------------------------------*/

// MMC1 Mode emulation
WRITE8_MEMBER(nes_somari_device::mmc1_w)
{
	assert(m_board_mode == 2);

	if (data & 0x80)
	{
		m_count = 0;
		m_mmc1_latch = 0;

		m_mmc1_reg[0] |= 0x0c;
		update_prg();
		return;
	}

	if (m_count < 5)
	{
		if (m_count == 0) m_mmc1_latch = 0;
		m_mmc1_latch >>= 1;
		m_mmc1_latch |= (data & 0x01) ? 0x10 : 0x00;
		m_count++;
	}

	if (m_count == 5)
	{
		m_mmc1_reg[(offset & 0x6000) >> 13] = m_mmc1_latch;
		update_mirror();
		update_prg();
		update_chr();

		m_count = 0;
	}
}

// MMC3 Mode emulation
WRITE8_MEMBER(nes_somari_device::mmc3_w)
{
	UINT8 mmc_helper, cmd;

	assert(m_board_mode == 1);

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_latch ^ data;
			m_latch = data;

			if (mmc_helper & 0x40)
				update_prg();

			if (mmc_helper & 0x80)
				update_chr();
			break;

		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 1:
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					update_chr();
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data & 0x3f;
					update_prg();
					break;
			}
			break;

		case 0x2000:
			m_mmc3_mirror_reg = data & 1;
			update_mirror();
			break;
		case 0x2001: break;
		case 0x4000: m_irq_count_latch = data; break;
		case 0x4001: m_irq_count = 0; break;
		case 0x6000: m_irq_enable = 0; m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE); break;
		case 0x6001: m_irq_enable = 1; break;
	}
}

// VRC2 Mode emulation
WRITE8_MEMBER(nes_somari_device::vrc2_w)
{
	UINT8 bank, shift;

	assert(m_board_mode == 0);

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_vrc_prg_bank[0] = data & 0x1f;
			update_prg();
			break;

		case 0x1000:
			m_vrc_mirror_reg = data & 1;
			update_mirror();
			break;

		case 0x2000:
			m_vrc_prg_bank[1] = data & 0x1f;
			update_prg();
			break;

		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
			// this makes no sense for vrc2 and breaks somari, but it's ok for garousp!!
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + BIT(offset, 1);
			shift = BIT(offset, 2) * 4;
			data = (data & 0x0f) << shift;
			m_vrc_vrom_bank[bank] = data;

			update_chr();
			break;
	}
}


void nes_somari_device::update_prg()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			prg8_89(m_vrc_prg_bank[0]);
			prg8_ab(m_vrc_prg_bank[1]);
			prg8_cd(0x3e);
			prg8_ef(0x3f);
			break;
		case SOMARI_MMC3_MODE:
			{
				UINT8 prg_flip = (m_latch & 0x40) ? 2 : 0;
				prg8_x(0, m_mmc_prg_bank[0 ^ prg_flip]);
				prg8_x(1, m_mmc_prg_bank[1]);
				prg8_x(2, m_mmc_prg_bank[2 ^ prg_flip]);
				prg8_x(3, m_mmc_prg_bank[3]);
			}
			break;
		case SOMARI_MMC1_MODE:
//      case SOMARI_MMC1_MODE_AGAIN:
			{
				UINT8 prg_offset = m_mmc1_reg[1] & 0x10;

				switch (m_mmc1_reg[0] & 0x0c)
				{
					case 0x00:
					case 0x04:
						prg32((prg_offset + m_mmc1_reg[3]) >> 1);
						break;
					case 0x08:
						prg16_89ab(prg_offset + 0);
						prg16_cdef(prg_offset + m_mmc1_reg[3]);
						break;
					case 0x0c:
						prg16_89ab(prg_offset + m_mmc1_reg[3]);
						prg16_cdef(prg_offset + 0x0f);
						break;
				}
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
			{
				UINT8 chr_page = (m_latch & 0x80) >> 5;
				chr1_x(chr_page ^ 0, m_chr_base | ((m_mmc_vrom_bank[0] & ~0x01)), CHRROM);
				chr1_x(chr_page ^ 1, m_chr_base | ((m_mmc_vrom_bank[0] |  0x01)), CHRROM);
				chr1_x(chr_page ^ 2, m_chr_base | ((m_mmc_vrom_bank[1] & ~0x01)), CHRROM);
				chr1_x(chr_page ^ 3, m_chr_base | ((m_mmc_vrom_bank[1] |  0x01)), CHRROM);
				chr1_x(chr_page ^ 4, m_chr_base | (m_mmc_vrom_bank[2]), CHRROM);
				chr1_x(chr_page ^ 5, m_chr_base | (m_mmc_vrom_bank[3]), CHRROM);
				chr1_x(chr_page ^ 6, m_chr_base | (m_mmc_vrom_bank[4]), CHRROM);
				chr1_x(chr_page ^ 7, m_chr_base | (m_mmc_vrom_bank[5]), CHRROM);
			}
			break;
		case SOMARI_MMC1_MODE:
//      case SOMARI_MMC1_MODE_AGAIN:
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
			set_nt_mirroring(m_vrc_mirror_reg ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case SOMARI_MMC3_MODE:
			set_nt_mirroring(m_mmc3_mirror_reg ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case SOMARI_MMC1_MODE:
//      case SOMARI_MMC1_MODE_AGAIN:
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


WRITE8_MEMBER(nes_somari_device::write_h)
{
	LOG_MMC(("somari write_h, mode %d, offset: %04x, data: %02x\n", m_board_mode, offset, data));

	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE: vrc2_w(space, offset, data, mem_mask); break;
		case SOMARI_MMC3_MODE: mmc3_w(space, offset, data, mem_mask); break;
		case SOMARI_MMC1_MODE: mmc1_w(space, offset, data, mem_mask); break;
	}
}

void nes_somari_device::bank_update_switchmode()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			break;
		case SOMARI_MMC3_MODE:
			break;
		case SOMARI_MMC1_MODE:
			break;
	}
	update_mirror();
	update_prg();
	update_chr();
}

WRITE8_MEMBER(nes_somari_device::write_m)
{
	LOG_MMC(("somari write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x100)
	{
		m_board_mode = data & 0x03;
		m_chr_base = ((m_board_mode & 0x04) << 6);
		if (m_board_mode != 1)
			m_irq_enable = 0;
		bank_update_switchmode();
	}
}
