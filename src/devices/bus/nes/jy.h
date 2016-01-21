// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_JY_H
#define __NES_JY_H

#include "nxrom.h"


// ======================> nes_jy_typea_device

class nes_jy_typea_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_READ8_MEMBER(nt_r) override;

	virtual void scanline_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	void irq_clock(int blanked, int mode);
	void update_banks(int reg);
	void update_prg();
	void update_chr();
	void update_mirror_typea();
	virtual void update_mirror() { update_mirror_typea(); }
	inline UINT8 unscramble(UINT8 bank);

	UINT8 m_mul[2];
	UINT8 m_latch;
	UINT8 m_reg[4];
	UINT8 m_chr_latch[2];   // type C uses a more complex CHR 4K mode, and these vars are only changed for those games
	UINT8 m_mmc_prg_bank[4];
	UINT16 m_mmc_nt_bank[4];
	UINT16 m_mmc_vrom_bank[8];
	UINT16 m_extra_chr_bank;
	UINT16 m_extra_chr_mask;
	int m_bank_6000;

	UINT8 m_irq_mode;
	UINT8 m_irq_count;
	UINT8 m_irq_prescale;
	UINT8 m_irq_prescale_mask;
	UINT8 m_irq_flip;
	int m_irq_enable;
	int m_irq_up, m_irq_down;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_jy_typeb_device

class nes_jy_typeb_device : public nes_jy_typea_device
{
public:
	// construction/destruction
	nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void update_mirror_typeb();
	virtual void update_mirror() override { update_mirror_typeb(); }
};

// ======================> nes_jy_typec_device

class nes_jy_typec_device : public nes_jy_typeb_device
{
public:
	// construction/destruction
	nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(chr_r) override;

protected:
	void update_mirror_typec();
	virtual void update_mirror() override { update_mirror_typec(); }
};





// device type definition
extern const device_type NES_JY_TYPEA;
extern const device_type NES_JY_TYPEB;
extern const device_type NES_JY_TYPEC;

#endif
