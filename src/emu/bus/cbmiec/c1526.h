// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1526/MPS-802/4023 Printer emulation

**********************************************************************/

#pragma once

#ifndef __C1526__
#define __C1526__

#include "emu.h"
#include "cbmiec.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/m6504.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1526_base_t

class c1526_base_t :  public device_t
{
public:
	// construction/destruction
	c1526_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


// ======================> c1526_t

class c1526_t :  public c1526_base_t,
				 public device_cbm_iec_interface
{
public:
	// construction/destruction
	c1526_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

protected:
	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state);
	void cbm_iec_data(int state);
	void cbm_iec_reset(int state);
};


// ======================> c4023_t

class c4023_t :  public c1526_base_t,
				 public device_ieee488_interface
{
public:
	// construction/destruction
	c4023_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

protected:
	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state);
	virtual void ieee488_ifc(int state);
};


// device type definition
extern const device_type C1526;
extern const device_type MPS802;
extern const device_type C4023;



#endif
