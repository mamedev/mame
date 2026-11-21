// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami PCBs

 iNES mappers: 21, 22, 23, 24, 25, 26, 73, 75, 85, 547


 Here we emulate the following PCBs (multiple mappers needed due to different wirings in the pcbs
 causing different addresses to be used for bankswitch & irq)

 * Konami VRC-1 [mapper 75]
 * Konami VRC-2 [mapper 22,23,25]
 * Konami VRC-3 [mapper 73]
 * Konami VRC-4 [mapper 21,23,25]
 * Konami VRC-5 (QTa adapter) [mapper 547]
 * Konami VRC-6 [mapper 24,26]
 * Konami VRC-7 [mapper 85]

 ***********************************************************************************************************/


#include "emu.h"
#include "konami.h"

#include "speaker.h"
#include "video/ppu2c0x.h"      // the VRC5 tells background pattern fetches from sprite ones, as the MMC5 does

#define LOG_UNHANDLED (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_UNHANDLED | LOG_GENERAL)
#else
#define VERBOSE (LOG_UNHANDLED)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_VRC1, nes_konami_vrc1_device, "nes_vrc1", "NES Cart Konami VRC-1 PCB")
DEFINE_DEVICE_TYPE(NES_VRC2, nes_konami_vrc2_device, "nes_vrc2", "NES Cart Konami VRC-2 PCB")
DEFINE_DEVICE_TYPE(NES_VRC3, nes_konami_vrc3_device, "nes_vrc3", "NES Cart Konami VRC-3 PCB")
DEFINE_DEVICE_TYPE(NES_VRC4, nes_konami_vrc4_device, "nes_vrc4", "NES Cart Konami VRC-4 PCB")
DEFINE_DEVICE_TYPE(NES_VRC5, nes_konami_vrc5_device, "nes_vrc5", "NES Cart Konami VRC-5 (QTa adapter) PCB")
DEFINE_DEVICE_TYPE(NES_VRC6, nes_konami_vrc6_device, "nes_vrc6", "NES Cart Konami VRC-6 PCB")
DEFINE_DEVICE_TYPE(NES_VRC7, nes_konami_vrc7_device, "nes_vrc7", "NES Cart Konami VRC-7 PCB")


nes_konami_vrc1_device::nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC1, tag, owner, clock)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_VRC2, tag, owner, clock)
{
}

nes_konami_vrc3_device::nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC3, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_prg_flip(0), m_wram_enable(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0), m_irq_prescale(0)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC4, tag, owner, clock)
{
}

nes_konami_vrc6_device::nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC6, tag, owner, clock), m_vrc6snd(*this, "vrc6snd")
{
}

nes_konami_vrc7_device::nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC7, tag, owner, clock), m_vrc7snd(*this, "vrc7snd")
{
}



void nes_konami_vrc1_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_mmc_vrom_bank[0] = m_mmc_vrom_bank[1] = 0;
}

void nes_konami_vrc2_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));
}

void nes_konami_vrc2_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}

void nes_konami_vrc3_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_konami_vrc3_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_mode = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}

void nes_konami_vrc4_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_prescale));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_prg_flip));
	save_item(NAME(m_wram_enable));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc4_device::pcb_reset()
{
	m_irq_mode = 0;
	m_irq_prescale = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_prg_flip = 0;
	m_wram_enable = 0;
	m_mmc_prg_bank[0] = 0;
	m_mmc_prg_bank[1] = 0;
	set_prg();

	for (int i = 0; i < 8; i++)
		m_mmc_vrom_bank[i] = i;
	set_chr();
}

void nes_konami_vrc7_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	prg32((m_prg_chunks >> 1) - 1);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Konami VRC1 and Jaleco JF20, JF22

 Games: Exciting Boxing, Ganbare Goemon!, Tetsuwan Atom

 iNES: mapper 75

 In MAME: Supported.

 -------------------------------------------------*/

