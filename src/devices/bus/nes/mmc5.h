// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC5_H
#define MAME_BUS_NES_MMC5_H

#pragma once

#include "nxrom.h"


// ======================> nes_exrom_device

class nes_exrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_READ8_MEMBER(nt_r) override;
	virtual DECLARE_WRITE8_MEMBER(nt_w) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

	void set_mirror(int page, int src);
	void update_prg();

	inline uint8_t base_chr_r(int bank, uint32_t offset);
	inline uint8_t split_chr_r(uint32_t offset);
	inline uint8_t bg_ex1_chr_r(uint32_t offset);
	inline bool in_split();

	uint16_t     m_irq_count;
	uint8_t      m_irq_status;
	int        m_irq_enable;

	int        m_mult1, m_mult2;

	int m_mmc5_scanline;
	int m_vrom_page_a;
	int m_vrom_page_b;
	uint16_t m_vrom_bank[12];            // MMC5 has 10bit wide VROM regs!

	int m_floodtile;
	int m_floodattr;

	int m_prg_mode;     // $5100
	int m_chr_mode;     // $5101
	int m_wram_protect_1;   // $5102
	int m_wram_protect_2;   // $5103
	int m_exram_control;    // $5104
	int m_wram_base;    // $5113

	uint8_t m_last_chr;
	uint8_t m_ex1_chr;
	uint8_t m_split_chr;
	uint8_t m_prg_regs[4];
	uint8_t m_prg_ram_mapped[4];

	uint8_t m_ex1_bank;

	uint8_t m_high_chr;   // $5130

	uint8_t m_split_scr;  // $5200
	uint8_t m_split_rev;  // $5200
	uint8_t m_split_ctrl; // $5200
	uint8_t m_split_yst;  // $5201
	uint8_t m_split_bank; // $5202
	int m_vcount;

	// MMC-5 contains 1K of internal ram
	uint8_t m_exram[0x400];

	uint8_t m_ram_hi_banks[4];

	//  int m_nes_vram_sprite[8];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_EXROM, nes_exrom_device)

#endif // MAME_BUS_NES_MMC5_H
