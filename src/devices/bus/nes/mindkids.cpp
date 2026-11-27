// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
/***********************************************************************************************************

 NES/Famicom cartridge emulation for Mindkids boards

 Here we emulate the following PCBs

 * Mindkids SMD172B_FPGA [mapper 446]

 The board is an FPGA configured as one of many mappers (MMC1, MMC3, Irem H3001, Sunsoft FME-7 and discrete
 boards among them); the MMC3 mode reuses MAME's MMC3 implementation, which is why the device derives from it.

 ***********************************************************************************************************/


#include "emu.h"
#include "mindkids.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SMD172B, nes_smd172b_device, "nes_smd172b", "NES Cart Mindkids SMD172B_FPGA PCB")

nes_smd172b_device::nes_smd172b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SMD172B, tag, owner, clock)
	, m_ym2149(*this, "5b")
	, m_submapper(0), m_mode(MODE_SUPERVISOR), m_dlatch(0)
	, m_mmc1_shift(0), m_mmc1_count(0), m_mmc1_last_write(~u64(0))
	, m_h3001_swap(0), m_h3001_nt(0), m_h3001_irq_enable(0), m_h3001_irq_count(0), m_h3001_irq_latch(0)
	, m_fme7_cmd(0), m_fme7_irq_ctrl(0), m_fme7_irq_count(0)
{
}



/*-------------------------------------------------

 Mindkids SMD172B_FPGA

 Games: Limited Run Games and Retro-Bit re-releases (Gimmick!, Maniac Mansion,
 S.C.A.T., A Boy and His Blob, Hammerin' Harry 2), Little Samson (aftermarket),
 Jay and Silent Bob: Mall Brawl, Blazing Rangers, Pixel Games multicarts

 An FPGA that is configured as one of several mappers. At power-up and reset
 a supervisor mode runs from flash ROM: registers at $5000-$5FFF (mask $5007)
 set the outer PRG bank ($5001/$5002, 8 KiB units), the inner PRG bank mask
 ($5003, inverted meaning in submapper 2), CHR-RAM size and mirroring ($5004),
 CHR-RAM write protect ($5005 bit 2) and the outer CHR bank ($5006); writing
 $5000 with bit 7 set selects the mapper in bits 0-4 and locks the registers
 until reset. Which mapper each value selects depends on the NES 2.0 submapper.

 NES 2.0: mapper 446

 In MAME: Partially supported.

 NESdev's page documents the registers and the
 mapper table for submappers 0-3; submapper 4, the modes it does not list and
 the outer CHR bank units follow FCEUmm's mapper 446 (NewRisingSun) and agree
 with puNES: $5006 counts 8 KiB in the discrete modes and 1 KiB in the ASIC
 modes. Implemented modes: UNROM, BNROM/NROM, CNROM, AxROM, GNROM, mapper 152,
 MMC1 (SKROM, SNROM, SUROM), MMC3 (TxROM, TKSROM), MMC6 (HKROM, its 1 KiB of
 internal RAM is not emulated: it runs as an MMC3), Irem H3001 and Sunsoft
 FME-7 with the 5B sound. Not implemented: VRC1, VRC2, VRC3, VRC4, VRC6, VRC7,
 MMC2 (PNROM), TC3294, PT8154, QJ and Konami's CNROM variant, and programming
 or erasing the flash ROM (reads work; writes are ignored).

 -------------------------------------------------*/

void nes_smd172b_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_submapper));
	save_item(NAME(m_reg));
	save_item(NAME(m_mode));
	save_item(NAME(m_dlatch));
	save_item(NAME(m_mmc1_reg));
	save_item(NAME(m_mmc1_shift));
	save_item(NAME(m_mmc1_count));
	save_item(NAME(m_mmc1_last_write));
	save_item(NAME(m_h3001_prg));
	save_item(NAME(m_h3001_swap));
	save_item(NAME(m_h3001_nt));
	save_item(NAME(m_h3001_chr));
	save_item(NAME(m_h3001_irq_enable));
	save_item(NAME(m_h3001_irq_count));
	save_item(NAME(m_h3001_irq_latch));
	save_item(NAME(m_fme7_cmd));
	save_item(NAME(m_fme7_prg));
	save_item(NAME(m_fme7_chr));
	save_item(NAME(m_fme7_irq_ctrl));
	save_item(NAME(m_fme7_irq_count));
}

