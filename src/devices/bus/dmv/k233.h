// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K233_H__
#define __DMV_K233_H__

#include "emu.h"
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
	dmv_k233_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void io_write(address_space &space, int ifsel, offs_t offset, UINT8 data) override;
	virtual bool read(offs_t offset, UINT8 &data) override;
	virtual bool write(offs_t offset, UINT8 data) override;

private:
	bool        m_enabled;
	UINT8 *     m_ram;
};


// device type definition
extern const device_type DMV_K233;

#endif  /* __DMV_K233_H__ */
