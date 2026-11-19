// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP VP-620 ASCII Keyboard Cable emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP620_H
#define MAME_BUS_VIP_VP620_H

#pragma once

#include "byteio.h"
#include "vp601.h"



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
	void data_w(uint8_t data) { m_data = data; }
	void st_w(int state) { m_slot->inst_w(state); }
	void kd_w(int state) { m_kd = bool(state); }

	required_device<vip_ascii_keyboard_port_device> m_kb;

	uint8_t m_data;
	bool m_kd;
};


// device type definition
DECLARE_DEVICE_TYPE(VP620, vp620_device)


#endif // MAME_BUS_VIP_VP620_H
