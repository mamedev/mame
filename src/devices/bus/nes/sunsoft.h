// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_SUNSOFT_H
#define MAME_BUS_NES_SUNSOFT_H

#pragma once

#include "nxrom.h"
#include "sound/ay8910.h"


// ======================> nes_sunsoft_1_device

class nes_sunsoft_1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sunsoft_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sunsoft_2_device

class nes_sunsoft_2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sunsoft_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sunsoft_3_device

class nes_sunsoft_3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sunsoft_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	uint16_t m_irq_count;
	int m_irq_enable, m_irq_toggle;

	emu_timer *irq_timer;
};


// ======================> nes_sunsoft_4_device

class nes_sunsoft_4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sunsoft_4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	void sun4_write(offs_t offset, u8 data);
	virtual void write_h(offs_t offset, u8 data) override { sun4_write(offset, data); }

	virtual void pcb_reset() override;

protected:
	nes_sunsoft_4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 m_wram_enable;

private:
	void sun4_mirror();

	u8 m_reg[3];
};


// ======================> nes_sunsoft_fme7_device

class nes_sunsoft_fme7_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sunsoft_fme7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	void fme7_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { fme7_write(offset, data); }

	virtual void pcb_reset() override;

protected:
	nes_sunsoft_fme7_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	uint16_t m_irq_count;
	int m_irq_enable;

	emu_timer *irq_timer;

	uint8_t m_latch;
	uint8_t m_wram_bank;
};


// ======================> nes_sunsoft_5_device

class nes_sunsoft_5_device : public nes_sunsoft_fme7_device
{
public:
	// construction/destruction
	nes_sunsoft_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ay8910_device> m_ym2149;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_SUNSOFT_1,    nes_sunsoft_1_device)
DECLARE_DEVICE_TYPE(NES_SUNSOFT_2,    nes_sunsoft_2_device)
DECLARE_DEVICE_TYPE(NES_SUNSOFT_3,    nes_sunsoft_3_device)
DECLARE_DEVICE_TYPE(NES_SUNSOFT_4,    nes_sunsoft_4_device)
DECLARE_DEVICE_TYPE(NES_SUNSOFT_FME7, nes_sunsoft_fme7_device)
DECLARE_DEVICE_TYPE(NES_SUNSOFT_5,    nes_sunsoft_5_device)

#endif // MAME_BUS_NES_SUNSOFT_H