void nes_konami_vrc1_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-1 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
		case 0x4000:
			prg8_x(BIT(offset, 13, 2), data & 0x0f);
			break;
		case 0x1000:
			// the mirroring bit is ignored on boards wired for four-screen VRAM (Vs. System VRC1 games; NESdev/Mesen)
			if (get_mirroring() != PPU_MIRROR_4SCREEN)
				set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x0f) | BIT(data, 1) << 4;
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x0f) | BIT(data, 2) << 4;
			chr4_0(m_mmc_vrom_bank[0], m_chr_source);
			chr4_4(m_mmc_vrom_bank[1], m_chr_source);
			break;
		case 0x6000:
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x10) | (data & 0x0f);
			chr4_0(m_mmc_vrom_bank[0], m_chr_source);
			break;
		case 0x7000:
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x10) | (data & 0x0f);
			chr4_4(m_mmc_vrom_bank[1], m_chr_source);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-2

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_konami_vrc2_device::read_m(offs_t offset)
{
	LOG("VRC-2 read_m, offset: %04x\n", offset);

	if (!m_battery.empty() || !m_prgram.empty())
		return device_nes_cart_interface::read_m(offset);
	else    // VRC2 was planned with EEPROM support; the non-working feature behaves as a vestigial 1-bit latch in $6000-$6fff on certain boards (contraj, ggoemon2 depend on this)
		return (offset < 0x1000) ? (get_open_bus() & 0xfe) | (m_latch & 1) : get_open_bus();
}

