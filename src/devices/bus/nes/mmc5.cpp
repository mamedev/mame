// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-5 / ExROM

 iNES mappers: 5


 Here we emulate the Nintendo MMC-5 / ExROM PCBs [mapper 5]

 Source: NESdev "MMC5" (revision of 29 September 2026), which this file follows register by register.

 Reset: the chip's own reset detection (an M2 gap) sets only $5100 = 3, $5102 = 1, $5103 = 2, $5104 = 3, $5117 = $FF,
 $5130 = 0, $5200 = 0, $5201 = 0 and the scanline IRQ enable to 0; every other register, the IRQ pending flag and
 the scanline counter keep their values. A console reset is such a gap, so pcb_reset() sets only those.

 CHR: the MMC5 keeps two sets of 10-bit CHR registers. With 8x8 sprites only set A ($5120-$5127) is used. With 8x16
 sprites, when the chip has seen PPUCTRL bit 5 set and an E bit of PPUMASK set, set A serves the sprite fetches and
 set B the background fetches, and the set written last serves PPUDATA reads. The PPU tells the fetches apart
 (sprite_fetch_slot() and rendering_now()); the chip counts fetches since its scanline detection.

 Substitutions (independent 8x16 banks, extended attributes, vertical split) need an E bit and are made on rendering
 fetches only; a PPUDATA access is never substituted.

 NOT DOCUMENTED (the choice made here):
  - Power-on values of the registers NESdev gives no reset value for: as listed in device_start().
  - A CPU write to ExRAM in $5104 mode 0 or 1 outside rendering ("ignored or corruption"): 0 is written.
  - The bits NESdev marks "?" when a CHR register is written in 4K or 8K mode: left unchanged.
  - $5204 bits 0-5 (bit 0, the M2/CPU A0 sequence latch, has no documented trigger): read as 0.
  - When in vblank $5201 is reloaded into the split counter, and which split row the two tiles prefetched at the end
    of a line use: $5201 is added to the scanline count as it stands at each fetch.
  - The MMC5A registers ($5207-$520A, $5800-$5BFF) are not emulated: which carts have an MMC5A is not known.

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc5.h"

#include "speaker.h"

#define LOG_UNHANDLED (1U << 1)
#define LOG_BANK_INFO (1U << 2)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_UNHANDLED | LOG_GENERAL)
#else
#define VERBOSE (LOG_UNHANDLED)
#endif
#include "logmacro.h"


#define LAST_CHR_REG_A 0
#define LAST_CHR_REG_B 1

static const uint8_t m_mmc5_attrib[4] = {0x00, 0x55, 0xaa, 0xff};

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_EXROM, nes_exrom_device, "nes_exrom", "NES Cart ExROM (MMC-5) PCB")


nes_exrom_device::nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_EXROM, tag, owner, clock), m_irq_count(0)
	, m_irq_status(0), m_irq_enable(false), m_pcm_irq(false), m_mult1(0), m_mult2(0), m_floodtile(0), m_floodattr(0)
	, m_prg_mode(3), m_chr_mode(0), m_wram_protect_1(0), m_wram_protect_2(0), m_exram_control(0), m_wram_base(0), m_last_chr(0)
	, m_ex_byte(0), m_high_chr(0), m_split_scr(0), m_split_rev(0), m_split_ctrl(0), m_split_yst(0), m_split_bank(0)
	, m_ppu(*this, ":ppu") // FIXME: this dependency should not exist
	, m_sound(*this, "mmc5snd")
{
}

nes_exrom_device::~nes_exrom_device()
{
}


