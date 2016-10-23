// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_RCM_H
#define __NES_RCM_H

#include "nxrom.h"


// ======================> nes_gs2015_device

class nes_gs2015_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return read_m(space, offset, mem_mask); }
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gs2004_device

class nes_gs2004_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gs2013_device

class nes_gs2013_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_tf9_device

class nes_tf9_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tf9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_3dblock_device

class nes_3dblock_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_3dblock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	uint8_t m_reg[4];
	uint8_t m_irq_count;
};




// device type definition
extern const device_type NES_GS2015;
extern const device_type NES_GS2004;
extern const device_type NES_GS2013;
extern const device_type NES_TF9IN1;
extern const device_type NES_3DBLOCK;

#endif