void nes_konami_vrc2_device::write_m(offs_t offset, u8 data)
{
	LOG("VRC-2 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!m_battery.empty() || !m_prgram.empty())
		device_nes_cart_interface::write_m(offset, data);
	else if (offset < 0x1000)
		m_latch = data;
}

void nes_konami_vrc2_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-2 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			prg8_x(BIT(offset, 13), data);
			break;
		case 0x1000:
			set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = 0x0f << shift;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | (((data >> m_vrc_ls_chr) << shift) & mask);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		}
		default:
			LOGMASKED(LOG_UNHANDLED, "VRC-2 write_h uncaught write, addr: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC3

 Games: Salamander

 iNES: mapper 73

 In MAME: Supported.

 -------------------------------------------------*/

void nes_konami_vrc3_device::irq_clock()
{
	if (m_irq_enable)
	{
		u16 mask = m_irq_mode ? 0x00ff : 0xffff; // 8 or 16 bit mode?

		// upper byte only incremented and reloaded in 16-bit mode
		m_irq_count = (m_irq_count & ~mask) | ((m_irq_count + 1) & mask);
		if (!(m_irq_count & mask))
		{
			set_irq_line(ASSERT_LINE);
			m_irq_count = (m_irq_count & ~mask) | (m_irq_count_latch & mask);
		}
	}
}

void nes_konami_vrc3_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-3 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
		{
			int shift = 4 * BIT(offset, 12, 2);
			m_irq_count_latch &= ~(0x000f << shift);
			m_irq_count_latch |= (data & 0x0f) << shift;
			break;
		}
		case 0x4000:
			m_irq_mode = data & 0x04;
			m_irq_enable = data & 0x02;
			m_irq_enable_latch = data & 0x01;
			if (m_irq_enable)
				m_irq_count = m_irq_count_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x5000:
			m_irq_enable = m_irq_enable_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x7000:
			prg16_89ab(data & 0x07);
			break;
		default:
			LOGMASKED(LOG_UNHANDLED, "VRC-3 write_h uncaught write, offset %04x, data: %02x\n", offset, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-4

 In MAME: Supported

 -------------------------------------------------*/

void nes_konami_vrc4_device::irq_tick()
{
	if (m_irq_count == 0xff)
	{
		m_irq_count = m_irq_count_latch;
		vrc_irq_w(ASSERT_LINE);
	}
	else
		m_irq_count++;
}

void nes_konami_vrc4_device::irq_clock()
{
	if (m_irq_enable)
	{
		if (m_irq_mode) // cycle mode
			irq_tick();
		else    // scanline mode
		{
			// A prescaler divides the passing CPU cycles by 114, 114, then 113 (and repeats that order).
			// This approximates 113+2/3 CPU cycles, which is one NTSC scanline.
			// Since this is a CPU-based IRQ, though, it is triggered also during non visible scanlines...
			m_irq_prescale -= 3;

			if (m_irq_prescale <= 0)
			{
				m_irq_prescale += 341;
				irq_tick();
			}
		}
	}
}

void nes_konami_vrc4_device::irq_ack_w()
{
	m_irq_enable = m_irq_enable_latch;
	vrc_irq_w(CLEAR_LINE);
}

void nes_konami_vrc4_device::irq_ctrl_w(u8 data)
{
	m_irq_mode = data & 0x04;
	m_irq_enable = data & 0x02;
	m_irq_enable_latch = data & 0x01;
	// NESdev VRC IRQ: any write acknowledges the IRQ and resets the prescaler; with E set the counter is also reloaded
	m_irq_prescale = 341;
	if (m_irq_enable)
		m_irq_count = m_irq_count_latch;
	vrc_irq_w(CLEAR_LINE);
}

void nes_konami_vrc4_device::set_mirror(u8 data)
{
	switch (data & 0x03)
	{
		case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

void nes_konami_vrc4_device::set_prg(int prg_base, int prg_mask)
{
	prg8_x(0 ^ m_prg_flip, prg_base | (m_mmc_prg_bank[0] & prg_mask));
	prg8_x(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
	prg8_x(2 ^ m_prg_flip, prg_base | (prg_mask & ~1));
	prg8_x(3, prg_base | prg_mask);
}

void nes_konami_vrc4_device::set_chr(int chr_base, int chr_mask)
{
	for (int i = 0; i < 8; i++)
		chr1_x(i, chr_base | (m_mmc_vrom_bank[i] & chr_mask), m_chr_source);
}

u8 nes_konami_vrc4_device::read_m(offs_t offset)
{
	LOG("VRC-4 read_m, offset: %04x\n", offset);

	if (m_wram_enable)
		return device_nes_cart_interface::read_m(offset);
	else
		return get_open_bus();
}

void nes_konami_vrc4_device::write_m(offs_t offset, u8 data)
{
	LOG("VRC-4 write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_wram_enable)
		device_nes_cart_interface::write_m(offset, data);
}

void nes_konami_vrc4_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-4 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			m_mmc_prg_bank[BIT(offset, 13)] = data;
			set_prg();
			break;
		case 0x1000:
			if (BIT(addr_lines, 1))
			{
				m_wram_enable = data & 0x01;
				m_prg_flip = data & 0x02;
				set_prg();
			}
			else
				set_mirror(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = shift ? 0x1f0 : 0x0f;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | ((data << shift) & mask);
			set_chr();
			break;
		}
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = (m_irq_count_latch & 0xf0) | (data & 0x0f);
					break;
				case 1:
					m_irq_count_latch = (m_irq_count_latch & 0x0f) | (data & 0x0f) << 4;
					break;
				case 2:
					irq_ctrl_w(data);
					break;
				case 3:
					irq_ack_w();
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-6

 In MAME: Supported. It also uses konami_irq (there are IRQ
 issues though: see Akumajou Densetsu intro).

 All four $B003 CHR layouts, the bit 5 pairing and nametables from
 CHR-ROM or CIRAM are implemented per NESdev; the three released
 VRC6 games do not use all of them.

 -------------------------------------------------*/

void nes_konami_vrc6_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_vrc6_chr));
	save_item(NAME(m_vrc6_mode));
}

void nes_konami_vrc6_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	std::fill(std::begin(m_vrc6_chr), std::end(m_vrc6_chr), 0);
	m_vrc6_mode = 0;
	vrc6_update_ppu();
}

// $B003 PPU banking style (NESdev VRC6, behaviour as implemented by Mesen): bits 0-1 CHR layout, bit 5 pairs 1KiB
// banks (A10 from the bank number), bit 4 nametables from CHR-ROM instead of CIRAM, bits 2-3/5 nametable arrangement
void nes_konami_vrc6_device::vrc6_update_ppu()
{
	const u8 *r = m_vrc6_chr;
	const u8 mask = BIT(m_vrc6_mode, 5) ? 0xfe : 0xff;
	const u8 orv = BIT(m_vrc6_mode, 5) ? 0x01 : 0x00;

	switch (m_vrc6_mode & 0x03)
	{
		case 0:
			for (int i = 0; i < 8; i++)
				chr1_x(i, r[i], m_chr_source);
			break;
		case 1:
			for (int i = 0; i < 4; i++)
			{
				chr1_x(2 * i, r[i] & mask, m_chr_source);
				chr1_x(2 * i + 1, (r[i] & mask) | orv, m_chr_source);
			}
			break;
		default:
			for (int i = 0; i < 4; i++)
				chr1_x(i, r[i], m_chr_source);
			chr1_x(4, r[4] & mask, m_chr_source);
			chr1_x(5, (r[4] & mask) | orv, m_chr_source);
			chr1_x(6, r[5] & mask, m_chr_source);
			chr1_x(7, (r[5] & mask) | orv, m_chr_source);
			break;
	}

	u8 nt[4];
	switch (m_vrc6_mode & 0x2f)
	{
		case 0x20: case 0x27: nt[0] = r[6] & 0xfe; nt[1] = (r[6] & 0xfe) | 1; nt[2] = r[7] & 0xfe; nt[3] = (r[7] & 0xfe) | 1; break;
		case 0x23: case 0x24: nt[0] = r[6] & 0xfe; nt[1] = r[7] & 0xfe; nt[2] = (r[6] & 0xfe) | 1; nt[3] = (r[7] & 0xfe) | 1; break;
		case 0x28: case 0x2f: nt[0] = nt[1] = r[6] & 0xfe; nt[2] = nt[3] = r[7] & 0xfe; break;
		case 0x2b: case 0x2c: nt[0] = nt[2] = (r[6] & 0xfe) | 1; nt[1] = nt[3] = (r[7] & 0xfe) | 1; break;
		default:
			switch (m_vrc6_mode & 0x07)
			{
				case 1: case 5: nt[0] = r[4]; nt[1] = r[5]; nt[2] = r[6]; nt[3] = r[7]; break;
				case 2: case 3: case 4: nt[0] = nt[2] = r[6]; nt[1] = nt[3] = r[7]; break;
				default: nt[0] = nt[1] = r[6]; nt[2] = nt[3] = r[7]; break;   // 0, 6, 7
			}
			break;
	}
	for (int page = 0; page < 4; page++)
	{
		if (BIT(m_vrc6_mode, 4))
		{
			// nametables from CHR memory: CHR-ROM, or the board's CHR-RAM when it has no CHR-ROM
			if (m_vrom_chunks)
				set_nt_page(page, VROM, nt[page], 0);
			else
				set_nt_page(page, VRAM_NT, nt[page] & ((get_vram_size() / 0x400) - 1), 1);
		}
		else
			set_nt_page(page, CIRAM, nt[page] & 0x01, 1);
	}
}

void nes_konami_vrc6_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-6 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg16_89ab(data & 0x0f);
			break;
		case 0x4000:
			prg8_cd(data & 0x1f);
			break;
		case 0x1000:    // pulse 1 & global control
			m_vrc6snd->write(addr_lines, data);
			break;
		case 0x2000:    // pulse 2
			m_vrc6snd->write(addr_lines | 0x100, data);
			break;
		case 0x3000:
			if (addr_lines == 3)
			{
				m_vrc6_mode = data;
				m_wram_enable = BIT(data, 7);
				vrc6_update_ppu();
			}
			else    // saw
				m_vrc6snd->write(addr_lines | 0x200, data);
			break;
		case 0x5000:
		case 0x6000:
			m_vrc6_chr[4 * BIT(offset, 13) + addr_lines] = data;
			vrc6_update_ppu();
			break;
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = data;
					break;
				case 1:
					irq_ctrl_w(data);
					break;
				case 2:
					irq_ack_w();
					break;
				default:
					LOGMASKED(LOG_UNHANDLED, "VRC-6 write_h uncaught write, addr: %04x value: %02x\n", ((offset & 0x7000) | addr_lines) + 0x8000, data);
					break;
			}
			break;
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_konami_vrc6_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware, mixed into the console audio through the cartridge connector

	// clocked from the cartridge's CPU clock (M2), so it follows the console: NTSC, PAL or Dendy
	// level measured with nes-audio-tests db_vrc6 against the hardware survey (+0.2 dB vs a full-volume APU square)
	VRC6(config, m_vrc6snd, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.175);
}