void nes_exrom_device::device_start()
{
	common_start();

	// power-on values NESdev does not give (the registers without a reset value): the ones this file always used
	for (auto &elem : m_vrom_bank)
		elem = 0x3ff;
	m_prg_regs[0] = 0xfc;
	m_prg_regs[1] = 0xfd;
	m_prg_regs[2] = 0xfe;
	m_prg_regs[3] = 0xff;
	for (int i = 0; i < 4; i++)
	{
		m_slot_ram[i] = false;
		m_slot_ram_bank[i] = 0;
	}
	m_last_chr = LAST_CHR_REG_A;
	std::fill(std::begin(m_exram), std::end(m_exram), 0);

	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_pcm_irq));
	save_item(NAME(m_mult1));
	save_item(NAME(m_mult2));
	save_item(NAME(m_floodtile));
	save_item(NAME(m_floodattr));
	save_item(NAME(m_prg_mode));
	save_item(NAME(m_chr_mode));
	save_item(NAME(m_wram_protect_1));
	save_item(NAME(m_wram_protect_2));
	save_item(NAME(m_exram_control));
	save_item(NAME(m_wram_base));
	save_item(NAME(m_vrom_bank));
	save_item(NAME(m_last_chr));
	save_item(NAME(m_snoop_ctrl));
	save_item(NAME(m_snoop_mask));
	save_item(NAME(m_prg_regs));
	save_item(NAME(m_slot_ram));
	save_item(NAME(m_slot_ram_bank));
	save_item(NAME(m_ex_byte));
	save_item(NAME(m_high_chr));
	save_item(NAME(m_split_scr));
	save_item(NAME(m_split_rev));
	save_item(NAME(m_split_ctrl));
	save_item(NAME(m_split_yst));
	save_item(NAME(m_split_bank));
	save_item(NAME(m_last_ppu_addr));
	save_item(NAME(m_nt_match));
	save_item(NAME(m_ppu_idle));
	save_item(NAME(m_ppu_reading));
	save_item(NAME(m_scan_count));
	save_item(NAME(m_exram));
}

void nes_exrom_device::pcb_reset()
{
	// the values NESdev gives for the chip's reset detection; nothing else changes
	m_prg_mode = 3;
	m_wram_protect_1 = 1;
	m_wram_protect_2 = 2;
	m_exram_control = 3;
	m_prg_regs[3] = 0xff;
	m_high_chr = 0;
	m_split_scr = 0;
	m_split_rev = 0;
	m_split_ctrl = 0;
	m_split_yst = 0;
	m_irq_enable = false;     // the pending flag, $5203 and the scanline counter are kept
	update_prg();
	update_irq();

	if (!m_bus_tap_on)
	{
		m_bus_tap = cpu_space().install_write_tap(0x2000, 0x2001, "mmc5_ppu_snoop",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (offset == 0x2000)
					{
						m_snoop_ctrl = data;
						return;
					}
					const bool was_off = !BIT(m_snoop_mask, 3, 2);
					m_snoop_mask = data;
					if (!BIT(data, 3, 2))
					{
						// both E bits clear: not in frame, last address undefined
						m_irq_status &= ~0x40;
						m_last_ppu_addr = 0xffff;
						m_nt_match = 0;
					}
					else if (was_off)
						scanline_reset();   // both clear, then one set
				}, &m_bus_tap);
		m_status_tap = cpu_space().install_read_tap(0x2002, 0x2002, "mmc5_status_snoop",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (!machine().side_effects_disabled() && BIT(data, 7))
						scanline_reset();
				}, &m_status_tap);
		m_dma_tap = cpu_space().install_write_tap(0x4014, 0x4014, "mmc5_oamdma_snoop",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					scanline_reset();
				}, &m_dma_tap);
		m_bus_tap_on = true;
	}
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 ExROM (MMC5 based) board emulation

 Games: Castlevania III, Just Breed, many Koei titles

 iNES: mapper 5

 In MAME: Supported.

 Tested: Castlevania III - Dracula's Curse (USA), Uchuu Keibitai SDF (Japan) (split-screen intro), Just Breed
 (Japan) (extended attributes) and Metal Slader Glory (Japan) (4K CHR) play; Bandit Kings of Ancient China (USA),
 Uncharted Waters (USA) and Laser Invasion (USA) reach their scenario, character and briefing screens.

 -------------------------------------------------*/

