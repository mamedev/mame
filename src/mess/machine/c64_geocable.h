/**********************************************************************

    geoCable Centronics Cable emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_GEOCABLE__
#define __C64_GEOCABLE__


#include "emu.h"
#include "machine/c64user.h"
#include "machine/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_geocable_device

class c64_geocable_device : public device_t,
							public device_c64_user_port_interface
{
public:
	// construction/destruction
	c64_geocable_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( busy_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_geocable"; }
	virtual void device_start();

	// device_c64_user_port_interface overrides
	virtual void c64_pb_w(address_space &space, offs_t offset, UINT8 data);
	virtual void c64_pa2_w(int level);

private:
	required_device<centronics_device> m_centronics;
};


// device type definition
extern const device_type C64_GEOCABLE;


#endif
