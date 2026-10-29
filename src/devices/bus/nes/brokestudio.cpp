// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Broke Studio PCBs

 iNES mappers: 682


 Here we emulate the following homebrew PCBs

 * Broke Studio Rainbow (RNBW) [mapper 682]


 Rainbow is an FPGA board for homebrew releases (documented in BrokeStudio's rainbow-net repository, NES/mapper-doc.md,
 which this board follows; the fetch-level behavior that document leaves open follows Mesen2):

  - PRG: five banking modes (32K, 16K+16K, 16K+8K+8K, four 8K, eight 4K) for $8000-$FFFF, each bank selecting
    PRG-ROM or PRG-RAM; $6000-$7FFF in one 8K or two 4K banks selecting PRG-ROM, PRG-RAM or FPGA-RAM
  - CHR: five banking modes (8K down to 512 bytes) from CHR-ROM, CHR-RAM, FPGA-RAM or CIRAM
  - each nametable can come from CIRAM, CHR-RAM, FPGA-RAM or CHR-ROM; fill mode; attribute and background extended
    modes (per tile palette, per tile 4K CHR bank); window split
  - 8K of FPGA-RAM shared by the CPU and the PPU, with an auto-incrementing reader and writer
  - a scanline IRQ close to the MMC5's (with a dot offset and a jitter counter), a 16-bit CPU cycle IRQ with cycle
    parity, NMI and IRQ vector redirection
  - generated 6502 routines at $4280 and $4282 that update OAM
  - expansion audio: two pulse channels and a sawtooth that work like the VRC6's (the VRC6 sound device is used)

  - Sprite Extended Mode: a 4 KiB (8x8) or 8 KiB (8x16) CHR bank per sprite from $4200-$423F and $4240; like the
    FPGA, which cannot see the PPU's OAM, the board copies each sprite's Y from the CPU's $2003/$2004 writes (OAM
    DMA included) and the sprite size from $2000, and picks each line's first eight sprites itself (Mesen2)

 Not emulated: the ESP8266 Wi-Fi chip (registers $4190-$4194 read as idle), the self-flashing of PRG-ROM and
 CHR-ROM, the IPCM mode of the audio output ($4011) and the automatic IRQ acknowledge on a $4011 read.

 NES 2.0: mapper 682

 In MAME: Preliminary supported.

 ***********************************************************************************************************/


#include "emu.h"
#include "brokestudio.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructors
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_RAINBOW, nes_rainbow_device, "nes_rainbow", "NES Cart Broke Studio Rainbow PCB")

nes_rainbow_device::nes_rainbow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_RAINBOW, tag, owner, clock)
	, m_ppu(*this, ":ppu") // as nes_exrom_device: the in-frame timeout needs to know whether the PPU is rendering
	, m_vrc6snd(*this, "vrc6snd")
{
}


void nes_rainbow_device::device_start()
{
	nes_nrom_device::device_start();

	save_item(NAME(m_oam_y));
	save_item(NAME(m_oam_addr));
	save_item(NAME(m_large_sprites));
	save_item(NAME(m_oam_map));

	std::fill(std::begin(m_fpga), std::end(m_fpga), 0);

	save_item(NAME(m_hi_mode));
	save_item(NAME(m_lo_mode));
	save_item(NAME(m_hi));
	save_item(NAME(m_lo));
	save_item(NAME(m_hi_type));
	save_item(NAME(m_hi_off));
	save_item(NAME(m_lo_type));
	save_item(NAME(m_lo_off));
	save_item(NAME(m_fpga_bank));
	save_item(NAME(m_chr_mode));
	save_item(NAME(m_chr_select));
	save_item(NAME(m_window_en));
	save_item(NAME(m_sprite_ext));
	save_item(NAME(m_bgext_offset));
	save_item(NAME(m_chr_reg));
	save_item(NAME(m_chr_off));
	save_item(NAME(m_nt_bank));
	save_item(NAME(m_nt_ctl));
	save_item(NAME(m_fill_tile));
	save_item(NAME(m_fill_attr));
	save_item(NAME(m_win_ctl));
	save_item(NAME(m_win_bank));
	save_item(NAME(m_win_x1));
	save_item(NAME(m_win_x2));
	save_item(NAME(m_win_y1));
	save_item(NAME(m_win_y2));
	save_item(NAME(m_win_scroll_x));
	save_item(NAME(m_win_scroll_y));
	save_item(NAME(m_in_window));
	save_item(NAME(m_sl_en));
	save_item(NAME(m_sl_pend));
	save_item(NAME(m_sl_line));
	save_item(NAME(m_sl_offset));
	save_item(NAME(m_last_ppu_addr));
	save_item(NAME(m_scanline_counter));
	save_item(NAME(m_ppu_idle));
	save_item(NAME(m_nt_read_counter));
	save_item(NAME(m_ppu_read_counter));
	save_item(NAME(m_in_frame));
	save_item(NAME(m_in_hblank));
	save_item(NAME(m_jitter));
	save_item(NAME(m_parity));
	save_item(NAME(m_nt_fetch_counter));
	save_item(NAME(m_last_sprite_slot));
	save_item(NAME(m_override_tile_fetch));
	save_item(NAME(m_ext_data));
	save_item(NAME(m_cpu_irq_counter));
	save_item(NAME(m_cpu_irq_latch));
	save_item(NAME(m_cpu_irq_en));
	save_item(NAME(m_cpu_irq_pend));
	save_item(NAME(m_cpu_irq_en_after_ack));
	save_item(NAME(m_cpu_irq_ack_4011));
	save_item(NAME(m_irq_asserted));
	save_item(NAME(m_fpga_addr));
	save_item(NAME(m_fpga_inc));
	save_item(NAME(m_nmi_vec_en));
	save_item(NAME(m_irq_vec_en));
	save_item(NAME(m_nmi_vec));
	save_item(NAME(m_irq_vec));
	save_item(NAME(m_oam_slow_page));
	save_item(NAME(m_oam_ext_page));
	save_item(NAME(m_oam_limit));
	save_item(NAME(m_oam_code));
	save_item(NAME(m_oam_locked));
	save_item(NAME(m_sprite_ext_data));
	save_item(NAME(m_sprite_ext_bank));
	save_item(NAME(m_esp_en));
	save_item(NAME(m_wifi_irq_en));
	save_item(NAME(m_send_src));
	save_item(NAME(m_recv_dst));
	save_item(NAME(m_audio_ctl));
	save_item(NAME(m_audio_vol));
	save_item(NAME(m_fpga));
}