// PRG: the register each CPU 8K slot uses in each mode (NESdev "PRG Bankswitching"); $5117 only ever maps ROM. A 16K
// window ignores bit 0 of its register and takes PRG A13 from CPU A13; the 32K window ignores bits 0 and 1
void nes_exrom_device::update_prg()
{
	static constexpr int reg_for_slot[4][4] = {
		{ 3, 3, 3, 3 },     // mode 0: $5117
		{ 1, 1, 3, 3 },     // mode 1: $5115, $5117
		{ 1, 1, 2, 3 },     // mode 2: $5115, $5116, $5117
		{ 0, 1, 2, 3 } };   // mode 3: $5114-$5117
	static constexpr u8 size_mask[4][4] = {
		{ 3, 3, 3, 3 },
		{ 1, 1, 1, 1 },
		{ 1, 1, 0, 0 },
		{ 0, 0, 0, 0 } };

	for (int slot = 0; slot < 4; slot++)
	{
		const int reg = reg_for_slot[m_prg_mode][slot];
		const u8 mask = size_mask[m_prg_mode][slot];
		const u8 value = m_prg_regs[reg];
		const u8 bank = (value & ~mask & 0x7f) | (slot & mask);
		m_slot_ram[slot] = (reg != 3) && !BIT(value, 7);
		m_slot_ram_bank[slot] = bank & 0x07;
		if (!m_slot_ram[slot])
		{
			switch (slot)
			{
			case 0: prg8_89(bank); break;
			case 1: prg8_ab(bank); break;
			case 2: prg8_cd(bank); break;
			case 3: prg8_ef(bank); break;
			}
		}
	}
}

// PRG-RAM: bits 0-1 select the 8K page in a chip, bit 2 the chip (NESdev "PRG-RAM configurations"). With two chips
// the first is the battery-backed one; with one chip, values 4-7 select no chip (open bus)
bool nes_exrom_device::ram_present(u8 bank) const
{
	if (!m_battery.empty() && !m_prgram.empty())
		return true;
	if (!m_battery.empty() || !m_prgram.empty())
		return !BIT(bank, 2);
	return false;
}

u8 nes_exrom_device::ram_r(u8 bank, offs_t offset)
{
	if (!ram_present(bank))
		return get_open_bus();
	std::vector<u8> &chip = (!m_battery.empty() && (m_prgram.empty() || !BIT(bank, 2))) ? m_battery : m_prgram;
	return chip[(((bank & 3) * 0x2000) + (offset & 0x1fff)) & (chip.size() - 1)];
}

void nes_exrom_device::ram_w(u8 bank, offs_t offset, u8 data)
{
	if (!ram_writable() || !ram_present(bank))
		return;
	std::vector<u8> &chip = (!m_battery.empty() && (m_prgram.empty() || !BIT(bank, 2))) ? m_battery : m_prgram;
	chip[(((bank & 3) * 0x2000) + (offset & 0x1fff)) & (chip.size() - 1)] = data;
}

void nes_exrom_device::update_irq()
{
	set_irq_line(((BIT(m_irq_status, 7) && m_irq_enable) || m_pcm_irq) ? ASSERT_LINE : CLEAR_LINE);
}

// "in frame" cleared, scanline IRQ acknowledged, scanline counter reset (NESdev: NMI vector fetch, $2001 E bits from
// both clear to set, OAMDMA, PPUSTATUS read with bit 7 set, the 241st scanline)
void nes_exrom_device::scanline_reset()
{
	m_irq_status &= ~0xc0;
	m_last_ppu_addr = 0xffff;
	m_nt_match = 0;
	m_scan_count = 0;
	update_irq();
}


void nes_exrom_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	if (m_ppu->dot_timing())
		return;     // the dot-timed PPU's reads drive the scanline detection, see ppu_read_snoop()

	// PPUs without dot timing: once-per-line approximation
	m_scan_count = scanline;
	if (scanline == m_irq_count && m_irq_count != 0)
	{
		m_irq_status |= 0x80;
		update_irq();
	}

	// "In Frame" flag
	if (scanline == 0)
		m_irq_status |= 0x40;
	else if (scanline > ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
		m_irq_status &= ~0x40;
}