void nes_smd172b_device::device_add_mconfig(machine_config &config)
{
	// the FME-7 mode's Sunsoft 5B sound, as on nes_sunsoft_5_device (clocked by M2, same level)
	SUNSOFT_5B_SOUND(config, m_ym2149, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.257);
}

void nes_smd172b_device::pcb_reset()
{
	// the supervisor registers are cleared at power-up and reset, which returns the board to the supervisor (FCEUmm)
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_mode = MODE_SUPERVISOR;
	m_dlatch = 0;
	m_mmc1_reg[0] = 0x0c;
	m_mmc1_reg[1] = m_mmc1_reg[2] = m_mmc1_reg[3] = 0;
	m_mmc1_shift = m_mmc1_count = 0;
	m_mmc1_last_write = ~u64(0);
	m_h3001_prg[0] = 0; m_h3001_prg[1] = 1;
	for (int i = 0; i < 8; i++)
		m_h3001_chr[i] = m_fme7_chr[i] = 0;
	for (int i = 0; i < 8; i++)
		m_h3001_chr[i] = i;
	m_h3001_swap = m_h3001_nt = m_h3001_irq_enable = 0;
	m_h3001_irq_count = m_h3001_irq_latch = 0;
	m_fme7_cmd = 0;
	std::fill(std::begin(m_fme7_prg), std::end(m_fme7_prg), 0);
	m_fme7_irq_ctrl = 0;
	m_fme7_irq_count = 0;
	set_irq_line(CLEAR_LINE);
	mmc3_common_initialize(0xff, 0xff, 0);
	update_banks();
}