void nes_rainbow_device::device_add_mconfig(machine_config &config)
{
	// the pulse and sawtooth channels work like the VRC6's; the level is the VRC6's, not measured on this board
	VRC6(config, m_vrc6snd, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.175);
}


//-------------------------------------------------
//  memory sources
//-------------------------------------------------

static inline u32 wrap_offset(u32 offset, u32 size)
{
	return (size & (size - 1)) ? (offset % size) : (offset & (size - 1));
}

u8 *nes_rainbow_device::work_ram(u32 &size)
{
	if (!m_battery.empty())
	{
		size = m_battery.size();
		return &m_battery[0];
	}
	size = m_prgram.size();
	return size ? &m_prgram[0] : nullptr;
}

u8 nes_rainbow_device::read_mem(int type, u32 offset)
{
	switch (type)
	{
		case 0:
			if (m_prg_size)
				return m_prg[wrap_offset(offset, m_prg_size)];
			break;

		case 1:
		{
			u32 size;
			u8 *ram = work_ram(size);
			if (ram)
				return ram[wrap_offset(offset, size)];
			break;
		}

		case 2:
			return m_fpga[offset & (FPGA_RAM_SIZE - 1)];
	}
	return get_open_bus();
}

void nes_rainbow_device::write_mem(int type, u32 offset, u8 data)
{
	switch (type)
	{
		case 1:
		{
			u32 size;
			u8 *ram = work_ram(size);
			if (ram)
				ram[wrap_offset(offset, size)] = data;
			break;
		}

		case 2:
			m_fpga[offset & (FPGA_RAM_SIZE - 1)] = data;
			break;
	}
}

u8 nes_rainbow_device::read_chr_abs(u32 addr)
{
	switch (m_chr_select)
	{
		case 0:
			return m_vrom_size ? m_vrom[wrap_offset(addr, m_vrom_size)] : 0;
		case 1:
			return m_vram.empty() ? 0 : m_vram[wrap_offset(addr, m_vram.size())];
		default:
			return m_fpga[addr & (FPGA_RAM_SIZE - 1)];
	}
}


//-------------------------------------------------
//  banking
//-------------------------------------------------

void nes_rainbow_device::map_high(int slot, int slots, int reg)
{
	const u32 size = slots * 0x1000;
	const bool ram = BIT(m_hi[reg], 15);
	const u32 index = m_hi[reg] & 0x07ff;
	for (int i = 0; i < slots; i++)
	{
		m_hi_type[slot + i] = ram ? 1 : 0;
		m_hi_off[slot + i] = index * size + i * 0x1000;
	}
}

void nes_rainbow_device::map_low(int slot, int slots, int reg)
{
	const u32 size = slots * 0x1000;
	const int chip = m_lo[reg] >> 14;                       // 0x: PRG-ROM, 10: PRG-RAM, 11: FPGA-RAM
	const u32 index = m_lo[reg] & 0x0fff;
	for (int i = 0; i < slots; i++)
	{
		m_lo_type[slot + i] = (chip < 2) ? 0 : (chip == 2) ? 1 : 2;
		m_lo_off[slot + i] = index * size + i * 0x1000;
	}
}

