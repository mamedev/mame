// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_BANDAI_H
#define MAME_BUS_NES_BANDAI_H

#pragma once

#include "nxrom.h"
#include "machine/i2cmem.h"


// ======================> nes_oekakids_device

class nes_oekakids_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_oekakids_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

	virtual void ppu_latch(offs_t offset) override;

	// TODO: add oeka kids controller emulation

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void update_chr();

	uint8_t m_reg, m_latch;
};


// ======================> nes_fcg_device

class nes_fcg_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fcg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void fcg_write(offs_t offset, uint8_t data);
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_fcg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	uint16_t   m_irq_count;
	int        m_irq_enable;

	emu_timer *irq_timer;
};


// ======================> nes_lz93d50_device

class nes_lz93d50_device : public nes_fcg_device
{
public:
	// construction/destruction
	nes_lz93d50_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override { fcg_write(offset, data); }

protected:
	nes_lz93d50_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_lz93d50_24c01_device

class nes_lz93d50_24c01_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_lz93d50_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// TODO: fix EEPROM I/O emulation
	required_device<i2cmem_device> m_i2cmem;
	uint8_t m_i2c_dir;
};


// ======================> nes_lz93d50_24c02_device

class nes_lz93d50_24c02_device : public nes_lz93d50_24c01_device
{
public:
	// construction/destruction
	nes_lz93d50_24c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_fjump2_device

class nes_fjump2_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_fjump2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void set_prg();
	uint8_t m_reg[5];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_OEKAKIDS,      nes_oekakids_device)
DECLARE_DEVICE_TYPE(NES_FCG,           nes_fcg_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50,       nes_lz93d50_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50_24C01, nes_lz93d50_24c01_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50_24C02, nes_lz93d50_24c02_device)
DECLARE_DEVICE_TYPE(NES_FJUMP2,        nes_fjump2_device)

#endif // MAME_BUS_NES_BANDAI_H
