// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 890 bus expander emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC890__
#define __ABC890__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MAX_SLOTS 8



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc890_device

class abc890_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc890_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	abc890_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);
	virtual UINT8 abcbus_inp();
	virtual void abcbus_out(UINT8 data);
	virtual UINT8 abcbus_stat();
	virtual void abcbus_c1(UINT8 data);
	virtual void abcbus_c2(UINT8 data);
	virtual void abcbus_c3(UINT8 data);
	virtual void abcbus_c4(UINT8 data);
	virtual UINT8 abcbus_xmemfl(offs_t offset);
	virtual void abcbus_xmemw(offs_t offset, UINT8 data);
};


// ======================> abc894_device

class abc894_device :  public abc890_device
{
public:
	// construction/destruction
	abc894_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};


// ======================> abc850_device

class abc850_device :  public abc890_device
{
public:
	// construction/destruction
	abc850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};


// ======================> abc852_device

class abc852_device :  public abc890_device
{
public:
	// construction/destruction
	abc852_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};


// ======================> abc856_device

class abc856_device :  public abc890_device
{
public:
	// construction/destruction
	abc856_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};


// device type definition
extern const device_type ABC890;
extern const device_type ABC894;
extern const device_type ABC850;
extern const device_type ABC852;
extern const device_type ABC856;



#endif
