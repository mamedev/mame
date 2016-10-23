// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_IREM_H
#define __NES_IREM_H

#include "nxrom.h"


// ======================> nes_lrog017_device

class nes_lrog017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lrog017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_holydivr_device

class nes_holydivr_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_holydivr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_tam_s1_device

class nes_tam_s1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tam_s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_g101_device

class nes_g101_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_g101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	uint8_t     m_latch;
};


// ======================> nes_h3001_device

class nes_h3001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_h3001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	uint16_t     m_irq_count, m_irq_count_latch;
	int        m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};



// device type definition
extern const device_type NES_LROG017;
extern const device_type NES_HOLYDIVR;
extern const device_type NES_TAM_S1;
extern const device_type NES_G101;
extern const device_type NES_H3001;

#endif
