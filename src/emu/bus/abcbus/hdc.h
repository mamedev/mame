// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor XEBEC Winchester controller card emulation

*********************************************************************/

#pragma once

#ifndef __ABC_HDC__
#define __ABC_HDC__

#include "emu.h"
#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "bus/scsi/scsihd.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_hdc_device

class abc_hdc_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ABC_HDC;



#endif
