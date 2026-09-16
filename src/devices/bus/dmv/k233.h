// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
#ifndef MAME_BUS_DMV_K233_H
#define MAME_BUS_DMV_K233_H

#pragma once

#include "dmvbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================>

class dmv_k233_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k233_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// dmvcart_interface overrides
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;
	virtual bool read(offs_t offset, uint8_t &data) override;
	virtual bool write(offs_t offset, uint8_t data) override;

private:
	bool        m_enabled;
	uint8_t *     m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K233, dmv_k233_device)

#endif // MAME_BUS_DMV_K233_H
