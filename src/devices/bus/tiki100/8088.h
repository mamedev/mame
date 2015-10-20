// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 8/16 8088/8087 expansion card emulation

**********************************************************************/

#pragma once

#ifndef __TIKI100_8088__
#define __TIKI100_8088__

#include "emu.h"
#include "bus/tiki100/exp.h"
#include "cpu/i86/i86.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_8088_t

class tiki100_8088_t : public device_t,
					   public device_tiki100bus_card_interface
{
public:
	// construction/destruction
	tiki100_8088_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_tiki100bus_card_interface overrides
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data);
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data);

private:
	required_device<i8088_cpu_device> m_maincpu;

	UINT8 m_data;
};


// device type definition
extern const device_type TIKI100_8088;


#endif
