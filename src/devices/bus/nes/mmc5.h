// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_MMC5_H
#define MAME_BUS_NES_MMC5_H

#pragma once

#include "nxrom.h"

#include "sound/mmc5.h"
#include "video/ppu2c0x.h"  // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE


// ======================> nes_exrom_device

class nes_exrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~nes_exrom_device();

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual uint8_t chr_r(offs_t offset) override;
	virtual uint8_t nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void set_mirror(int page, int src);
	void update_prg();

	void update_irq();
	void pcm_irq(int state);
	void scanline_reset();

	u32 chr_addr(int reg, offs_t offset) const;
	uint8_t chr_mem_r(uint32_t addr);
	bool ram_present(u8 bank) const;
	u8 ram_r(u8 bank, offs_t offset);
	void ram_w(u8 bank, offs_t offset, u8 data);
	bool ram_writable() const { return m_wram_protect_1 == 0x02 && m_wram_protect_2 == 0x01; }

	bool in_split();
	int split_y();
	bool rendering_fetch();
	bool ext_attr_active() { return m_exram_control == 1 && substitutions() && rendering_fetch(); }
	bool split_active() { return m_split_scr && substitutions() && !BIT(m_exram_control, 1) && rendering_fetch() && in_split(); }

	uint8_t      m_irq_count;   // $5203
	uint8_t      m_irq_status;  // bit 7 pending, bit 6 in frame
	bool         m_irq_enable;
	bool         m_pcm_irq;

	uint8_t      m_mult1, m_mult2;

	uint16_t m_vrom_bank[12];   // $5120-$512B, 10 bits each, stored as NESdev lays them out for the CHR mode at write time

	uint8_t m_floodtile;
	uint8_t m_floodattr;

	uint8_t m_prg_mode;     // $5100
	uint8_t m_chr_mode;     // $5101
	uint8_t m_wram_protect_1;   // $5102
	uint8_t m_wram_protect_2;   // $5103
	uint8_t m_exram_control;    // $5104
	uint8_t m_wram_base;    // $5113

	uint8_t m_last_chr;     // the CHR register set written last (PPUDATA reads with 8x16 sprites)
	uint8_t m_prg_regs[4];  // $5114-$5117
	bool m_slot_ram[4];     // CPU $8000/$A000/$C000/$E000: PRG-RAM selected by the register the PRG mode uses there
	uint8_t m_slot_ram_bank[4];

	uint8_t m_ex_byte;      // extended attribute byte latched at the last nametable fetch

	uint8_t m_high_chr;   // $5130

	uint8_t m_split_scr;  // $5200
	uint8_t m_split_rev;  // $5200
	uint8_t m_split_ctrl; // $5200
	uint8_t m_split_yst;  // $5201
	uint8_t m_split_bank; // $5202

	// MMC-5 contains 1K of internal ram
	uint8_t m_exram[0x400];

	// hardware scanline detection (NESdev MMC5, "Scanline Detection and Scanline IRQ")
	void ppu_read_snoop(u16 addr);
	u16 m_last_ppu_addr = 0xffff;   // 0xffff: undefined
	u8 m_nt_match = 0;
	u8 m_ppu_idle = 0;
	bool m_ppu_reading = false;
	u8 m_scan_count = 0;

	// the chip watches the PPU's /RD strobe (NESdev: three consecutive reads of the same nametable address), so the reads
	// come from the strobe and not from calls to nt_r()/chr_r(): that also covers the reads those never see, such as a
	// PPUDATA read of the palette range, and leaves out the debugger's
	virtual bool wants_ppu_strobes() override { return true; }
	virtual void ppu_rd(offs_t addr) override { ppu_read_snoop(addr & 0x3fff); }

	// the chip listens to the CPU's writes to the fully decoded PPUCTRL ($2000, bit 5: sprite size) and PPUMASK ($2001,
	// bits 3 and 4: the E bits), to reads of PPUSTATUS ($2002) and to writes to OAMDMA ($4014) (NESdev, MMC5 "NES internal
	// state monitoring"); its reset detection does not clear the PPUCTRL and PPUMASK copies
	u8 m_snoop_ctrl = 0;
	u8 m_snoop_mask = 0;
	memory_passthrough_handler m_bus_tap;
	memory_passthrough_handler m_status_tap;
	memory_passthrough_handler m_dma_tap;
	bool m_bus_tap_on = false;
	bool substitutions() const { return BIT(m_snoop_mask, 3, 2) != 0; }
	bool independent_banks() const { return BIT(m_snoop_ctrl, 5) && substitutions(); }
	// what a CHR access is: a sprite pattern fetch, a background pattern fetch, or a PPUDATA read (the PPU is not rendering)
	enum class chr_access { SPRITE, BACKGROUND, DATA };
	chr_access chr_access_kind();

	required_device<ppu2c0x_device> m_ppu;
	required_device<mmc5_sound_device> m_sound;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_EXROM, nes_exrom_device)

#endif // MAME_BUS_NES_MMC5_H
