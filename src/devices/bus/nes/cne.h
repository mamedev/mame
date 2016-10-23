// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_CNE_H
#define __NES_CNE_H

#include "nxrom.h"


// ======================> nes_cne_decathl_device

class nes_cne_decathl_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_decathl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_cne_fsb_device

class nes_cne_fsb_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_fsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_cne_shlz_device

class nes_cne_shlz_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_shlz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};





// device type definition
extern const device_type NES_CNE_DECATHL;
extern const device_type NES_CNE_FSB;
extern const device_type NES_CNE_SHLZ;

#endif
