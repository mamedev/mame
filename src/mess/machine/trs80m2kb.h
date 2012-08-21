/**********************************************************************

    Tandy Radio Shack TRS-80 Model II keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __TRS80M2_KEYBOARD__
#define __TRS80M2_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TRS80M2_KEYBOARD_TAG	"trs80m2kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TRS80M2_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(TRS80M2_KEYBOARD_TAG, TRS80M2_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define TRS80M2_KEYBOARD_INTERFACE(_name) \
	const trs80m2_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> trs80m2_keyboard_interface

struct trs80m2_keyboard_interface
{
	devcb_write_line	m_out_clock_cb;
};


// ======================> trs80m2_keyboard_device

class trs80m2_keyboard_device :  public device_t,
								 public trs80m2_keyboard_interface
{
public:
    // construction/destruction
    trs80m2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_t1_r );
	DECLARE_READ8_MEMBER( kb_p0_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "trs80m2kb"; }

private:
	devcb_resolved_write_line	m_out_clock_func;

	required_device<cpu_device> m_maincpu;

	int m_busy;
	int m_data;
	int m_clk;

	UINT8 m_y;
};


// device type definition
extern const device_type TRS80M2_KEYBOARD;



#endif
