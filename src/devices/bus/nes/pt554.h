// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_PT554_H
#define __NES_PT554_H

#include "nxrom.h"


// ======================> nes_bandai_pt554_device

class nes_bandai_pt554_device : public nes_cnrom_device
{
public:
	// construction/destruction
	nes_bandai_pt554_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	required_device<samples_device> m_samples;
};




// device type definition
extern const device_type NES_BANDAI_PT554;

#endif