void nes_rainbow_device::update_state()
{
	switch (m_hi_mode & 7)
	{
		case 0:
			map_high(0, 8, 0);
			break;
		case 1:
			map_high(0, 4, 0);
			map_high(4, 4, 4);
			break;
		case 2:
			map_high(0, 4, 0);
			map_high(4, 2, 4);
			map_high(6, 2, 6);
			break;
		case 3:
			map_high(0, 2, 0);
			map_high(2, 2, 2);
			map_high(4, 2, 4);
			map_high(6, 2, 6);
			break;
		default:
			for (int i = 0; i < 8; i++)
				map_high(i, 1, i);
			break;
	}

	if (m_lo_mode)
	{
		map_low(0, 1, 0);
		map_low(1, 1, 1);
	}
	else
		map_low(0, 2, 0);

	// 8K, 4K, 2K, 1K or 512 byte CHR banks (the doc's 1xx is mode 4)
	const int mode = BIT(m_chr_mode, 2) ? 4 : (m_chr_mode & 3);
	const int count = 1 << mode;
	const u32 bank_size = 0x2000 >> mode;
	const int slots = bank_size / 0x200;
	for (int i = 0; i < count; i++)
		for (int s = 0; s < slots; s++)
			m_chr_off[i * slots + s] = m_chr_reg[i] * bank_size + s * 0x200;
}


//-------------------------------------------------
//  IRQ
//-------------------------------------------------

void nes_rainbow_device::update_irq()
{
	const bool active = (m_cpu_irq_en && m_cpu_irq_pend) || (m_sl_en && m_sl_pend);
	if (active && !m_irq_asserted)
		m_jitter = 0;
	m_irq_asserted = active;
	set_irq_line(active ? ASSERT_LINE : CLEAR_LINE);
}

void nes_rainbow_device::ack_cpu_irq()
{
	// acknowledging does not reload the counter; A (enable after acknowledge) decides whether it keeps running
	m_cpu_irq_en = m_cpu_irq_en_after_ack;
	m_cpu_irq_pend = false;
	update_irq();
}

void nes_rainbow_device::m2_w(int state, offs_t address, bool write)
{
	if (state)
		return;

	m_jitter++;
	m_parity ^= 1;

	if (m_cpu_irq_en && m_cpu_irq_counter && --m_cpu_irq_counter == 0)
	{
		m_cpu_irq_counter = m_cpu_irq_latch;
		m_cpu_irq_pend = true;
		update_irq();
	}

	// the in-frame flag is cleared when no PPU read has happened for three CPU cycles. In MAME the CPU runs ahead of the
	// PPU, whose reads arrive here in bursts, so counting M2 cycles between them would end the frame mid-line; while the
	// PPU renders it reads every other dot, so the count restarts (as nes_exrom_device does for the MMC5)
	if (m_ppu->dot_timing() && m_ppu->rendering_now())
		m_ppu_idle = 3;
	if (m_ppu_idle && --m_ppu_idle == 0)
	{
		m_in_frame = false;
		m_in_hblank = false;
		m_scanline_counter = -1;
		m_nt_fetch_counter = 0;
		m_last_sprite_slot = -1;
		m_nt_read_counter = 0;
	}
}


//-------------------------------------------------
//  OAM routines
//-------------------------------------------------

// OAM slow update: LDA #$00 / STA $2003, then LDA #[FPGA-RAM byte] / STA $2004 for every byte of sprites 0 to the limit
void nes_rainbow_device::generate_oam_slow_update()
{
	if (m_oam_locked)
		return;
	m_oam_locked = true;

	const int bytes = (m_oam_limit + 1) * 4;
	int i = 0;
	m_oam_code[i++] = 0xa9;     // LDA #$00
	m_oam_code[i++] = 0x00;
	m_oam_code[i++] = 0x8d;     // STA $2003
	m_oam_code[i++] = 0x03;
	m_oam_code[i++] = 0x20;
	for (int j = 0; j < bytes; j++)
	{
		m_oam_code[i++] = 0xa9; // LDA #[FPGA-RAM value]
		m_oam_code[i++] = m_fpga[0x1800 + (m_oam_slow_page * 0x100) + j];
		m_oam_code[i++] = 0x8d; // STA $2004
		m_oam_code[i++] = 0x04;
		m_oam_code[i++] = 0x20;
	}
	m_oam_code[i++] = 0x60;     // RTS
}

// OAM ext update: LDA #[FPGA-RAM byte, one every four] / STA $42xx for each sprite up to the limit
void nes_rainbow_device::generate_oam_ext_update()
{
	if (m_oam_locked)
		return;
	m_oam_locked = true;

	int i = 2;
	for (int spr = 0; spr <= m_oam_limit; spr++)
	{
		m_oam_code[i++] = 0xa9; // LDA #[FPGA-RAM value]
		m_oam_code[i++] = m_fpga[0x1800 + (m_oam_ext_page * 0x100) + (spr * 4)];
		m_oam_code[i++] = 0x8d; // STA $42xx
		m_oam_code[i++] = spr;
		m_oam_code[i++] = 0x42;
	}
	m_oam_code[i++] = 0x60;     // RTS
}


//-------------------------------------------------
//  PPU side
//-------------------------------------------------

