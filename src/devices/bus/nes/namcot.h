// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NAMCOT_H
#define __NES_NAMCOT_H

#include "nxrom.h"


// ======================> nes_namcot3433_device

class nes_namcot3433_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3433_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_namcot3433_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void dxrom_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { dxrom_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
};


// ======================> nes_namcot3446_device

class nes_namcot3446_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
};


// ======================> nes_namcot3425_device

class nes_namcot3425_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3425_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
	uint8_t m_reg[4];
};


// ======================> nes_namcot340_device

class nes_namcot340_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot340_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_namcot340_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual uint8_t n340_loread(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void n340_lowrite(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void n340_hiwrite(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return n340_loread(space, offset, mem_mask); }
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { n340_lowrite(space, offset, data, mem_mask); }
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { n340_hiwrite(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	// Namcot-163 chip contains 8K of internal ram, possibly battery backed up (not emulated yet)
	// was it also present in 175 & 340 chips?
	uint8_t m_n163_ram[0x2000];
};


// ======================> nes_namcot175_device

class nes_namcot175_device : public nes_namcot340_device
{
public:
	// construction/destruction
	nes_namcot175_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_wram_protect;
};


// ======================> nes_namcot163_device

class nes_namcot163_device : public nes_namcot340_device
{
public:
	// construction/destruction
	nes_namcot163_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	// we have to overwrite these to allow CIRAM to be used for VRAM, even if it's not clear which game(s) use this
	virtual uint8_t chr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	void set_mirror(uint8_t page, uint8_t data);

	uint8_t m_wram_protect, m_latch, m_chr_bank;
	// TODO: add emulation of the sound part of the chip
};



// device type definition
extern const device_type NES_NAMCOT3433;
extern const device_type NES_NAMCOT3446;
extern const device_type NES_NAMCOT3425;
extern const device_type NES_NAMCOT175;
extern const device_type NES_NAMCOT340;
extern const device_type NES_NAMCOT163;

#endif
