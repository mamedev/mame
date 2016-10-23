// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_LEGACY_H
#define __NES_LEGACY_H

#include "nxrom.h"


// ======================> nes_ffe3_device

class nes_ffe3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ffe4_device

class nes_ffe4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe4_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_ffe4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t chr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	uint8_t m_latch;
	int m_exram_enabled;
	int m_exram_bank;
	uint8_t m_exram[0x8000];
};


// ======================> nes_ffe8_device

class nes_ffe8_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override {}

	virtual void pcb_reset() override;
};



// device type definition
extern const device_type NES_FFE3;
extern const device_type NES_FFE4;
extern const device_type NES_FFE8;


#endif