/*-------------------------------------------------

 Konami VRC7

 Games: Lagrange Point, Tiny Toon Adventures 2

 iNES: mapper 85

 In MAME: Supported. It also uses konami_irq.

 -------------------------------------------------*/

void nes_konami_vrc7_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-7 write_h, offset: %04x, data: %02x\n", offset, data);

	// one decode for both wirings (as Mesen): the audio registers are $9010 (select) and $9030 (data) on VRC7a and
	// VRC7b alike; any other address with A4 set acts as A3, so VRC7a (A4) and VRC7b (A3) games both work without a
	// submapper (unspecified mapper 85 images were decoded as VRC7b only, which put VRC7a audio writes on PRG banks)
	u16 addr = offset | 0x8000;
	if ((addr & 0xf038) == 0x9010)
	{
		m_vrc7snd->address_w(data);
		return;
	}
	if ((addr & 0xf038) == 0x9030)
	{
		m_vrc7snd->data_w(data);
		return;
	}
	if (BIT(addr, 4))
		addr = (addr | 0x08) & ~0x10;
	u8 reg = bitswap<4>(addr, 14, 13, 12, 3);

	switch (reg)
	{
		case 0x0:
		case 0x1:
		case 0x2:
			prg8_x(reg, data & 0x3f);
			break;
		case 0x3:   // $9008/$9028: no register
			break;
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
			chr1_x(reg - 4, data, m_chr_source);
			break;
		case 0xc:
			set_mirror(data);
			m_wram_enable = BIT(data, 7);
			break;
		case 0xd:
			m_irq_count_latch = data;
			break;
		case 0xe:
			irq_ctrl_w(data);
			break;
		case 0xf:
			irq_ack_w();
			break;
	}
}