void nes_smd172b_device::select_mode()
{
	// FCEUmm's table: (submapper << 8) | $5000 bits 0-4
	switch ((m_submapper << 8) | (m_reg[0] & 0x1f))
	{
		case 0x000: case 0x100: case 0x200: case 0x400:             m_mode = MODE_UXROM; break;
		case 0x001: case 0x105: case 0x205: case 0x106: case 0x206: m_mode = MODE_MMC1; break;   // SKROM, SNROM
		case 0x107: case 0x208:                                     m_mode = MODE_SUROM; break;
		case 0x002: case 0x102: case 0x202:                         m_mode = MODE_BNROM; break;  // NROM or BNROM
		case 0x003: case 0x103: case 0x203:                         m_mode = MODE_CNROM; break;
		case 0x004: case 0x101: case 0x201: case 0x209:             m_mode = MODE_MMC3; break;   // TxROM
		case 0x10e: case 0x20e:                                     m_mode = MODE_TXSROM; break;
		case 0x10a: case 0x20a:                                     m_mode = MODE_MMC6; break;
		case 0x104: case 0x204:                                     m_mode = MODE_AXROM; break;
		case 0x108:                                                 m_mode = MODE_GNROM; break;
		case 0x10b: case 0x20b:                                     m_mode = MODE_152; break;
		case 0x301:                                                 m_mode = MODE_H3001; break;
		case 0x402:                                                 m_mode = MODE_FME7; break;
		default:
			m_mode = MODE_UNSUPPORTED;
			logerror("SMD172B: submapper %d mapper %02x is not emulated\n", m_submapper, m_reg[0] & 0x1f);
			break;
	}

	switch (m_mode)
	{
		case MODE_MMC3: case MODE_TXSROM: case MODE_MMC6:
			mmc3_common_initialize(0xff, 0xff, m_mode == MODE_MMC6);
			// FCEUmm starts the MMC3's mirroring from $5004 bit 2 (set: vertical)
			set_nt_mirroring(BIT(m_reg[4], 2) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			break;
		case MODE_H3001:
			m_h3001_nt = 0;
			break;
		case MODE_FME7:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		default:
			break;
	}
	update_banks();
}

void nes_smd172b_device::set_mirror_reg4()
{
	set_nt_mirroring(BIT(m_reg[4], 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

// the MMC3 core calls these when its registers change; they rebuild the mapping for the current mode
void nes_smd172b_device::set_prg(int prg_base, int prg_mask) { update_banks(); }
void nes_smd172b_device::set_chr(u8 chr, int chr_base, int chr_mask) { update_banks(); }

void nes_smd172b_device::update_banks()
{
	const int pand = prg_and(), por = prg_or(), cand = chr_and(), cor = chr_or();
	const int and16 = pand >> 1, or16 = por >> 1, and32 = pand >> 2, or32 = por >> 2;

	switch (m_mode)
	{
		case MODE_SUPERVISOR:
		case MODE_UNSUPPORTED:
			prg8_x(0, por);
			prg8_x(1, por + 1);
			prg8_x(2, (m_submapper == 3) ? 0x1e : 0x3e);
			prg8_x(3, (m_submapper == 3) ? 0x1f : 0x3f);
			chr8(cor, CHRRAM);
			set_mirror_reg4();
			break;

		case MODE_UXROM:
			prg16_89ab((m_dlatch & and16) | (or16 & ~and16));
			prg16_cdef(or16 | and16);
			chr8(cor, CHRRAM);
			set_mirror_reg4();
			break;

		case MODE_BNROM:
			for (int i = 0; i < 4; i++)
				prg8_x(i, (((m_dlatch << 2) | i) & pand) | por);
			chr8(cor, CHRRAM);
			set_mirror_reg4();
			break;

		case MODE_CNROM:
			for (int i = 0; i < 4; i++)
				prg8_x(i, (i & pand) | por);
			chr8(m_dlatch & (BIT(m_reg[4], 0) ? 7 : 3), CHRRAM);
			set_mirror_reg4();
			break;

		case MODE_AXROM:
			prg32((m_dlatch & and32) | (or32 & ~and32));
			chr8(cor, CHRRAM);
			set_nt_mirroring(BIT(m_dlatch, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;

		case MODE_GNROM:
			prg32(((m_dlatch >> 4) & and32) | (or32 & ~and32));
			chr8(m_dlatch & 3, CHRRAM);
			set_mirror_reg4();
			break;

		case MODE_152:
			prg16_89ab(((m_dlatch >> 4) & and16) | (or16 & ~and16));
			prg16_cdef(or16 | and16);
			chr8((m_dlatch & (cand >> 3)) | ((cor >> 3) & ~(cand >> 3)), CHRRAM);
			set_nt_mirroring(BIT(m_dlatch, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;

		case MODE_MMC1:
		case MODE_SUROM:
		{
			const bool surom = (m_mode == MODE_SUROM);
			const int sur = surom ? (m_mmc1_reg[1] & 0x10) : 0;     // SUROM: CHR register bit 4 is PRG A18
			const int r = (m_mmc1_reg[3] & 0x0f) | sur;
			int lo, hi;
			switch (BIT(m_mmc1_reg[0], 2, 2))
			{
				case 0: case 1: lo = r & ~1; hi = lo | 1; break;     // 32 KiB
				case 2:         lo = sur;    hi = r;      break;     // first bank fixed at $8000
				default:        lo = r;      hi = 0x0f | sur; break; // last bank fixed at $C000
			}
			const int prg_or16 = or16 & ~and16;
			prg16_89ab((lo & and16) | prg_or16);
			prg16_cdef((hi & and16) | prg_or16);

			int and4 = cand >> 2;
			if (surom)
				and4 &= 0x0f;
			const int or4 = (cor >> 2) & ~and4;
			int c0 = m_mmc1_reg[1], c1 = m_mmc1_reg[2];
			if (!BIT(m_mmc1_reg[0], 4))
			{
				c0 &= ~1;
				c1 = c0 | 1;
			}
			chr4_0((c0 & and4) | or4, CHRRAM);
			chr4_4((c1 & and4) | or4, CHRRAM);

			switch (m_mmc1_reg[0] & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 2: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			}
			break;
		}

		case MODE_MMC3:
		case MODE_TXSROM:
		case MODE_MMC6:
		{
			m_prg_mask = pand;
			m_prg_base = por & ~pand;
			m_chr_mask = (m_mode == MODE_TXSROM) ? (cand & 0x7f) : cand;
			m_chr_base = cor & ~m_chr_mask;
			nes_txrom_device::set_prg(m_prg_base, m_prg_mask);
			nes_txrom_device::set_chr(CHRRAM, m_chr_base, m_chr_mask);
			if (m_mode == MODE_TXSROM)
			{
				// TKSROM/TLSROM: one-screen mirroring from bit 7 of the CHR bank at PPU $0000 (FCEUmm)
				const int bank0 = (m_latch & 0x80) ? m_mmc_vrom_bank[2] : (m_mmc_vrom_bank[0] & ~1);
				set_nt_mirroring(BIT(bank0, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			}
			break;
		}

		case MODE_H3001:
		{
			// FCEUmm: $9000 bit 7 swaps $8000 and $C000; the second-to-last and last banks are $FE and $FF
			const int b0 = prg8(m_h3001_prg[0]), b1 = prg8(m_h3001_prg[1]), fe = prg8(0xfe), ff = prg8(0xff);
			prg8_x(0, m_h3001_swap ? fe : b0);
			prg8_x(1, b1);
			prg8_x(2, m_h3001_swap ? b0 : fe);
			prg8_x(3, ff);
			for (int i = 0; i < 8; i++)
				chr1_x(i, m_h3001_chr[i], CHRRAM);     // FCEUmm's H3001 applies no outer CHR bank
			if (BIT(m_h3001_nt, 6))
				set_nt_mirroring(BIT(m_h3001_nt, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			else
				set_nt_mirroring(BIT(m_h3001_nt, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		}

		case MODE_FME7:
			prg8_x(0, prg8(m_fme7_prg[1]));
			prg8_x(1, prg8(m_fme7_prg[2]));
			prg8_x(2, prg8(m_fme7_prg[3]));
			prg8_x(3, prg8(0xff));
			for (int i = 0; i < 8; i++)
				chr1_x(i, chr1(m_fme7_chr[i]), CHRRAM);
			break;
	}
}

void nes_smd172b_device::m2_w(int state, offs_t address, bool write)
{
	if (!state)
		return;

	if (m_mode == MODE_H3001)
	{
		// FCEUmm: while enabled the 16-bit counter decrements every CPU cycle and the IRQ fires when it reaches 0, which
		// also disables it
		if (BIT(m_h3001_irq_enable, 7) && !--m_h3001_irq_count)
		{
			set_irq_line(ASSERT_LINE);
			m_h3001_irq_enable = 0;
		}
	}
	else if (m_mode == MODE_FME7)
	{
		// NESdev Sunsoft FME-7: the counter decrements every CPU cycle while $0D bit 7 is set; the IRQ is raised when it
		// wraps from $0000 to $FFFF and $0D bit 0 is set (as nes_sunsoft_fme7_device)
		if (BIT(m_fme7_irq_ctrl, 7) && (--m_fme7_irq_count == 0xffff) && BIT(m_fme7_irq_ctrl, 0))
			set_irq_line(ASSERT_LINE);
	}
}

void nes_smd172b_device::write_l(offs_t offset, u8 data)
{
	offset += 0x100;
	if (offset < 0x1000)        // $5000-$5FFF
		return;
	if (BIT(m_reg[0], 7))       // locked until reset once a mapper is selected
		return;

	m_reg[offset & 7] = data;
	if (((offset & 7) == 0) && BIT(data, 7))
		select_mode();
	else
		update_banks();
}

void nes_smd172b_device::mmc1_write(offs_t offset, u8 data)
{
	// NESdev MMC1: writes on consecutive CPU cycles (read-modify-write instructions) are ignored after the first
	const u64 cycle = cpu_total_cycles();
	const bool consecutive = (cycle == m_mmc1_last_write + 1);
	m_mmc1_last_write = cycle;
	if (consecutive)
		return;

	if (BIT(data, 7))
	{
		m_mmc1_shift = m_mmc1_count = 0;
		m_mmc1_reg[0] |= 0x0c;
		update_banks();
		return;
	}

	m_mmc1_shift = (m_mmc1_shift >> 1) | ((data & 1) << 4);
	if (++m_mmc1_count == 5)
	{
		m_mmc1_reg[BIT(offset, 13, 2)] = m_mmc1_shift;
		m_mmc1_shift = m_mmc1_count = 0;
		update_banks();
	}
}

void nes_smd172b_device::h3001_write(offs_t offset, u8 data)
{
	switch ((offset >> 12) & 7)
	{
		case 0: case 2:         // $8000, $A000: PRG
			m_h3001_prg[BIT(offset, 13)] = data;
			update_banks();
			break;
		case 1:                 // $9000-$9007
			switch (offset & 7)
			{
				case 0: m_h3001_swap = BIT(data, 7); update_banks(); break;
				case 1: m_h3001_nt = data; update_banks(); break;
				case 3: m_h3001_irq_enable = data; set_irq_line(CLEAR_LINE); break;
				case 4: m_h3001_irq_count = m_h3001_irq_latch; set_irq_line(CLEAR_LINE); break;
				case 5: m_h3001_irq_latch = (m_h3001_irq_latch & 0x00ff) | (data << 8); break;
				case 6: m_h3001_irq_latch = (m_h3001_irq_latch & 0xff00) | data; break;
			}
			break;
		case 3:                 // $B000-$B007: CHR
			m_h3001_chr[offset & 7] = data;
			update_banks();
			break;
	}
}

void nes_smd172b_device::fme7_write(offs_t offset, u8 data)
{
	switch (offset & 0x6000)
	{
		case 0x0000:
			m_fme7_cmd = data & 0x0f;
			break;
		case 0x2000:
			switch (m_fme7_cmd)
			{
				case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
					m_fme7_chr[m_fme7_cmd] = data;
					update_banks();
					break;
				case 8: case 9: case 10: case 11:
					m_fme7_prg[m_fme7_cmd - 8] = data;
					update_banks();
					break;
				case 12:
					switch (data & 0x03)
					{
						case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
						case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
						case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
						case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
					}
					break;
				case 13:
					m_fme7_irq_ctrl = data;
					set_irq_line(CLEAR_LINE);
					break;
				case 14:
					m_fme7_irq_count = (m_fme7_irq_count & 0xff00) | data;
					break;
				case 15:
					m_fme7_irq_count = (m_fme7_irq_count & 0x00ff) | (data << 8);
					break;
			}
			break;
		case 0x4000:            // 5B sound
			m_ym2149->address_w(data & 0x0f);
			break;
		case 0x6000:
			m_ym2149->data_w(data);
			break;
	}
}

void nes_smd172b_device::write_h(offs_t offset, u8 data)
{
	LOG("smd172b write_h, offset: %04x, data: %02x\n", offset, data);

	switch (m_mode)
	{
		case MODE_UXROM: case MODE_BNROM: case MODE_CNROM: case MODE_AXROM: case MODE_GNROM: case MODE_152:
			m_dlatch = data;
			update_banks();
			break;
		case MODE_MMC1: case MODE_SUROM:
			mmc1_write(offset, data);
			break;
		case MODE_MMC3: case MODE_TXSROM: case MODE_MMC6:
			txrom_write(offset, data);
			break;
		case MODE_H3001:
			h3001_write(offset, data);
			break;
		case MODE_FME7:
			fme7_write(offset, data);
			break;
		default:
			// supervisor mode: flash ROM commands, not emulated
			break;
	}
}

u8 nes_smd172b_device::read_m(offs_t offset)
{
	auto ram_r = [this] (offs_t off) -> int
	{
		if (!m_battery.empty())
			return m_battery[off & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[off & (m_prgram.size() - 1)];
		return -1;
	};

	int data = -1;
	switch (m_mode)
	{
		case MODE_MMC3: case MODE_TXSROM: case MODE_MMC6:
			return nes_txrom_device::read_m(offset);
		case MODE_MMC1: case MODE_SUROM:
			if (!BIT(m_mmc1_reg[3], 4))     // MMC1B: PRG-RAM enabled while bit 4 is clear
				data = ram_r(offset);
			break;
		case MODE_FME7:
			if (!BIT(m_fme7_prg[0], 6))     // PRG-ROM at $6000
				return m_prg[((prg8(m_fme7_prg[0]) << 13) | (offset & 0x1fff)) & (m_prg_size - 1)];
			if (BIT(m_fme7_prg[0], 7))
				data = ram_r(offset);
			break;
		default:
			data = ram_r(offset);
			break;
	}
	return (data < 0) ? get_open_bus() : u8(data);
}

void nes_smd172b_device::write_m(offs_t offset, u8 data)
{
	bool enabled;
	switch (m_mode)
	{
		case MODE_MMC3: case MODE_TXSROM: case MODE_MMC6:
			nes_txrom_device::write_m(offset, data);
			return;
		case MODE_MMC1: case MODE_SUROM:
			enabled = !BIT(m_mmc1_reg[3], 4);
			break;
		case MODE_FME7:
			enabled = BIT(m_fme7_prg[0], 6) && BIT(m_fme7_prg[0], 7);
			break;
		default:
			enabled = true;
			break;
	}
	if (!enabled)
		return;
	if (!m_battery.empty())
		m_battery[offset & (m_battery.size() - 1)] = data;
	else if (!m_prgram.empty())
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

void nes_smd172b_device::chr_w(offs_t offset, u8 data)
{
	if (BIT(m_reg[5], 2))       // $5005 bit 2: CHR-RAM write-protected
		return;
	device_nes_cart_interface::chr_w(offset, data);
}