u8 nes_rainbow_device::pattern_data(u16 addr)
{
	switch (m_chr_select)
	{
		case 0:
			return m_vrom_size ? m_vrom[wrap_offset(m_chr_off[addr >> 9] + (addr & 0x1ff), m_vrom_size)] : 0;
		case 1:
			return m_vram.empty() ? 0 : m_vram[wrap_offset(m_chr_off[addr >> 9] + (addr & 0x1ff), m_vram.size())];
		case 2:
			return m_fpga[addr & 0xfff];                    // 4K mode forced, $0000-$0FFF mirrored at $1000-$1FFF
		default:
			return m_ciram[addr & 0x7ff];
	}
}

u8 nes_rainbow_device::nt_data(u16 addr)
{
	const int page = (addr >> 10) & 3;
	const u32 off = addr & 0x3ff;
	const u32 bank = m_nt_bank[page];
	switch (m_nt_ctl[page] >> 6)
	{
		case 0:
			return m_ciram[((bank & 1) * 0x400) + off];
		case 1:
			return m_vram.empty() ? 0 : m_vram[wrap_offset(bank * 0x400 + off, m_vram.size())];
		case 2:
			return m_fpga[((bank & 3) * 0x400) + off];
		default:
			return m_vrom_size ? m_vrom[wrap_offset(bank * 0x400 + off, m_vrom_size)] : 0;
	}
}

u8 nes_rainbow_device::internal_read(u16 addr)
{
	return (addr < 0x2000) ? pattern_data(addr) : nt_data(addr);
}

u8 nes_rainbow_device::chr_r(offs_t offset)
{
	return ppu_read(offset & 0x1fff);
}

void nes_rainbow_device::chr_w(offs_t offset, u8 data)
{
	const u16 addr = offset & 0x1fff;
	switch (m_chr_select)
	{
		case 1:
			if (!m_vram.empty())
				m_vram[wrap_offset(m_chr_off[addr >> 9] + (addr & 0x1ff), m_vram.size())] = data;
			break;
		case 2:
			m_fpga[addr & 0xfff] = data;
			break;
		case 3:
			m_ciram[addr & 0x7ff] = data;
			break;
		default:
			break;      // CHR-ROM (self-flashing is not emulated)
	}
}

u8 nes_rainbow_device::nt_r(offs_t offset)
{
	return ppu_read(0x2000 | (offset & 0xfff));
}

void nes_rainbow_device::nt_w(offs_t offset, u8 data)
{
	const int page = (offset >> 10) & 3;
	const u32 off = offset & 0x3ff;
	const u32 bank = m_nt_bank[page];
	switch (m_nt_ctl[page] >> 6)
	{
		case 0:
			m_ciram[((bank & 1) * 0x400) + off] = data;
			break;
		case 1:
			if (!m_vram.empty())
				m_vram[wrap_offset(bank * 0x400 + off, m_vram.size())] = data;
			break;
		case 2:
			m_fpga[((bank & 3) * 0x400) + off] = data;
			break;
		default:
			break;
	}
}

// Three identical nametable reads in a row start a scanline (as on the MMC5); the in-frame flag follows the first one
void nes_rainbow_device::detect_scanline_start(u16 addr)
{
	if (addr >= 0x2000 && addr <= 0x2fff)
	{
		if (m_last_ppu_addr == addr)
		{
			m_nt_read_counter++;

			if (m_nt_read_counter >= 2)
			{
				if (!m_in_frame)
				{
					m_in_frame = true;
					m_scanline_counter = 0;
				}
				else
					m_scanline_counter++;

				sprite_eval();
				m_nt_fetch_counter = 0;
				m_last_sprite_slot = -1;
				m_ppu_read_counter = 0;
				m_nt_read_counter = 0;
				m_in_hblank = false;
			}
		}
		else
			m_nt_read_counter = 0;
	}
	else
		m_nt_read_counter = 0;
}

void nes_rainbow_device::update_in_window()
{
	bool ymatch, xmatch;

	const u8 scanline = (m_nt_fetch_counter >= 41) ? m_scanline_counter + 1 : m_scanline_counter;
	if (m_win_y1 >= m_win_y2)
		ymatch = scanline <= m_win_y2 || scanline > m_win_y1;
	else
		ymatch = scanline >= m_win_y1 && scanline <= m_win_y2;

	const u8 column = (m_nt_fetch_counter + 1) % 42;
	if (m_win_x1 >= m_win_x2)
		xmatch = column <= m_win_x2 || column > m_win_x1;
	else
		xmatch = column >= m_win_x1 && column <= m_win_x2;

	m_in_window = xmatch && ymatch;
}

