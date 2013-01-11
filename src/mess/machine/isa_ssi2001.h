#ifndef __SSI2001__
#define __SSI2001__

#include "emu.h"
#include "machine/isa.h"
#include "sound/sid6581.h"
#include "machine/pc_joy.h"

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
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<pc_joy_device> m_joy;
	required_device<sid6581_device> m_sid;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
};

// device type definition

extern const device_type ISA8_SSI2001;

#endif
