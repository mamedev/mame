// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_MMC5_H
#define __NES_MMC5_H

#include "nxrom.h"


// ======================> nes_exrom_device

class nes_exrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_exrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
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
	void set_mirror(int page, int src);
	void update_prg();

	inline UINT8 base_chr_r(int bank, UINT32 offset);
	inline UINT8 split_chr_r(UINT32 offset);
	inline UINT8 bg_ex1_chr_r(UINT32 offset);
	inline bool in_split();

	UINT16     m_irq_count;
	UINT8      m_irq_status;
	int        m_irq_enable;

	int        m_mult1, m_mult2;

	int m_mmc5_scanline;
	int m_vrom_page_a;
	int m_vrom_page_b;
	UINT16 m_vrom_bank[12];            // MMC5 has 10bit wide VROM regs!

	int m_floodtile;
	int m_floodattr;

	int m_prg_mode;     // $5100
	int m_chr_mode;     // $5101
	int m_wram_protect_1;   // $5102
	int m_wram_protect_2;   // $5103
	int m_exram_control;    // $5104
	int m_wram_base;    // $5113

	UINT8 m_last_chr;
	UINT8 m_ex1_chr;
	UINT8 m_split_chr;
	UINT8 m_prg_regs[4];
	UINT8 m_prg_ram_mapped[4];

	UINT8 m_ex1_bank;

	UINT8 m_high_chr;   // $5130

	UINT8 m_split_scr;  // $5200
	UINT8 m_split_rev;  // $5200
	UINT8 m_split_ctrl; // $5200
	UINT8 m_split_yst;  // $5201
	UINT8 m_split_bank; // $5202
	int m_vcount;

	// MMC-5 contains 1K of internal ram
	UINT8 m_exram[0x400];

	UINT8 m_ram_hi_banks[4];

	//  int m_nes_vram_sprite[8];
};



// device type definition
extern const device_type NES_EXROM;

#endif