// Every PPU read, pattern or nametable, passes through here: the scanline and the in-frame flag are found by watching
// the fetches, and the extended modes and the window split replace nametable, attribute and tile data on the way
u8 nes_rainbow_device::ppu_read(u16 addr)
{
	m_ppu_read_counter++;
	detect_scanline_start(addr);

	if (m_sl_line == m_scanline_counter && m_sl_offset == m_ppu_read_counter)
	{
		m_sl_pend = true;
		update_irq();
	}

	m_ppu_idle = 3;
	m_last_ppu_addr = addr;

	if (!m_in_frame)
		return internal_read(addr);

	if (addr >= 0x2000 && addr <= 0x2fff)
	{
		// nametable or attribute fetch
		const bool is_attr = (addr & 0x3ff) >= 0x3c0;
		if (!is_attr)
		{
			// the PPU makes two garbage nametable-range fetches in each sprite slot (NESdev, PPU rendering); the counter keeps
			// counting one fetch per slot, so that the eight sprite slots are fetches 33-40 as the window and the background
			// extended modes below expect. Without a slot (rendering off, or a PPU with no timing of its own) every fetch counts.
			const int slot = m_ppu->sprite_fetch_slot();
			if ((slot < 0) || (slot != m_last_sprite_slot))
				m_nt_fetch_counter++;
			m_last_sprite_slot = slot;
			if (m_nt_fetch_counter == 33)
				m_in_hblank = true;
			m_in_window = false;

			if (m_window_en)
				update_in_window();
		}

		const u8 ctl = m_in_window ? m_win_ctl : m_nt_ctl[(addr >> 10) & 3];
		const bool attr_ext = BIT(ctl, 0);
		const bool bg_ext = BIT(ctl, 1);
		const u8 fpga_src = BIT(ctl, 2, 2);
		const bool fill = BIT(ctl, 5);

		u8 shift = 0;
		if (m_in_window)
		{
			const u8 scanline = (m_nt_fetch_counter >= 41) ? m_scanline_counter + 1 : m_scanline_counter;
			const u8 window_scanline = (scanline + m_win_scroll_y) % 240;
			const u8 column = (m_nt_fetch_counter + 1) % 42;
			const u16 nt_addr = ((window_scanline / 8) * 32) + ((column + m_win_scroll_x) & 0x1f);
			if (!is_attr)
				addr = nt_addr;
			else
			{
				addr = 0x3c0 + (((window_scanline >> 2) & 0xf8) | (((column + m_win_scroll_x) & 0x1f) >> 2));
				shift = ((nt_addr >> 4) & 0x04) | (nt_addr & 0x02);
			}
		}

		// background and attribute extended modes and fill mode
		if (!is_attr)
		{
			m_override_tile_fetch = bg_ext;
			if (attr_ext || bg_ext)
				m_ext_data = m_fpga[((fpga_src * 0x400) + (addr & 0x3ff)) & (FPGA_RAM_SIZE - 1)];
			if (fill)
				return m_fill_tile;
		}
		else
		{
			if (attr_ext)
				return ((m_ext_data & 0xc0) >> 6) * 0x55;
			else if (fill)
				return m_fill_attr * 0x55;
		}

		if (m_in_window)
		{
			// in the window (and neither fill nor an extended mode): the window nametable's data
			const u8 data = m_fpga[((m_win_bank * 0x400) + addr) & (FPGA_RAM_SIZE - 1)];
			return is_attr ? (((data >> shift) & 0x03) * 0x55) : data;
		}
	}
	else
	{
		// tile data fetches. The PPU says which sprite slot it is fetching (dots 257-320 of a rendered line); the dot-accurate PPU
		// makes two nametable-range fetches per slot there, so counting nametable fetches no longer tells the sprite fetches
		// from the background ones. Outside the sprite fetches the counter is still used (and without a slot, as on a PPU that
		// has no timing of its own)
		const int sprite_slot = m_ppu->sprite_fetch_slot();
		const bool is_bg_fetch = (sprite_slot < 0) && (m_nt_fetch_counter < 33 || m_nt_fetch_counter >= 41);
		if (m_in_window && is_bg_fetch)
		{
			const u8 scanline = (m_nt_fetch_counter >= 41) ? m_scanline_counter + 1 : m_scanline_counter;
			const u8 window_scanline = (scanline + m_win_scroll_y) % 240;
			if (m_override_tile_fetch)
				return read_chr_abs((addr & 0xff8) | (window_scanline & 0x07) | ((m_ext_data & 0x3f) << 12) | (m_bgext_offset << 18));
			else
				return internal_read((addr & 0x1ff8) | (window_scanline & 0x07));
		}
		else if (m_override_tile_fetch && is_bg_fetch)
			return read_chr_abs((addr & 0xfff) | ((m_ext_data & 0x3f) << 12) | (m_bgext_offset << 18));
		else if (m_sprite_ext && (sprite_slot >= 0))
		{
			// Sprite Extended Mode: the sprite in this fetch slot takes its CHR bank from $4200 + its OAM index (the board's own
			// evaluation of the line) and the bank set from $4240: 4 KiB banks for 8x8 sprites, 8 KiB banks for 8x16 sprites (Mesen2)
			const u8 spr = m_oam_map[sprite_slot & 7];
			if (m_large_sprites)
				return read_chr_abs((u32(m_sprite_ext_bank) << 21) | (u32(m_sprite_ext_data[spr]) << 13) | (addr & 0x1fff));
			return read_chr_abs((u32(m_sprite_ext_bank) << 20) | (u32(m_sprite_ext_data[spr]) << 12) | (addr & 0xfff));
		}
	}

	return internal_read(addr);
}


//-------------------------------------------------
//  CPU side
//-------------------------------------------------