//-------------------------------------------------
//  MACHINE_DRIVER( vrc7 )
//-------------------------------------------------

// From NESdev wiki: The VRC7, in addition to being a mapper chip, also produces 6 channels of
// 2-operator FM Synthesis Audio. It is a derivative of the Yamaha YM2413 OPLL, implementing a
// subset of its features and containing a custom fixed patch set.
// The synthesis core is related to the Yamaha YM2413 OPLL:
// - Register layout is the same.
// - VRC7 has 6 channels, OPLL has 9.
// - VRC7 has no rhythm channels, OPLL does (the last 3 channels are either FM or Rhythm on OPLL).
// - VRC7 built-in instruments are not the same as OPLL instruments.
// - VRC7 has no readily-accessible status register, under normal circumstances it is write-only;
//   OPLL has an undocumented, 2-bit 'internal state' register.
// - VRC7 has an internal state output pin (may be serial version of the 2 bit internal state register)
//   and has one output pin for audio, multiplexed for all 6 channels; OPLL has two output pins, one for
//   FM and one for Rhythm, and has no special status pin.

void nes_konami_vrc7_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware, mixed into the console audio through the cartridge connector

	// the sound core runs at twice the cartridge's CPU clock (M2), so it follows the console: NTSC, PAL or Dendy
	// level measured with nes-audio-tests db_vrc7 against the hardware survey (+11.3 dB vs a full-volume APU square)
	DS1001(config, m_vrc7snd, DERIVED_CLOCK(2, 1)).add_route(0, DEVICE_SELF_OWNER, 2.43).add_route(1, DEVICE_SELF_OWNER, 0.0);
}


