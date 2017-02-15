// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K233_H__
#define __DMV_K233_H__

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
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void io_write(address_space &space, int ifsel, offs_t offset, uint8_t data) override;
	virtual bool read(offs_t offset, uint8_t &data) override;
	virtual bool write(offs_t offset, uint8_t data) override;

private:
	bool        m_enabled;
	uint8_t *     m_ram;
};


// device type definition
extern const device_type DMV_K233;

#endif  /* __DMV_K233_H__ */
