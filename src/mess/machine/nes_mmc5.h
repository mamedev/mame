#ifndef __NES_MMC5_H
#define __NES_MMC5_H

#include "machine/nes_nxrom.h"


// ======================> nes_exrom_device

class nes_exrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_READ8_MEMBER(nt_r);
	virtual DECLARE_WRITE8_MEMBER(nt_w);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

protected:
	void set_mirror(int page, int src);
	void prgram_bank8_x(int start, int bank);
	void update_render_mode();
	void update_prg();
	void update_chr();

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

	UINT8 m_mmc5_last_chr_a;
	UINT8 m_last_chr;
	UINT8 m_prg_regs[4];

	UINT8 m_high_chr;   // $5130

	UINT8 m_split_scr;  // $5200
	UINT8 m_split_ctrl; // $5200
	UINT8 m_split_yst;  // $5201
	UINT8 m_split_bank; // $5202

	// MMC-5 contains 1K of internal ram
	UINT8 *m_exram;

	//  int m_nes_vram_sprite[8];
};



// device type definition
extern const device_type NES_EXROM;

#endif