// NESdev "Scanline Detection and Scanline IRQ": three consecutive reads of the same address in $2000-$2FFF, and the
// scanline is detected at the read that follows (the attribute fetch at PPU dot 4, where NESdev puts the IRQ)
void nes_exrom_device::ppu_read_snoop(u16 addr)
{
	if (!m_ppu->dot_timing())
		return;

	m_ppu_reading = true;

	if (m_nt_match >= 2)
	{
		if (!BIT(m_irq_status, 6))
		{
			m_irq_status |= 0x40;
			m_scan_count = 0;
			// a compare value of 0 or 240-255 never fires, and clears a pending IRQ when the frame starts (NESdev forum t=658)
			if (m_irq_count == 0 || m_irq_count >= 240)
			{
				m_irq_status &= ~0x80;
				update_irq();
			}
		}
		else if (m_scan_count == 239)
			scanline_reset();   // the 241st scanline
		else
		{
			m_scan_count++;
			if (m_scan_count == m_irq_count)
			{
				m_irq_status |= 0x80;
				update_irq();
			}
		}
	}
	else if (addr >= 0x2000 && addr <= 0x2fff && addr == m_last_ppu_addr)
		m_nt_match++;
	if (addr != m_last_ppu_addr)
		m_nt_match = 0;

	m_last_ppu_addr = addr;
}

// M2 rise: in frame ends when the PPU has not read for 3 M2 rises; a read of $FFFA/$FFFB (the NMI vector) resets the
// scanline detection. The PPU is caught up first so that every read it made before this rise has been seen
void nes_exrom_device::m2_w(int state, offs_t address, bool write)
{
	if (!state || !m_ppu->dot_timing())
		return;

	m_ppu->catch_up();
	if (m_ppu_reading)
		m_ppu_idle = 0;
	else if (m_ppu_idle < 3 && ++m_ppu_idle == 3)
	{
		m_irq_status &= ~0x40;
		m_last_ppu_addr = 0xffff;
		m_nt_match = 0;
	}
	m_ppu_reading = false;

	if (!write && (address == 0xfffa || address == 0xfffb))
		scanline_reset();
}


void nes_exrom_device::pcm_irq(int state)
{
	m_pcm_irq = state;
	update_irq();
}


void nes_exrom_device::set_mirror(int page, int src)
{
	switch (src)
	{
	case 0:
		set_nt_page(page, CIRAM, 0, 1);
		break;
	case 1:
		set_nt_page(page, CIRAM, 1, 1);
		break;
	case 2:
		set_nt_page(page, EXRAM, 0, 1);
		break;
	case 3:
		set_nt_page(page, MMC5FILL, 0, 0);
		break;
	default:
		fatalerror("This should never happen\n");
	}
}

// a fetch of the PPU's rendering, as opposed to a PPUDATA access
bool nes_exrom_device::rendering_fetch()
{
	return !m_ppu->dot_timing() || m_ppu->rendering_now();
}

// the split region by the count of background tiles fetched for the line (NESdev "Horizontal Behavior")
bool nes_exrom_device::in_split()
{
	int const tile = m_ppu->get_tilenum();

	if (tile >= 34)
		return false;
	return m_split_rev ? (tile >= m_split_ctrl) : (tile < m_split_ctrl);
}

// the split region's row: $5201 plus the scanline count; from below 240 it rolls over from 239 to 0, and 240-255 count
// on to 255 and then roll over to 0 (NESdev "Vertical Behavior")
int nes_exrom_device::split_y()
{
	int y = m_split_yst + m_scan_count;
	if (m_split_yst >= 240)
	{
		if (y < 256)
			return y;
		y -= 256;
	}
	return y % 240;
}

