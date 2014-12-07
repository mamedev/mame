// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2040/3040/4040 Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C2040__
#define __C2040__

#include "emu.h"
#include "ieee488.h"
#include "c2040fdc.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m6504.h"
#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/mos6530.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2040_device

class c2040_device :  public device_t,
						public device_ieee488_interface
{
public:
	// construction/destruction
	c2040_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	c2040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_WRITE8_MEMBER( via_pb_w );
	DECLARE_WRITE_LINE_MEMBER( mode_sel_w );
	DECLARE_WRITE_LINE_MEMBER( rw_sel_w );
	DECLARE_READ8_MEMBER( miot_pb_r );
	DECLARE_WRITE8_MEMBER( miot_pb_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state);
	virtual void ieee488_ifc(int state);

	enum
	{
		LED_POWER = 0,
		LED_ACT0,
		LED_ACT1,
		LED_ERR
	};

	inline void update_ieee_signals();

	required_device<m6502_device> m_maincpu;
	required_device<m6504_device> m_fdccpu;
	required_device<riot6532_device> m_riot0;
	required_device<riot6532_device> m_riot1;
	required_device<mos6530_device> m_miot;
	required_device<via6522_device> m_via;
	required_device<floppy_image_device> m_floppy0;
	optional_device<floppy_image_device> m_floppy1;
	required_device<c2040_fdc_t> m_fdc;
	required_memory_region m_gcr;
	required_ioport m_address;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

	// signals
	int m_miot_irq;                     // MIOT interrupt
};


// ======================> c3040_device

class c3040_device :  public c2040_device
{
public:
	// construction/destruction
	c3040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> c4040_device

class c4040_device :  public c2040_device
{
public:
	// construction/destruction
	c4040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// device type definition
extern const device_type C2040;
extern const device_type C3040;
extern const device_type C4040;



#endif