/*-------------------------------------------------

 Konami VRC-5 (QTa adapter)

 Games: NHK Gakuen - Space School: Sansu 4-6 Nen (Jou, Ge),
 Idemitsu - Space College

 NESdev NES 2.0 Mapper 547. The adapter holds 128 KiB of PRG-
 ROM, the 128 KiB Kanji ROM, 8 KiB of WRAM and 8 KiB of CHR-
 RAM; each 40-pin game cartridge adds 512 KiB of PRG-ROM and
 8 KiB of battery-backed WRAM. NES 2.0 images hold the adapter
 PRG-ROM, then the cartridge PRG-ROM (640 KiB), and the Kanji
 ROM as the CHR-ROM.
 - $D000/$D100: 4 KiB WRAM banks at $6000/$7000:
   bit 3 chip (0 cartridge, battery-backed; 1 adapter), bit 0
   A12
 - $D200/$D300/$D400: 8 KiB PRG-ROM banks at $8000/$A000/
   $C000: bit 6 chip (0 adapter, 1 cartridge), bits 5-0
   A18..A13. $E000-$FFFF is the cartridge's last bank.
 - $D500: bit 0 is CHR A12 for PPU $0000-$0FFF (sprites and
   PPUDATA); PPU $1000-$1FFF is CHR-RAM bank 1
 - $D600/$D700: 16-bit IRQ latch; $D800: acknowledges and
   moves A to E; $D900 (..EA): acknowledges, loads the counter
   from the latch when E is set; the enabled counter counts
   M2 cycles and its wrap from $FFFF raises the IRQ and
   reloads it
 - $DA00: bit 1 mirroring (0 vertical, 1 horizontal), bit 0
   sends PPU $2000-$2FFF writes to the 2 KiB QTRAM instead of
   CIRAM
 - $DB00/$DC00/$DD00 (write) and $DC00/$DD00 (read): the
   JIS X 0208 translation; the tile number reads at $DC00 and
   the QTRAM byte at $DD00. The translation uses the table
   FCEUX read out of the hardware (vrc5.cpp), which NESdev
   names as the hardware's own.
 - every background nametable fetch also fetches the QTRAM
   byte at the same address (RCBB BBBb); the following
   background pattern fetches come from CHR-RAM bank b (C=0,
   PPU A12 ignored) or Kanji ROM bank BBBBBb (C=1). The Kanji
   ROM is 1 bpp, wired to look like 2 bpp: PPU A3=1 reads all
   1s when R is set and all 0s when it is clear; PPU A0-A2,
   A4-A11 and the bank go to the ROM's A1-A3, A0, A4-A10 and
   A11-A16.
 Sprite pattern fetches work normally.

 Not documented, so not emulated: the register mask (taken as
 $FF00), the M2 edge the counter counts (falling here), the
 power-on values (all 0 here: adapter banks 0 at $8000-$DFFF,
 the cartridge's WRAM at $6000-$7FFF, IRQ disabled), and the
 Kanji ROM window at PPU $1000-$1FFF that FCEUX mentions and
 no game uses.
 Also not documented, chosen here: QTRAM is addressed through
 the same mirroring as CIRAM (NESdev only calls it a 2 KiB
 shadow nametable); the QTRAM byte is also latched on CPU
 PPUDATA reads of the nametables (NESdev describes it for
 background fetches only; the next rendering fetch replaces
 it); background and sprite pattern fetches are told apart with
 the MMC5's test; CPU PPUDATA access to CHR $0000-$0FFF uses the
 $D500 bank (NESdev describes that bank for sprites only).
 Translation check: rendered from the NHK Gakuen Kanji ROM, the
 table gives the right glyphs for JIS $2341 (A), $2330 (0),
 $2422 (hiragana a), $3021, $3022 and $467E (kanji);
 NESdev's written formula selects other glyphs. No game was run after these notes were written.

 NES 2.0: mapper 547

 In MAME: Supported.

 -------------------------------------------------*/