uint8_t nes_exrom_device::nt_r(offs_t offset)
{
	int const page = ((offset & 0xc00) >> 10);
	bool const attr = (offset & 0x3ff) >= 0x3c0;

	// vertical split: ExRAM is the nametable, without extended attributes
	if (split_active())
	{
		int const tile = m_ppu->get_tilenum() & 0x1f;
		int const y = split_y();
		if (attr)
			return m_exram[0x3c0 | ((y >> 5) << 3) | (tile >> 2)];
		return m_exram[((y >> 3) << 5) | tile];
	}

	bool const ext = ext_attr_active();
	if (ext)
	{
		// extended attributes, for every nametable source: the byte at the same offset of ExRAM
		if (attr)
			return m_mmc5_attrib[m_ex_byte >> 6];
		if (!machine().side_effects_disabled())
			m_ex_byte = m_exram[offset & 0x3ff];
	}

	switch (m_nt_src[page])
	{
	case MMC5FILL:
		if (attr)
			return m_floodattr;
		return m_floodtile;

	case EXRAM:
		// modes 2 and 3: the nametable reads as all zeros
		return BIT(m_exram_control, 1) ? 0x00 : m_exram[offset & 0x3ff];

	case CIRAM:
	default:
		return m_nt_access[page][offset & 0x3ff];
	}
}

void nes_exrom_device::nt_w(offs_t offset, uint8_t data)
{
	int const page = ((offset & 0xc00) >> 10);

	if (!m_nt_writable[page])
		return;

	switch (m_nt_src[page])
	{
	case EXRAM:
		// PPUDATA reaches ExRAM in modes 0 and 1 only
		if (!BIT(m_exram_control, 1))
			m_exram[offset & 0x3ff] = data;
		break;

	case CIRAM:
	default:
		m_nt_access[page][offset & 0x3ff] = data;
		break;
	}
}

// CHR A19-A10 from a 10-bit register: A19 down to the bank size's top bit come from the register, the rest from the
// PPU address (NESdev "CHR select")
// reg is the 1K slot's register in 1K mode ($5120-$5127 = 0-7, $5128-$512B = 8-11); a larger bank uses the last register
// of its group: 2K $5121/$5123/$5125/$5127/$5129/$512B, 4K $5123/$5127/$512B, 8K $5127/$512B
u32 nes_exrom_device::chr_addr(int reg, offs_t offset) const
{
	switch (m_chr_mode)
	{
	case 0:  return (u32(m_vrom_bank[(reg < 8) ? 7 : 11] >> 3) << 13) | (offset & 0x1fff);
	case 1:  return (u32(m_vrom_bank[reg | 3] >> 2) << 12) | (offset & 0x0fff);
	case 2:  return (u32(m_vrom_bank[reg | 1] >> 1) << 11) | (offset & 0x07ff);
	default: return (u32(m_vrom_bank[reg]) << 10) | (offset & 0x03ff);
	}
}

// CHR memory read: CHR-ROM, or the board's CHR-RAM when it has none (CHR-RAM MMC5 images have no CHR-ROM pointer to read)
uint8_t nes_exrom_device::chr_mem_r(uint32_t addr)
{
	if (m_vrom_size)
		return m_vrom[addr & (m_vrom_size - 1)];
	if (get_vram_size())
		return m_vram[addr & (get_vram_size() - 1)];
	return get_open_bus();
}

// The chip tells sprite fetches from background fetches by counting the fetches since the scanline start; the PPU knows
// which it is: sprite_fetch_slot() is set from dot 257 to 320 of a rendered line and rendering_now() is false in vblank
// and forced blank, where a CHR access can only be a PPUDATA read. A PPU without timing of its own has only its draw phase
nes_exrom_device::chr_access nes_exrom_device::chr_access_kind()
{
	if (!m_ppu->dot_timing())
		return (m_ppu->get_draw_phase() == PPU_DRAW_OAM) ? chr_access::SPRITE : chr_access::BACKGROUND;
	if (!m_ppu->rendering_now())
		return chr_access::DATA;
	return (m_ppu->sprite_fetch_slot() >= 0) ? chr_access::SPRITE : chr_access::BACKGROUND;
}

