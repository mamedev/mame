// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
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
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void set_mirror(int page, int src);
	void update_prg();

	void update_irq();
	void pcm_irq(int state);

	inline uint8_t base_chr_r(int bank, uint32_t offset);
	inline uint8_t split_chr_r(uint32_t offset);
	inline uint8_t bg_ex1_chr_r(uint32_t offset);
	inline bool in_split();

	uint16_t     m_irq_count;
	uint8_t      m_irq_status;
	bool         m_irq_enable;
	bool         m_pcm_irq;

	int32_t      m_mult1, m_mult2;

	int32_t m_mmc5_scanline;
	int32_t m_vrom_page_a;
	int32_t m_vrom_page_b;
	uint16_t m_vrom_bank[12];            // MMC5 has 10bit wide VROM regs!

	uint8_t m_floodtile;
	uint8_t m_floodattr;

	uint8_t m_prg_mode;     // $5100
	uint8_t m_chr_mode;     // $5101
	uint8_t m_wram_protect_1;   // $5102
	uint8_t m_wram_protect_2;   // $5103
	uint8_t m_exram_control;    // $5104
	uint8_t m_wram_base;    // $5113

	uint8_t m_last_chr;
	uint8_t m_ex1_chr;
	uint8_t m_split_chr;
	uint8_t m_prg_regs[4];
	uint8_t m_prg_ram_mapped[4];

	uint8_t m_ex1_bank;
	uint8_t m_ex1_attrib;

	uint8_t m_high_chr;   // $5130

	uint8_t m_split_scr;  // $5200
	uint8_t m_split_rev;  // $5200
	uint8_t m_split_ctrl; // $5200
	uint8_t m_split_yst;  // $5201
	uint8_t m_split_bank; // $5202
	int32_t m_vcount;

	// MMC-5 contains 1K of internal ram
	uint8_t m_exram[0x400];

	uint8_t m_ram_hi_banks[4];

	//  int m_nes_vram_sprite[8];

	required_device<ppu2c0x_device> m_ppu;
	required_device<mmc5_sound_device> m_sound;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_EXROM, nes_exrom_device)

#endif // MAME_BUS_NES_MMC5_H
