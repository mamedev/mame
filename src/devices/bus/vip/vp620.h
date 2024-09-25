// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP ASCII Keyboard Interface VP-620 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP620_H
#define MAME_BUS_VIP_VP620_H

#pragma once

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
	vp620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vip_byteio_port_interface overrides
	virtual uint8_t vip_in_r() override;
	virtual int vip_ef4_r() override;

private:
	void kb_w(uint8_t data);

	uint8_t m_keydata;
	int m_keystb;
};


// device type definition
DECLARE_DEVICE_TYPE(VP620, vp620_device)

#endif // MAME_BUS_VIP_VP620_H