nes_konami_vrc5_device::nes_konami_vrc5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC5, tag, owner, clock)
	, m_ppu(*this, ":ppu") // as for the MMC5, the board has to know which PPU fetch it answers
	, m_qt_byte(0), m_irq_latch(0), m_irq_count(0), m_irq_enable(false), m_irq_enable_ack(false)
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	std::fill(std::begin(m_qtram), std::end(m_qtram), 0);
}

void nes_konami_vrc5_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_qtram));
	save_item(NAME(m_qt_byte));
	save_item(NAME(m_irq_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_ack));
}

void nes_konami_vrc5_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_qt_byte = 0;
	m_irq_latch = m_irq_count = 0;
	m_irq_enable = m_irq_enable_ack = false;
	set_irq_line(CLEAR_LINE);
	set_nt_mirroring(PPU_MIRROR_VERT);
	update_prg();
}

void nes_konami_vrc5_device::update_prg()
{
	// 640 KiB image: 16 banks of the adapter's ROM, then 64 of the cartridge's
	for (int i = 0; i < 3; i++)
	{
		const u8 v = m_reg[2 + i];
		prg8_x(i, BIT(v, 6) ? (16 + (v & 0x3f)) : (v & 0x0f));
	}
	prg8_ef(16 + 63);
}

u8 nes_konami_vrc5_device::read_m(offs_t offset)
{
	const u8 r = m_reg[BIT(offset, 12)];
	const std::vector<u8> &ram = BIT(r, 3) ? m_prgram : m_battery;
	if (ram.empty())
		return get_open_bus();
	return ram[((BIT(r, 0) << 12) | (offset & 0xfff)) & (ram.size() - 1)];
}

void nes_konami_vrc5_device::write_m(offs_t offset, u8 data)
{
	const u8 r = m_reg[BIT(offset, 12)];
	std::vector<u8> &ram = BIT(r, 3) ? m_prgram : m_battery;
	if (!ram.empty())
		ram[((BIT(r, 0) << 12) | (offset & 0xfff)) & (ram.size() - 1)] = data;
}

// FCEUX vrc5.cpp: the conversion table read out of the hardware, indexed by the column's bits 6-5 and the row's bits 6-4
static const u8 vrc5_conv_tbl[4][8] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x40, 0x10, 0x28, 0x00, 0x18, 0x30 },
	{ 0x00, 0x00, 0x48, 0x18, 0x30, 0x08, 0x20, 0x38 },
	{ 0x00, 0x00, 0x80, 0x20, 0x38, 0x10, 0x28, 0xb0 }
};

u8 nes_konami_vrc5_device::translated_bank() const
{
	const u8 col = m_reg[0xc], row = m_reg[0xd];
	const u8 tabl = vrc5_conv_tbl[(col >> 5) & 3][(row & 0x7f) >> 4];
	u8 res = 0x40 | (tabl & 0x3f) | ((row >> 1) & 7) | ((m_reg[0xb] & 4) << 5);
	if (tabl & 0x40)
		res &= 0xfb;
	else if (tabl & 0x80)
		res |= 0x04;
	return res;
}

u8 nes_konami_vrc5_device::translated_tile() const
{
	return (BIT(m_reg[0xd], 0) << 7) | ((m_reg[0xc] & 0x1f) << 2) | (m_reg[0xb] & 3);
}

u8 nes_konami_vrc5_device::read_h(offs_t offset)
{
	switch (offset & 0x7f00)
	{
		case 0x5c00: return translated_tile();   // $DC00
		case 0x5d00: return translated_bank();   // $DD00
		default:     return hi_access_rom(offset);
	}
}

