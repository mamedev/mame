// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

**********************************************************************/

#pragma once

#ifndef _MB8421_H
#define _MB8421_H

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// note: INT pins are only available on MB84x1
// INTL is for the CPU on the left side, INTR for the one on the right
#define MCFG_MB8421_INTL_HANDLER(_devcb) \
	devcb = &mb8421_device::set_intl_handler(*device, DEVCB_##_devcb);

#define MCFG_MB8421_INTR_HANDLER(_devcb) \
	devcb = &mb8421_device::set_intr_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb8421_device

class mb8421_device : public device_t
{
public:
	mb8421_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_intl_handler(device_t &device, _Object object) { return downcast<mb8421_device &>(device).m_intl_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_intr_handler(device_t &device, _Object object) { return downcast<mb8421_device &>(device).m_intr_handler.set_callback(object); }

	DECLARE_READ_LINE_MEMBER( busy_r ) { return 0; } // _BUSY pin - not emulated
	UINT8 peek(offs_t offset) { return m_ram[offset & 0x7ff]; }

	DECLARE_WRITE8_MEMBER( left_w );
	DECLARE_READ8_MEMBER( left_r );
	DECLARE_WRITE8_MEMBER( right_w );
	DECLARE_READ8_MEMBER( right_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_ram[0x800];

	devcb_write_line m_intl_handler;
	devcb_write_line m_intr_handler;
};

// device type definition
extern const device_type MB8421;


#endif /* _MB8421_H */
