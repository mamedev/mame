// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor X37 SASI hard disk controller emulation

*********************************************************************/

#ifndef MAME_LUXOR_X37_SASI_H
#define MAME_LUXOR_X37_SASI_H

#pragma once


#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_x37_sasi_device

class luxor_x37_sasi_device :  public device_t, public nscsi_device_interface
{
public:
	// construction/destruction
	luxor_x37_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_write_int.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// nscsi_device_interface overrides
	virtual void scsi_ctrl_changed() override;

private:
	devcb_write_line m_write_int;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_X37_SASI, luxor_x37_sasi_device)


#endif // MAME_LUXOR_X37_SASI_H
