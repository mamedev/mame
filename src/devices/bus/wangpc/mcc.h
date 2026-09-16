// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM043 Multiport Communications Controller emulation

**********************************************************************/

#ifndef MAME_BUS_WANGPC_MCC_H
#define MAME_BUS_WANGPC_MCC_H

#pragma once

#include "wangpc.h"
#include "machine/z80sio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_mcc_device

class wangpc_mcc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_mcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;

private:
	inline void set_irq(int state);

	required_device<z80sio_device> m_sio;
	required_device<z80dart_device> m_dart;

	uint8_t m_option;
	int m_irq;
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_MCC, wangpc_mcc_device)

#endif // MAME_BUS_WANGPC_MCC_H
