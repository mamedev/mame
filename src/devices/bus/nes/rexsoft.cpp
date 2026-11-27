// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Rex Soft PCBs

 iNES mappers: 12, 14


 Here we emulate the following PCBs

 * Rex Soft Dragon Ball Z V [mapper 12]
 * Rex Soft SL-1632 [mapper 14]

 The GAL on the Dragon Ball Z V board only decodes (A & 0xe100) == 0x4100,
 so 0x6000-0x7fff is plain MMC3 territory.

 ***********************************************************************************************************/


#include "emu.h"
#include "rexsoft.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_REX_DBZ5,   nes_rex_dbz5_device,   "nes_rex_dbz5",   "NES Cart Rex Soft Dragon Ball Z V PCB")
DEFINE_DEVICE_TYPE(NES_REX_SL1632, nes_rex_sl1632_device, "nes_rex_sl1632", "NES Cart Rex Soft SL-1632 PCB")


nes_rex_dbz5_device::nes_rex_dbz5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_REX_DBZ5, tag, owner, clock), m_extra(0)
{
}

nes_rex_sl1632_device::nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_REX_SL1632, tag, owner, clock), m_mode(0)
{
}




void nes_rex_dbz5_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_extra));
}

void nes_rex_dbz5_device::pcb_reset()
{
	m_extra = 0;
	mmc3_common_initialize(0xff, 0xff, 1);    // MMC3A IRQ behaviour, required by Dragon Ball Z 5
}

void nes_rex_sl1632_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_mirror));
	save_item(NAME(m_vrc2_prg_bank));
	save_item(NAME(m_vrc2_vrom_bank));
}

void nes_rex_sl1632_device::pcb_reset()
{
	m_mode = 0;
	m_mirror[0] = m_mirror[1] = 0;
	m_vrc2_prg_bank[0] = m_vrc2_prg_bank[1] = 0;
	std::fill(std::begin(m_vrc2_vrom_bank), std::end(m_vrc2_vrom_bank), 0xff);    // Huang-1 power-on value
	mmc3_common_initialize(0xff, 0xff, 0);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Rex Soft

 Games: Dragon Ball Z 5, Dragon Ball Z Super

 Gouder SL-5020B PCB: Huang-1 ASIC in MMC3A mode, plus a GAL16V8
 providing an outer CHR bank register, decoded at (A & 0xe100) == 0x4100.
 Write: bit0 = CHR A18 when PPU A12=0, bit4 = CHR A18 when PPU A12=1.
 The register lives outside the ASIC, so it is not affected by MMC3
 $8000 bit 7 and takes effect immediately (FCEUX instead latches it
 until the next MMC3 CHR bank write; that is not hardware behaviour).
 Read: bit0 = language select (hard-wired in all known copies).

 iNES: mapper 12

 In MAME: Supported.

 -------------------------------------------------*/

void nes_rex_dbz5_device::write_l(offs_t offset, uint8_t data)
{
	LOG("rex_dbz write_l, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset + 0x4100, 8))    // GAL decodes (A & 0xe100) == 0x4100
	{
		m_extra = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

// the game reads the language bit from the same GAL-decoded address it writes the CHR register to
uint8_t nes_rex_dbz5_device::read_l(offs_t offset)
{
	LOG("rex_dbz read_l, offset: %04x\n", offset);

	if (BIT(offset + 0x4100, 8))    // only D0 (language) is driven
		return (get_open_bus() & 0xfe) | 0x01;
	return get_open_bus();
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

 This board uses a Huang-1 chip, which can simulate the
 behavior of MMC1, MMC3, and VRC2. A PAL controls which
 mode is active and limits it to the latter two here.
 The MMC3 scanline IRQ only exists in MMC3 mode; VRC2 has
 no IRQ, so the counter is not clocked while in VRC2 mode.

 iNES: mapper 14

 In MAME: Supported.

 -------------------------------------------------*/

void nes_rex_sl1632_device::set_prg(int prg_base, int prg_mask)
{
	if (BIT(m_mode, 1))    // MMC3 mode
		nes_txrom_device::set_prg(prg_base, prg_mask);
	else                   // VRC2 mode
	{
		prg8_89(m_vrc2_prg_bank[0]);
		prg8_ab(m_vrc2_prg_bank[1]);
		prg16_cdef(m_prg_chunks - 1);
	}
}

void nes_rex_sl1632_device::chr_cb(int start, int bank, int source)
{
	u16 hi = BIT(m_mode, std::max(start | 1, 3)) << 8;

	if (BIT(m_mode, 1))    // MMC3 mode
		chr1_x(start, hi | bank, source);
	else                   // VRC2 mode
		chr1_x(start, hi | m_vrc2_vrom_bank[start], source);
}

void nes_rex_sl1632_device::ppu_a12_rise()
{
	if (BIT(m_mode, 1))    // MMC3 mode only
		nes_txrom_device::ppu_a12_rise();
}

void nes_rex_sl1632_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	if (BIT(m_mode, 1))    // MMC3 mode only
		nes_txrom_device::hblank_irq(scanline, vblank, blanked);
}

void nes_rex_sl1632_device::write_h(offs_t offset, u8 data)
{
	LOG("rex_sl1632 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset == 0x2131)       // Mode control register at $A131
	{
		m_mode = data;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		set_nt_mirroring(m_mirror[BIT(m_mode, 1)] ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		if (!BIT(m_mode, 1))
			set_irq_line(CLEAR_LINE);
	}
	else if (BIT(m_mode, 1))    // MMC3 mode
	{
		txrom_write(offset, data);
		if ((offset & 0x6001) == 0x2000)
			m_mirror[1] = data & 1;
	}
	else                        // VRC2 mode
		switch (offset & 0x7000)
		{
			case 0x0000:
			case 0x2000:
				m_vrc2_prg_bank[BIT(offset, 13)] = data;
				prg8_x(BIT(offset, 13), data);
				break;
			case 0x1000:
				m_mirror[0] = data & 1;
				set_nt_mirroring(m_mirror[0] ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x3000:
			case 0x4000:
			case 0x5000:
			case 0x6000:
			{
				u8 bank = ((offset >> 12) - 3) * 2 + BIT(offset, 1);
				u8 shift = (offset & 1) << 2;
				u8 mask = 0x0f << shift;
				m_vrc2_vrom_bank[bank] = (m_vrc2_vrom_bank[bank] & ~mask) | ((data << shift) & mask);
				chr_cb(bank, 0, CHRROM);
				break;
			}
		}
}
