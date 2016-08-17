// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __SSI2001__
#define __SSI2001__

#include "emu.h"
#include "isa.h"
#include "sound/mos6581.h"
#include "bus/pc_joy/pc_joy.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> ssi2001_device

class ssi2001_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	ssi2001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<pc_joy_device> m_joy;
	required_device<mos6581_device> m_sid;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition

extern const device_type ISA8_SSI2001;

#endif