uint8_t nes_exrom_device::chr_r(offs_t offset)
{
	int const bank = (offset >> 10) & 7;
	chr_access const kind = chr_access_kind();

	if (kind == chr_access::BACKGROUND && substitutions())
	{
		// vertical split (board in CL mode): CHR A19-A12 from $5202, A11-A0 from the PPU
		if (split_active())
			return chr_mem_r((u32(m_split_bank) << 12) | (offset & 0xfff));
		// extended attributes: CHR A17-A12 from the latched ExRAM byte, A19-A18 from $5130 as it is now
		if (m_exram_control == 1)
			return chr_mem_r((u32((m_ex_byte & 0x3f) | (m_high_chr << 6)) << 12) | (offset & 0xfff));
	}

	// 8x8 sprites, or the substitutions off: only $5120-$5127 are used, $5128-$512B are ignored
	if (!independent_banks())
		return chr_mem_r(chr_addr(bank, offset));

	switch (kind)
	{
	case chr_access::SPRITE:
		return chr_mem_r(chr_addr(bank, offset));

	case chr_access::BACKGROUND:
		return chr_mem_r(chr_addr(8 + (bank & 3), offset));

	default:
		// PPUDATA: the set that was written to last
		if (m_last_chr == LAST_CHR_REG_A)
			return chr_mem_r(chr_addr(bank, offset));
		return chr_mem_r(chr_addr(8 + (bank & 3), offset));
	}
}


uint8_t nes_exrom_device::read_l(offs_t offset)
{
	int value;
	LOG("exrom read_l, offset: %04x\n", offset);
	offset += 0x100;

	if ((offset >= 0x1000) && (offset <= 0x1015))
		return m_sound->read(offset & 0x1f);

	if ((offset >= 0x1c00) && (offset <= 0x1fff))
	{
		// EXRAM
		if (BIT(m_exram_control, 1))    // Modes 2,3 = read
			return m_exram[offset - 0x1c00];
		else
			return get_open_bus();   // Modes 0,1 = open bus
	}

	switch (offset)
	{
	case 0x1204:
		value = m_irq_status & 0xc0;
		if (!machine().side_effects_disabled())
		{
			m_irq_status &= ~0x80;
			update_irq();
		}
		return value;

	case 0x1205:
		return (m_mult1 * m_mult2) & 0xff;

	case 0x1206:
		return ((m_mult1 * m_mult2) & 0xff00) >> 8;

	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_UNHANDLED, "MMC5 uncaught read, offset: %04x\n", offset + 0x4100);
		return get_open_bus();
	}
}


