// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM070 Local Interconnect option card emulation

**********************************************************************/

#ifndef MAME_BUS_WANGPC_LIC_H
#define MAME_BUS_WANGPC_LIC_H

#pragma once

#include "wangpc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_lic_device

class wangpc_lic_device : public device_t,
								public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_lic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_LIC, wangpc_lic_device)

#endif // MAME_BUS_WANGPC_LIC_H
