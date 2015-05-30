// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

*********************************************************************/

#pragma once

#ifndef __INTERPOD__
#define __INTERPOD__

#include "emu.h"
#include "cbmiec.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"
#include "machine/6850acia.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define INTERPOD_TAG            "interpod"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTERPOD_ADD() \
	MCFG_DEVICE_ADD(INTERPOD_TAG, INTERPOD, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> interpod_device

class interpod_device :  public device_t,
							public device_cbm_iec_interface
{
public:
	// construction/destruction
	interpod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<mos6532_t> m_riot;
	required_device<acia6850_device> m_acia;
	required_device<ieee488_device> m_ieee;
};


// device type definition
extern const device_type INTERPOD;



#endif
