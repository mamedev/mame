// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_CAMERICA_H
#define __NES_CAMERICA_H

#include "nxrom.h"


// ======================> nes_bf9093_device

class nes_bf9093_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bf9093_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bf9096_device

class nes_bf9096_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bf9096_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { write_h(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

private:
	uint8_t m_bank_base, m_latch;
};


// ======================> nes_golden5_device

class nes_golden5_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_golden5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_bank_base, m_latch;
};





// device type definition
extern const device_type NES_BF9093;
extern const device_type NES_BF9096;
extern const device_type NES_GOLDEN5;

#endif