u8 nes_rainbow_device::read_l(offs_t offset)
{
	const u16 addr = 0x4100 + offset;

	if (addr >= 0x4800 && addr < 0x5000)
		return m_fpga[0x1800 + (addr - 0x4800)];
	if (addr >= 0x5000)
		return m_fpga[(m_fpga_bank * 0x1000) + (addr - 0x5000)];

	switch (addr)
	{
		case 0x4100:
			return m_hi_mode | (m_lo_mode << 7);
		case 0x4120:
			return m_chr_mode | (m_window_en << 4) | (m_sprite_ext << 5) | (m_chr_select << 6);
		case 0x412a: case 0x412b: case 0x412c: case 0x412d:
			return m_nt_ctl[addr - 0x412a];
		case 0x412f:
			return m_win_ctl;

		case 0x4150:
			return m_in_frame ? (m_scanline_counter & 0xff) : 0;
		case 0x4151:
		{
			const u8 data = (m_in_hblank << 7) | (m_in_frame << 6);
			m_sl_pend = false;
			update_irq();
			return data;
		}
		case 0x4154:
			return m_jitter;
		case 0x4157:
			return m_parity << 7;

		case 0x415f:
		{
			const u8 data = m_fpga[m_fpga_addr];
			m_fpga_addr = (m_fpga_addr + m_fpga_inc) & (FPGA_RAM_SIZE - 1);
			return data;
		}

		case 0x4160:
			return 0x21;        // platform 1 (emulator), version 1
		case 0x4161:
			return (m_sl_pend << 7) | (m_cpu_irq_pend << 6);

		case 0x4190:
			return m_esp_en | (m_wifi_irq_en << 1);
		case 0x4191:
		case 0x4192:
			return 0;           // no ESP: no data ready, nothing received or sent
	}

	if (addr >= 0x4280 && addr < 0x4800)
	{
		// the built-in OAM routines: the code is generated when the CPU reads the entry point
		if (addr == 0x4280)
			generate_oam_slow_update();
		else if (addr == 0x4282)
			generate_oam_ext_update();

		if (addr >= 0x4286)
			m_oam_locked = false;

		const u32 index = addr - 0x4280;
		return (index < sizeof(m_oam_code)) ? m_oam_code[index] : 0;
	}

	return get_open_bus();
}

