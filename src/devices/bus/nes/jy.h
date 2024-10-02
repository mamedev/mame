// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_JY_H
#define MAME_BUS_NES_JY_H

#pragma once

#include "nxrom.h"


// ======================> nes_jy_typea_device

class nes_jy_typea_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual uint8_t chr_r(offs_t offset) override;
	virtual uint8_t nt_r(offs_t offset) override;

	virtual void scanline_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	void irq_clock(bool blanked, int mode);
	void update_banks(int reg);
	void update_prg();
	void update_chr();
	void update_mirror_typea();
	virtual void update_mirror() { update_mirror_typea(); }
	inline uint8_t unscramble(uint8_t bank);

	uint8_t m_mul[2];
	uint8_t m_latch;
	uint8_t m_reg[4];
	uint8_t m_chr_latch[2];   // type C uses a more complex CHR 4K mode, and these vars are only changed for those games
	uint8_t m_mmc_prg_bank[4];
	uint16_t m_mmc_nt_bank[4];
	uint16_t m_mmc_vrom_bank[8];
	uint16_t m_extra_chr_bank;
	uint16_t m_extra_chr_mask;
	int m_bank_6000;

	uint8_t m_irq_mode;
	uint8_t m_irq_count;
	uint8_t m_irq_prescale;
	uint8_t m_irq_prescale_mask;
	uint8_t m_irq_flip;
	int m_irq_enable;
	int m_irq_up, m_irq_down;

	emu_timer *irq_timer;
	attotime timer_freq;
};


// ======================> nes_jy_typeb_device

class nes_jy_typeb_device : public nes_jy_typea_device
{
public:
	// construction/destruction
	nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void update_mirror_typeb();
	virtual void update_mirror() override { update_mirror_typeb(); }
};

// ======================> nes_jy_typec_device

class nes_jy_typec_device : public nes_jy_typeb_device
{
public:
	// construction/destruction
	nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t chr_r(offs_t offset) override;

protected:
	void update_mirror_typec();
	virtual void update_mirror() override { update_mirror_typec(); }
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JY_TYPEA, nes_jy_typea_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEB, nes_jy_typeb_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEC, nes_jy_typec_device)

#endif // MAME_BUS_NES_JY_H
