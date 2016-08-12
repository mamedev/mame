// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

**********************************************************************/


#ifndef __ELECTRON_M2105__
#define __ELECTRON_M2105__

#include "emu.h"
#include "exp.h"
#include "machine/6522via.h"
#include "machine/mc68681.h"
#include "sound/tms5220.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_m2105_device:
	public device_t,
	public device_electron_expansion_interface

{
public:
	// construction/destruction
	electron_m2105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_exp_rom;
	required_device<via6522_device> m_via6522_0;
	required_device<via6522_device> m_via6522_1;
	required_device<tms5220_device> m_tms;
	required_device<centronics_device> m_centronics;
};


// device type definition
extern const device_type ELECTRON_M2105;


#endif /* __ELECTRON_M2105__ */