void nes_rainbow_device::write_l(offs_t offset, u8 data)
{
	const u16 addr = 0x4100 + offset;

	if (addr >= 0x4800 && addr < 0x5000)
	{
		m_fpga[0x1800 + (addr - 0x4800)] = data;
		return;
	}
	if (addr >= 0x5000)
	{
		m_fpga[(m_fpga_bank * 0x1000) + (addr - 0x5000)] = data;
		return;
	}

	if (addr >= 0x4106 && addr <= 0x4107)
	{
		m_lo[addr - 0x4106] = (m_lo[addr - 0x4106] & 0x00ff) | (data << 8);
		update_state();
	}
	else if (addr >= 0x4116 && addr <= 0x4117)
	{
		m_lo[addr - 0x4116] = (m_lo[addr - 0x4116] & 0xff00) | data;
		update_state();
	}
	else if (addr >= 0x4108 && addr <= 0x410f)
	{
		m_hi[addr - 0x4108] = (m_hi[addr - 0x4108] & 0x00ff) | (data << 8);
		update_state();
	}
	else if (addr >= 0x4118 && addr <= 0x411f)
	{
		m_hi[addr - 0x4118] = (m_hi[addr - 0x4118] & 0xff00) | data;
		update_state();
	}
	else if (addr >= 0x4130 && addr <= 0x413f)
	{
		m_chr_reg[addr - 0x4130] = (m_chr_reg[addr - 0x4130] & 0x00ff) | ((data & 0x1f) << 8);
		update_state();
	}
	else if (addr >= 0x4140 && addr <= 0x414f)
	{
		m_chr_reg[addr - 0x4140] = (m_chr_reg[addr - 0x4140] & 0xff00) | data;
		update_state();
	}
	else if (addr >= 0x4126 && addr <= 0x4129)
		m_nt_bank[addr - 0x4126] = data;
	else if (addr >= 0x412a && addr <= 0x412d)
		m_nt_ctl[addr - 0x412a] = data;
	else if (addr >= 0x41a0 && addr <= 0x41a8)
	{
		// pulse 1, pulse 2 and the sawtooth: the VRC6's registers $9000-$9002, $A000-$A002 and $B000-$B002
		const int channel = (addr - 0x41a0) / 3;
		m_vrc6snd->write(((addr - 0x41a0) % 3) | (channel << 8), data);
	}
	else if (addr >= 0x4200 && addr <= 0x423f)
		m_sprite_ext_data[addr - 0x4200] = data;
	else
	{
		switch (addr)
		{
			case 0x4100:
				m_hi_mode = data & 0x07;
				m_lo_mode = BIT(data, 7);
				update_state();
				break;
			case 0x4115:
				m_fpga_bank = data & 0x01;
				break;
			case 0x4120:
				m_chr_mode = data & 0x07;
				m_window_en = BIT(data, 4);
				m_sprite_ext = BIT(data, 5);
				m_chr_select = data >> 6;
				update_state();
				break;
			case 0x4121:
				m_bgext_offset = data & 0x1f;
				break;
			case 0x4124:
				m_fill_tile = data;
				break;
			case 0x4125:
				m_fill_attr = data & 0x03;
				break;
			case 0x412e:
				m_win_bank = data;
				break;
			case 0x412f:
				m_win_ctl = data;
				break;

			case 0x4150:
				m_sl_line = data;
				break;
			case 0x4151:
				m_sl_en = true;
				update_irq();
				break;
			case 0x4152:
				m_sl_en = false;
				m_sl_pend = false;
				update_irq();
				break;
			case 0x4153:
				m_sl_offset = std::clamp<u8>(data, 1, 170);
				break;

			case 0x4157:
				m_parity = 0;
				break;
			case 0x4158:
				m_cpu_irq_latch = (m_cpu_irq_latch & 0x00ff) | (data << 8);
				break;
			case 0x4159:
				m_cpu_irq_latch = (m_cpu_irq_latch & 0xff00) | data;
				break;
			case 0x415a:
				m_cpu_irq_en = BIT(data, 0);
				m_cpu_irq_en_after_ack = BIT(data, 1);
				m_cpu_irq_ack_4011 = BIT(data, 2);
				m_cpu_irq_pend = false;
				if (m_cpu_irq_en)
					m_cpu_irq_counter = m_cpu_irq_latch;
				update_irq();
				break;
			case 0x415b:
				ack_cpu_irq();
				break;

			case 0x415c:
				m_fpga_addr = (m_fpga_addr & 0x00ff) | ((data & 0x1f) << 8);
				break;
			case 0x415d:
				m_fpga_addr = (m_fpga_addr & 0xff00) | data;
				break;
			case 0x415e:
				m_fpga_inc = data;
				break;
			case 0x415f:
				m_fpga[m_fpga_addr] = data;
				m_fpga_addr = (m_fpga_addr + m_fpga_inc) & (FPGA_RAM_SIZE - 1);
				break;

			case 0x416b:
				m_nmi_vec_en = BIT(data, 0);
				m_irq_vec_en = BIT(data, 1);
				break;
			case 0x416c:
				m_nmi_vec = (m_nmi_vec & 0x00ff) | (data << 8);
				break;
			case 0x416d:
				m_nmi_vec = (m_nmi_vec & 0xff00) | data;
				break;
			case 0x416e:
				m_irq_vec = (m_irq_vec & 0x00ff) | (data << 8);
				break;
			case 0x416f:
				m_irq_vec = (m_irq_vec & 0xff00) | data;
				break;

			case 0x4170:
				m_win_x1 = data & 0x1f;
				break;
			case 0x4171:
				m_win_x2 = data & 0x1f;
				break;
			case 0x4172:
				m_win_y1 = data;
				break;
			case 0x4173:
				m_win_y2 = data;
				break;
			case 0x4174:
				m_win_scroll_x = data & 0x1f;
				break;
			case 0x4175:
				m_win_scroll_y = data;
				break;

			case 0x4190:
				m_esp_en = BIT(data, 0);
				m_wifi_irq_en = BIT(data, 1);
				break;
			case 0x4193:
				m_recv_dst = data & 0x07;
				break;
			case 0x4194:
				m_send_src = data & 0x07;
				break;

			case 0x41a9:
				m_audio_ctl = data & 0x07;
				break;
			case 0x41aa:
				m_audio_vol = data & 0x0f;
				break;

			case 0x4240:
				m_sprite_ext_bank = data & 0x07;
				break;
			case 0x4241:
				m_oam_slow_page = data & 0x07;
				break;
			case 0x4242:
				m_oam_ext_page = data & 0x07;
				break;
			case 0x4243:
				m_oam_limit = data & 0x3f;
				break;
		}
	}
}

u8 nes_rainbow_device::read_m(offs_t offset)
{
	const int slot = (offset >> 12) & 1;
	return read_mem(m_lo_type[slot], m_lo_off[slot] + (offset & 0xfff));
}

void nes_rainbow_device::write_m(offs_t offset, u8 data)
{
	const int slot = (offset >> 12) & 1;
	write_mem(m_lo_type[slot], m_lo_off[slot] + (offset & 0xfff), data);
}

u8 nes_rainbow_device::read_h(offs_t offset)
{
	if (offset >= 0x7ffa)
	{
		// reading the NMI vector means an NMI is being taken: the scanline detection starts over (as on the MMC5)
		if (offset <= 0x7ffb)
		{
			m_in_frame = false;
			m_last_ppu_addr = 0;
			m_scanline_counter = 0;
			m_sl_pend = false;
			update_irq();

			if (m_nmi_vec_en)
				return BIT(offset, 0) ? (m_nmi_vec >> 8) : (m_nmi_vec & 0xff);
		}
		else if (offset >= 0x7ffe && m_irq_vec_en)
			return BIT(offset, 0) ? (m_irq_vec >> 8) : (m_irq_vec & 0xff);
	}

	const int slot = offset >> 12;
	return read_mem(m_hi_type[slot], m_hi_off[slot] + (offset & 0xfff));
}

