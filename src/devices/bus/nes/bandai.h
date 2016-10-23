// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_BANDAI_H
#define __NES_BANDAI_H

#include "nxrom.h"
#include "machine/i2cmem.h"


// ======================> nes_oekakids_device

class nes_oekakids_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_oekakids_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t nt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void nt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

	virtual void ppu_latch(offs_t offset) override;

	// TODO: add oeka kids controller emulation
protected:
	void update_chr();
	uint8_t m_reg, m_latch;
};


// ======================> nes_fcg_device

class nes_fcg_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fcg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_fcg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void fcg_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	uint16_t     m_irq_count;
	int        m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_lz93d50_device

class nes_lz93d50_device : public nes_fcg_device
{
public:
	// construction/destruction
	nes_lz93d50_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_lz93d50_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { fcg_write(space, offset, data, mem_mask); }
};


// ======================> nes_lz93d50_24c01_device

class nes_lz93d50_24c01_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_lz93d50_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

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

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> nes_fjump2_device

class nes_fjump2_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_fjump2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	void set_prg();
	uint8_t m_reg[5];
};


// device type definition
extern const device_type NES_OEKAKIDS;
extern const device_type NES_FCG;
extern const device_type NES_LZ93D50;
extern const device_type NES_LZ93D50_24C01;
extern const device_type NES_LZ93D50_24C02;
extern const device_type NES_FJUMP2;

#endif
