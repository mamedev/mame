// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NTDEC_H
#define __NES_NTDEC_H

#include "nxrom.h"


// ======================> nes_ntdec_asder_device

class nes_ntdec_asder_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_asder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
};


// ======================> nes_ntdec_fh_device

class nes_ntdec_fh_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_fh_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};





// device type definition
extern const device_type NES_NTDEC_ASDER;
extern const device_type NES_NTDEC_FH;

#endif
