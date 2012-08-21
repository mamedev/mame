/**********************************************************************

    RCA VIP ASCII Keyboard Interface VP-620 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VP620__
#define __VP620__

#include "emu.h"
#include "machine/keyboard.h"
#include "machine/vip_byteio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp620_device

class vp620_device : public device_t,
					 public device_vip_byteio_port_interface
{
public:
	// construction/destruction
	vp620_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE8_MEMBER( kb_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "vp620"; }
	virtual void device_start();

	// device_vip_byteio_port_interface overrides
	virtual UINT8 vip_in_r();
	virtual int vip_ef3_r();

private:
	UINT8 m_keydata;
	int m_keystb;
};


// device type definition
extern const device_type VP620;


#endif
