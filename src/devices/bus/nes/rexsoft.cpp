// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Rex Soft PCBs


 Here we emulate the following PCBs

 * Rex Soft DragonBall Z V [mapper 12]
 * Rex Soft SL-1632 [mapper 14]

 TODO:
 - fix 0x6000-0x7fff accesses, write_m/read_m
 - check glitches in SL-1632

 ***********************************************************************************************************/


#include "emu.h"
#include "rexsoft.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_REX_DBZ5 = &device_creator<nes_rex_dbz5_device>;
const device_type NES_REX_SL1632 = &device_creator<nes_rex_sl1632_device>;


nes_rex_dbz5_device::nes_rex_dbz5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_REX_DBZ5, "NES Cart Rex Soft DragonBall Z V PCB", tag, owner, clock, "nes_rex_dbz5", __FILE__),
	m_extra(0)
				{
}

nes_rex_sl1632_device::nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_REX_SL1632, "NES Cart Rex Soft SL-1632 PCB", tag, owner, clock, "nes_rex_sl1632", __FILE__), m_mode(0), m_mirror(0)
				{
}




void nes_rex_dbz5_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_extra));
}

void nes_rex_dbz5_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	m_extra = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_rex_sl1632_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_mirror));
	save_item(NAME(m_extra_bank));
}

void nes_rex_sl1632_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_mode = 0;
	m_mirror = 0;
	memset(m_extra_bank, 0, sizeof(m_extra_bank));
	m_extra_bank[2] = 0xfe;
	m_extra_bank[3] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Rex Soft

 Games: Dragon Ball Z 5, Dragon Ball Z Super

 MMC3 clone. Writes to 0x4100-0x5fff (or from 0x4020?)
 possibly select higher VROM pages (to allow up to
 512 banks instead of 256 only): bit0 selects the
 higher pages for PPU banks 0-3 (0x0000-0x0fff), while
 bit4 selects the higher pages for PPU banks 4-7 (0x1000-0x1fff)

 iNES: mapper 12

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_rex_dbz5_device::write_l)
{
	LOG_MMC(("rex_dbz write_l, offset: %04x, data: %02x\n", offset, data));

	m_extra = data;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/* we would need to use this read handler in 0x6000-0x7fff as well */
READ8_MEMBER(nes_rex_dbz5_device::read_l)
{
	LOG_MMC(("rex_dbz read_l, offset: %04x\n", offset));
	return 0x01;
}

void nes_rex_dbz5_device::chr_cb(int start, int bank, int source)
{
	int shift = (start < 4) ? 8 : 4;

	bank |= ((m_extra << shift) & 0x100);
	chr1_x(start, bank, source);
}

/*-------------------------------------------------

 Rex Soft SL1632 Board

 Games: Samurai Spirits

 MMC3 clone

 iNES: mapper 14

 In MESS: Supported

 -------------------------------------------------*/

void nes_rex_sl1632_device::set_prg(int prg_base, int prg_mask)
{
	if (m_mode & 0x02)
	{
		// here standard MMC3 bankswitch
		UINT8 prg_flip = (m_latch & 0x40) ? 2 : 0;

		prg_cb(0, prg_base | (m_mmc_prg_bank[0 ^ prg_flip] & prg_mask));
		prg_cb(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
		prg_cb(2, prg_base | (m_mmc_prg_bank[2 ^ prg_flip] & prg_mask));
		prg_cb(3, prg_base | (m_mmc_prg_bank[3] & prg_mask));
	}
	else
	{
		prg8_89(m_extra_bank[0]);
		prg8_ab(m_extra_bank[1]);
		prg8_cd(m_extra_bank[2]);
		prg8_ef(m_extra_bank[3]);
	}
}

void nes_rex_sl1632_device::set_chr(UINT8 chr, int chr_base, int chr_mask)
{
	static const UINT8 conv_table[8] = {5, 5, 5, 5, 3, 3, 1, 1};
	UINT8 chr_page = (m_latch & 0x80) >> 5;
	UINT8 bank[8];
	UINT8 chr_base2[8];

	if (m_mode & 0x02)
	{
		for (int i = 0; i < 8; i++)
		{
			// since the mapper acts on 1K CHR chunks, we have 8 banks which use the 6 m_mmc_vrom_bank from MMC3 base class
			if (i < 4)
				bank[i] = ((m_mmc_vrom_bank[i / 2] & 0xfe) | (i & 1));
			else
				bank[i] = m_mmc_vrom_bank[i - 2];
			chr_base2[i] = chr_base | ((m_mode << conv_table[i]) & 0x100);
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			bank[i] = m_extra_bank[i + 4];   // first 4 m_extra_banks are PRG
			chr_base2[i] = chr_base;
		}
	}

	chr1_x(chr_page ^ 0, chr_base2[0] | (bank[0] & chr_mask), chr);
	chr1_x(chr_page ^ 1, chr_base2[1] | (bank[1] & chr_mask), chr);
	chr1_x(chr_page ^ 2, chr_base2[2] | (bank[2] & chr_mask), chr);
	chr1_x(chr_page ^ 3, chr_base2[3] | (bank[3] & chr_mask), chr);
	chr1_x(chr_page ^ 4, chr_base2[4] | (bank[4] & chr_mask), chr);
	chr1_x(chr_page ^ 5, chr_base2[5] | (bank[5] & chr_mask), chr);
	chr1_x(chr_page ^ 6, chr_base2[6] | (bank[6] & chr_mask), chr);
	chr1_x(chr_page ^ 7, chr_base2[7] | (bank[7] & chr_mask), chr);
}

WRITE8_MEMBER(nes_rex_sl1632_device::write_h)
{
	UINT8 helper1, helper2;
	LOG_MMC(("rex_sl1632 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x2131)
	{
		m_mode = data;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		if (!(m_mode & 0x02))
			set_nt_mirroring(BIT(m_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	if (m_mode & 0x02)
	{
		switch (offset & 0x6001)
		{
			case 0x2000:
				set_nt_mirroring(BIT(m_mirror, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;

			default:
				txrom_write(space, offset, data, mem_mask);
				break;
		}
	}
	else if (offset >= 0x3000 && offset <= 0x6003)
	{
		helper1 = (offset & 0x01) << 2;
		offset = ((offset & 0x02) | (offset >> 10)) >> 1;
		helper2 = ((offset + 2) & 0x07) + 4; // '+4' because first 4 m_extra_banks are for PRG!
		m_extra_bank[helper2] = (m_extra_bank[helper2] & (0xf0 >> helper1)) | ((data & 0x0f) << helper1);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
	{
		switch (offset & 0x7003)
		{
			case 0x0000:
			case 0x2000:
				m_extra_bank[offset >> 13] = data;
				set_prg(m_prg_base, m_prg_mask);
				break;

			case 0x1000:
				m_mirror = data;
				set_nt_mirroring(BIT(m_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
		}
	}
}
