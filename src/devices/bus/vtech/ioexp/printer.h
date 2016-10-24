// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Printer Interface

    Dick Smith Electronics X-7320

***************************************************************************/

#pragma once

#ifndef __VTECH_IOEXP_PRINTER_H__
#define __VTECH_IOEXP_PRINTER_H__

#include "emu.h"
#include "ioexp.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> printer_interface_device

class printer_interface_device : public device_t, public device_ioexp_interface
{
public:
	// construction/destruction
	printer_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void busy_w(int state);
	uint8_t busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void strobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;

	int m_centronics_busy;
};

// device type definition
extern const device_type PRINTER_INTERFACE;

#endif // __VTECH_IOEXP_PRINTER_H__
