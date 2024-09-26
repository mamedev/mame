// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_KONAMI_H
#define MAME_BUS_NES_KONAMI_H

#pragma once

#include "nxrom.h"
#include "sound/vrc6.h"
#include "sound/ymopl.h"


// ======================> nes_konami_vrc1_device

class nes_konami_vrc1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_mmc_vrom_bank[2];
};


// ======================> nes_konami_vrc2_device

class nes_konami_vrc2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_mmc_vrom_bank[8];
	u8 m_latch;
};


// ======================> nes_konami_vrc3_device

class nes_konami_vrc3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	u16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;

	emu_timer *irq_timer;
};


// ======================> nes_konami_vrc4_device

class nes_konami_vrc4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	void set_mirror(u8 data);
	void set_prg(int prg_base, int prg_mask);
	void set_chr(int chr_base, int chr_mask);
	virtual void set_prg() { set_prg(0x00, 0x1f); }
	virtual void set_chr() { set_chr(0x00, 0x1ff); }

	u16 m_mmc_vrom_bank[8];
	u8 m_mmc_prg_bank[2];
	u8 m_prg_flip;
	u8 m_wram_enable;

	void irq_tick();
	virtual void irq_ack_w();
	void irq_ctrl_w(u8 data);
	u8 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;
	int m_irq_prescale;

	emu_timer *irq_timer;
};


// ======================> nes_konami_vrc6_device

class nes_konami_vrc6_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<vrc6snd_device> m_vrc6snd;
};


// ======================> nes_konami_vrc7_device

class nes_konami_vrc7_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ds1001_device> m_vrc7snd;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_VRC1, nes_konami_vrc1_device)
DECLARE_DEVICE_TYPE(NES_VRC2, nes_konami_vrc2_device)
DECLARE_DEVICE_TYPE(NES_VRC3, nes_konami_vrc3_device)
DECLARE_DEVICE_TYPE(NES_VRC4, nes_konami_vrc4_device)
DECLARE_DEVICE_TYPE(NES_VRC6, nes_konami_vrc6_device)
DECLARE_DEVICE_TYPE(NES_VRC7, nes_konami_vrc7_device)

#endif // MAME_BUS_NES_KONAMI_H