void nes_exrom_device::write_l(offs_t offset, uint8_t data)
{
	LOG("exrom write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	if ((offset >= 0x1000) && (offset <= 0x1015))
	{
		m_sound->write(offset & 0x1f, data);
		return;
	}

	if ((offset >= 0x1c00) && (offset <= 0x1fff))
	{
		// EXRAM
		if (m_exram_control == 0x02)    // Mode 2 = write data
			m_exram[offset - 0x1c00] = data;
		else if (m_exram_control != 0x03)   // Modes 0,1 = write data in frame / write 0 otherwise
		{
			if (BIT(m_irq_status, 6))
				m_exram[offset - 0x1c00] = data;
			else
				m_exram[offset - 0x1c00] = 0x00;
		}
		// Mode 3 = read only
		return;
	}

	switch (offset)
	{
	case 0x1100:
		m_prg_mode = data & 0x03;
		update_prg();
		LOGMASKED(LOG_BANK_INFO, "MMC5 rom bank mode: %02x\n", data);
		break;

	case 0x1101:
		m_chr_mode = data & 0x03;
		LOGMASKED(LOG_BANK_INFO, "MMC5 vrom bank mode: %02x\n", data);
		break;

	case 0x1102:
		m_wram_protect_1 = data & 0x03;
		break;

	case 0x1103:
		m_wram_protect_2 = data & 0x03;
		break;

	case 0x1104: // Extra VRAM (EXRAM)
		m_exram_control = data & 0x03;
		break;

	case 0x1105:
		set_mirror(0, (data & 0x03) >> 0);
		set_mirror(1, (data & 0x0c) >> 2);
		set_mirror(2, (data & 0x30) >> 4);
		set_mirror(3, (data & 0xc0) >> 6);
		break;

	case 0x1106:
		m_floodtile = data;
		break;

	case 0x1107:
		m_floodattr = m_mmc5_attrib[data & 3];
		break;

	case 0x1113:
		m_wram_base = data & 0x07;
		break;

	case 0x1114:
	case 0x1115:
	case 0x1116:
	case 0x1117:
		m_prg_regs[offset & 3] = data;
		update_prg();
		break;

	case 0x1120: case 0x1121: case 0x1122: case 0x1123:
	case 0x1124: case 0x1125: case 0x1126: case 0x1127:
	case 0x1128: case 0x1129: case 0x112a: case 0x112b:
	{
		// the 10 bits are stored as NESdev lays them out for the CHR mode at the time of the write; bits it marks "?"
		// keep their value
		u16 &reg = m_vrom_bank[offset & 0x0f];
		switch (m_chr_mode)
		{
		case 0: reg = (reg & 0x006) | (u16(data & 0x7f) << 3) | (data & 1); break;
		case 1: reg = (reg & 0x002) | (u16(data) << 2) | (data & 1); break;
		case 2: reg = (u16(m_high_chr & 1) << 9) | (u16(data) << 1) | (data & 1); break;
		default: reg = (u16(m_high_chr) << 8) | data; break;
		}
		m_last_chr = (offset >= 0x1128) ? LAST_CHR_REG_B : LAST_CHR_REG_A;
		break;
	}

	case 0x1130:
		m_high_chr = data & 0x03;
		break;

	case 0x1200:
		m_split_scr = BIT(data, 7);
		m_split_rev = BIT(data, 6);
		m_split_ctrl = data & 0x1f;
		break;

	case 0x1201:
		m_split_yst = data;
		break;

	case 0x1202:
		m_split_bank = data;
		break;

	case 0x1203:
		m_irq_count = data;
		break;

	case 0x1204:
		m_irq_enable = BIT(data, 7);
		update_irq();
		break;

	case 0x1205:
		m_mult1 = data;
		break;

	case 0x1206:
		m_mult2 = data;
		break;

	default:
		LOG("MMC5 uncaught write, offset: %04x, data: %02x\n", offset + 0x4100, data);
		break;
	}
}

// $6000-$7FFF: PRG-RAM from $5113 in every mode
uint8_t nes_exrom_device::read_m(offs_t offset)
{
	return ram_r(m_wram_base, offset);
}

void nes_exrom_device::write_m(offs_t offset, uint8_t data)
{
	ram_w(m_wram_base, offset, data);
}

// $8000-$FFFF: ROM, or PRG-RAM where the register for the slot selects it (Bandit Kings of Ancient China writes
// PRG-RAM through $8000-$DFFF)
uint8_t nes_exrom_device::read_h(offs_t offset)
{
	int const slot = offset >> 13;
	uint8_t const ret = m_slot_ram[slot] ? ram_r(m_slot_ram_bank[slot], offset) : hi_access_rom(offset);
	if (!machine().side_effects_disabled() && BIT(~offset, 14))
		m_sound->pcm_w(ret);
	return ret;
}

void nes_exrom_device::write_h(offs_t offset, uint8_t data)
{
	int const slot = offset >> 13;
	if (m_slot_ram[slot])
		ram_w(m_slot_ram_bank[slot], offset, data);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_exrom_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware, mixed into the console audio through the cartridge connector

	MMC5_SOUND(config, m_sound, XTAL(21'477'272)/12);
	m_sound->irq().set(FUNC(nes_exrom_device::pcm_irq));
	// level measured with nes-audio-tests db_mmc5 against the hardware survey (+1.0 dB vs a full-volume APU square)
	m_sound->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.01);
}
