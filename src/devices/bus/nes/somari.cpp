// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Somari Team PCBs


 Here we emulate the Somari Team PCBs [mapper 116]


 ***********************************************************************************************************/


#include "emu.h"
#include "somari.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"

#define SOMARI_VRC2_MODE 0
#define SOMARI_MMC3_MODE 1
#define SOMARI_MMC1_MODE 2


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SOMARI, nes_somari_device, "nes_somari", "NES Cart Team Somari PCB")
DEFINE_DEVICE_TYPE(NES_HUANG2, nes_huang2_device, "nes_huang2", "NES Cart Huang-2 PCB")
DEFINE_DEVICE_TYPE(NES_SOMARI_MC, nes_somari_mc_device, "nes_somari_mc", "NES Cart SL-FC5-1 Mario 5-in-1 PCB (mapper 116 submapper 3)")


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

	save_item(NAME(m_outer_on));
	save_item(NAME(m_prg_and));
	save_item(NAME(m_prg_or));
	save_item(NAME(m_chr_and));
	save_item(NAME(m_chr_or));
}

void nes_somari_device::pcb_reset()
{
	m_board_mode = SOMARI_MMC3_MODE;
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
			m_mmc1_reg[BIT(offset, 13, 2)] = m_mmc1_latch;
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

void nes_somari_device::prg_cb(int start, int bank)
{
	if (m_outer_on)
		bank = (bank & m_prg_and) | m_prg_or;
	nes_txrom_device::prg_cb(start, bank);
}

void nes_somari_device::chr_cb(int start, int bank, int source)
{
	if (m_outer_on)
		bank = (bank & m_chr_and) | m_chr_or;
	nes_txrom_device::chr_cb(start, bank, source);
}

void nes_somari_device::update_prg()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			if (m_outer_on)
			{
				prg8_89((m_vrc_prg_bank[0] & m_prg_and) | m_prg_or);
				prg8_ab((m_vrc_prg_bank[1] & m_prg_and) | m_prg_or);
				prg16_cdef(((0xff & m_prg_and) | m_prg_or) >> 1);
			}
			else
			{
				prg8_89(m_vrc_prg_bank[0]);
				prg8_ab(m_vrc_prg_bank[1]);
				prg16_cdef(m_prg_chunks - 1);
			}
			break;
		case SOMARI_MMC3_MODE:
			set_prg(m_prg_base, m_prg_mask);
			break;
		case SOMARI_MMC1_MODE:
		{
			// with the multicart's outer bank: 16K banks use and/or >> 1, 32K banks >> 2
			auto p16 = [this] (int b) { return m_outer_on ? ((b & (m_prg_and >> 1)) | (m_prg_or >> 1)) : b; };
			auto p32 = [this] (int b) { return m_outer_on ? ((b & (m_prg_and >> 2)) | (m_prg_or >> 2)) : b; };
			switch (BIT(m_mmc1_reg[0], 2, 2))
			{
				case 0:
				case 1:
					prg32(p32(m_mmc1_reg[3] >> 1));
					break;
				case 2:
					prg16_89ab(p16(0));
					prg16_cdef(p16(m_mmc1_reg[3] >> m_mmc1_prg_shift));
					break;
				case 3:
					prg16_89ab(p16(m_mmc1_reg[3] >> m_mmc1_prg_shift));
					prg16_cdef(p16(0x0f >> m_mmc1_prg_shift));
					break;
			}
			break;
		}
	}
}

void nes_somari_device::update_chr()
{
	switch (m_board_mode)
	{
		case SOMARI_VRC2_MODE:
			for (int i = 0; i < 8; i++)
				chr1_x(i, m_outer_on ? ((m_vrc_vrom_bank[i] & m_chr_and) | m_chr_or) : (m_chr_base | m_vrc_vrom_bank[i]), CHRROM);
			break;
		case SOMARI_MMC3_MODE:
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		case SOMARI_MMC1_MODE:
		{
			// with the multicart's outer bank: 4K banks use and/or >> 2, 8K banks >> 3
			auto c4 = [this] (int b) { return m_outer_on ? ((b & (m_chr_and >> 2)) | (m_chr_or >> 2)) : b; };
			auto c8 = [this] (int b) { return m_outer_on ? ((b & (m_chr_and >> 3)) | (m_chr_or >> 3)) : b; };
			if (BIT(m_mmc1_reg[0], 4))
			{
				chr4_0(c4(m_mmc1_reg[1] & 0x1f), CHRROM);
				chr4_4(c4(m_mmc1_reg[2] & 0x1f), CHRROM);
			}
			else
				chr8(c8(BIT(m_mmc1_reg[1], 1, 4)), CHRROM);
			break;
		}
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
	LOG("somari write_h, mode %d, offset: %04x, data: %02x\n", m_board_mode, offset, data);

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
	LOG("somari write_m, offset: %04x, data: %02x\n", offset, data);

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


/*-------------------------------------------------

 SL-FC5-1 Mario 5-in-1 (SOMARI-P board, reset-based)

 NESdev INES Mapper 116: NES 2.0 submapper 3 makes mapper 116 a
 reset-based multicart that switches games on reset; it is used
 for the one known SL-FC5-1 Mario 5-in-1 with five MMC3-based
 Mario hacks. NESdev gives no more detail, so the layout is
 FCEUmm's 116.c: the first game is 256 KiB of PRG and 256 KiB of
 CHR (8 KiB PRG banks ANDed with $1F, 1 KiB CHR banks with $FF),
 the others 128 KiB each (ANDed with $0F and $7F, ORed with the
 game number times 128 KiB); each reset steps to the next game,
 skipping game number 1 because the first game takes two 128 KiB
 slots, and wraps after the last. The outer bank applies in all
 three of the board's modes (VRC2, MMC3 and MMC1), as in FCEUmm;
 the $4100 outer CHR bit (CHR A18) is not used. Power-on starts
 game 0.

 iNES: mapper 116, NES 2.0 submapper 3

 In MAME: Supported.

 -------------------------------------------------*/

nes_somari_mc_device::nes_somari_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_somari_device(mconfig, NES_SOMARI_MC, tag, owner, clock, 0), m_game(0xff)
{
}

void nes_somari_mc_device::device_start()
{
	nes_somari_device::device_start();
	save_item(NAME(m_game));
}

void nes_somari_mc_device::pcb_reset()
{
	// power-on starts game 0 (m_game is $FF until then); every reset steps to the next game
	const u8 games = std::max<u32>(m_prg_size / 0x20000, 1);
	if (m_game == 0xff)
		m_game = 0;
	else
	{
		if (++m_game == 1)
			++m_game;       // the first game takes two 128 KiB slots
		if (m_game >= games)
			m_game = 0;
	}
	m_outer_on = true;
	m_prg_and = m_game ? 0x0f : 0x1f;
	m_chr_and = m_game ? 0x7f : 0xff;
	m_prg_or = (m_game << 4) & ~m_prg_and;
	m_chr_or = (m_game << 7) & ~m_chr_and;
	nes_somari_device::pcb_reset();
	update_all_banks();
}