void nes_rainbow_device::write_h(offs_t offset, u8 data)
{
	const int slot = offset >> 12;
	write_mem(m_hi_type[slot], m_hi_off[slot] + (offset & 0xfff), data);       // PRG-ROM ignores the write (no self-flashing here)
}


//-------------------------------------------------
//  reset: the main registers get the values of the doc's power-up and reset table
//-------------------------------------------------

// the FPGA's own sprite evaluation for the line the PPU is fetching sprites for: the first eight sprites whose Y range
// covers the line, in OAM order; unused slots point at sprite 0 (Mesen2)
void nes_rainbow_device::sprite_eval()
{
	const int height = m_large_sprites ? 16 : 8;
	const int line = m_scanline_counter;
	std::fill(std::begin(m_oam_map), std::end(m_oam_map), 0);
	int found = 0;
	for (int i = 0; (i < 64) && (found < 8); i++)
	{
		const int y = m_oam_y[i];
		if ((line >= y) && (line < y + height))
			m_oam_map[found++] = i;
	}
}

void nes_rainbow_device::pcb_reset()
{
	// the FPGA watches the CPU bus: $2000 bit 5 (sprite size), $2003 (OAM address) and $2004 (OAM data, also written
	// by OAM DMA), as Mesen2's Rainbow does; only the exact addresses are decoded, as there
	std::fill(std::begin(m_oam_y), std::end(m_oam_y), 0);
	std::fill(std::begin(m_oam_map), std::end(m_oam_map), 0);
	m_oam_addr = 0;
	m_large_sprites = false;
	if (!m_bus_tap_on)
	{
		m_bus_tap = cpu_space().install_write_tap(0x2000, 0x2004, "rainbow_oam",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					switch (offset)
					{
						case 0x2000: m_large_sprites = BIT(data, 5); break;
						case 0x2003: m_oam_addr = data; break;
						case 0x2004:
							if (!(m_oam_addr & 3))
								m_oam_y[m_oam_addr >> 2] = data;
							m_oam_addr++;
							break;
					}
				}, &m_bus_tap);
		m_bus_tap_on = true;
	}
	m_hi_mode = 0;
	m_lo_mode = 0;
	std::fill(std::begin(m_hi), std::end(m_hi), 0);
	std::fill(std::begin(m_lo), std::end(m_lo), 0);
	std::fill(std::begin(m_chr_reg), std::end(m_chr_reg), 0);
	m_fpga_bank = 0;

	m_chr_mode = 0;
	m_chr_select = 0;
	m_window_en = false;
	m_sprite_ext = false;
	m_bgext_offset = 0;

	// horizontal mirroring from CIRAM
	m_nt_bank[0] = 0;
	m_nt_bank[1] = 0;
	m_nt_bank[2] = 1;
	m_nt_bank[3] = 1;
	std::fill(std::begin(m_nt_ctl), std::end(m_nt_ctl), 0);
	m_fill_tile = 0;
	m_fill_attr = 0;
	m_win_ctl = 0x80;
	m_win_bank = 0;
	m_win_x1 = m_win_x2 = m_win_y1 = m_win_y2 = 0;
	m_win_scroll_x = m_win_scroll_y = 0;
	m_in_window = false;

	m_oam_slow_page = 7;
	m_oam_ext_page = 6;
	m_oam_limit = 63;
	std::fill(std::begin(m_oam_code), std::end(m_oam_code), 0);
	m_oam_locked = false;
	std::fill(std::begin(m_sprite_ext_data), std::end(m_sprite_ext_data), 0);
	m_sprite_ext_bank = 0;

	m_sl_en = false;
	m_sl_pend = false;
	m_sl_line = 0;
	m_sl_offset = 135;
	m_last_ppu_addr = 0;
	m_scanline_counter = -1;
	m_ppu_idle = 0;
	m_nt_read_counter = 0;
	m_ppu_read_counter = 0;
	m_in_frame = false;
	m_in_hblank = false;
	m_jitter = 0;
	m_parity = 0;
	m_nt_fetch_counter = 0;
	m_last_sprite_slot = -1;
	m_override_tile_fetch = false;
	m_ext_data = 0;

	m_cpu_irq_counter = 0;
	m_cpu_irq_latch = 0;
	m_cpu_irq_en = false;
	m_cpu_irq_pend = false;
	m_cpu_irq_en_after_ack = false;
	m_cpu_irq_ack_4011 = false;
	m_irq_asserted = false;

	m_fpga_addr = 0;
	m_fpga_inc = 0;

	m_nmi_vec_en = false;
	m_irq_vec_en = false;
	m_nmi_vec = 0;
	m_irq_vec = 0;

	m_esp_en = false;
	m_wifi_irq_en = false;
	m_send_src = 0;
	m_recv_dst = 0;
	m_audio_ctl = 0;
	m_audio_vol = 0x0f;

	update_state();
	set_irq_line(CLEAR_LINE);
}
