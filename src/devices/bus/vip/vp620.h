// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP ASCII Keyboard Interface VP-620 emulation

**********************************************************************/

#pragma once

#ifndef __VP620__
#define __VP620__

#include "emu.h"
#include "byteio.h"
#include "machine/keyboard.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp620_device

class vp620_device : public device_t,
						public device_vip_byteio_port_interface
{
public:
	// construction/destruction
	vp620_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	DECLARE_WRITE8_MEMBER( kb_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vip_byteio_port_interface overrides
	virtual UINT8 vip_in_r() override;
	virtual int vip_ef4_r() override;

private:
	UINT8 m_keydata;
	int m_keystb;
};


// device type definition
extern const device_type VP620;


#endif
