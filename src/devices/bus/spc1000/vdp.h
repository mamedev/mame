// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SPC1000_VDP_H__
#define __SPC1000_VDP_H__

#include "exp.h"
#include "video/tms9928a.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spc1000_vdp_exp_device

class spc1000_vdp_exp_device : public device_t,
						public device_spc1000_card_interface
{
public:
	// construction/destruction
	spc1000_vdp_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

public:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void vdp_interrupt(int state);

private:
	// internal state
	required_device<tms9928a_device>   m_vdp;
};


// device type definition
extern const device_type SPC1000_VDP_EXP;

#endif  /* __SPC1000_VDP_H__ */