void nes_konami_vrc5_device::write_h(offs_t offset, u8 data)
{
	LOG("vrc5 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x7000) != 0x5000)   // registers at $D000-$DFFF
		return;

	const u8 reg = (offset >> 8) & 0x0f;
	m_reg[reg] = data;
	switch (reg)
	{
		case 0x2: case 0x3: case 0x4:
			update_prg();
			break;
		case 0x6:
			m_irq_latch = (m_irq_latch & 0xff00) | data;
			break;
		case 0x7:
			m_irq_latch = (m_irq_latch & 0x00ff) | (data << 8);
			break;
		case 0x8:
			m_irq_enable = m_irq_enable_ack;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x9:
			m_irq_enable = BIT(data, 1);
			m_irq_enable_ack = BIT(data, 0);
			if (m_irq_enable)
				m_irq_count = m_irq_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0xa:
			set_nt_mirroring(BIT(data, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

void nes_konami_vrc5_device::m2_w(int state, offs_t address, bool write)
{
	if (state || !m_irq_enable)
		return;
	if (!++m_irq_count)
	{
		set_irq_line(ASSERT_LINE);
		m_irq_count = m_irq_latch;
	}
}

bool nes_konami_vrc5_device::background_fetch()
{
	// the same test as the MMC5's chr_access_kind()
	if (!m_ppu->dot_timing())
		return m_ppu->get_draw_phase() != PPU_DRAW_OAM;
	if (!m_ppu->rendering_now())
		return false;
	return m_ppu->sprite_fetch_slot() < 0;
}

u8 nes_konami_vrc5_device::chr_r(offs_t offset)
{
	if (background_fetch())
	{
		if (!BIT(m_qt_byte, 6))
			return m_vram[((BIT(m_qt_byte, 0) << 12) | (offset & 0xfff)) & (m_vram.size() - 1)];

		// Kanji ROM: 1 bpp, the second bit plane is all 1s or all 0s by QTRAM bit 7
		const u32 a = (u32(m_qt_byte & 0x3f) << 12) | (offset & 0xfff);
		if (BIT(a, 3))
			return BIT(m_qt_byte, 7) ? 0xff : 0x00;
		const u32 rom = ((a & 0x00007) << 1) | ((a & 0x00010) >> 4) | ((a & 0x3ffe0) >> 1);
		return m_vrom_size ? m_vrom[rom % m_vrom_size] : 0;
	}

	const u8 bank = BIT(offset, 12) ? 1 : BIT(m_reg[5], 0);
	return m_vram[((bank << 12) | (offset & 0xfff)) & (m_vram.size() - 1)];
}

void nes_konami_vrc5_device::chr_w(offs_t offset, u8 data)
{
	const u8 bank = BIT(offset, 12) ? 1 : BIT(m_reg[5], 0);
	m_vram[((bank << 12) | (offset & 0xfff)) & (m_vram.size() - 1)] = data;
}

u16 nes_konami_vrc5_device::qtram_index(offs_t offset) const
{
	// QTRAM shadows CIRAM: the same page for the same nametable
	const u8 page = BIT(m_reg[0xa], 1) ? BIT(offset, 11) : BIT(offset, 10);
	return (page << 10) | (offset & 0x3ff);
}

u8 nes_konami_vrc5_device::nt_r(offs_t offset)
{
	if (((offset & 0x3ff) < 0x3c0) && !machine().side_effects_disabled())
		m_qt_byte = m_qtram[qtram_index(offset)];
	return device_nes_cart_interface::nt_r(offset);
}

void nes_konami_vrc5_device::nt_w(offs_t offset, u8 data)
{
	if (BIT(m_reg[0xa], 0))
		m_qtram[qtram_index(offset)] = data;
	else
		device_nes_cart_interface::nt_w(offset, data);
}
