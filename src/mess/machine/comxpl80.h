/**********************************************************************

    COMX PL-80 plotter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMXPL80__
#define __COMXPL80__

#include "emu.h"
#include "cpu/m6805/m6805.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define COMXPL80_TAG	"comxpl80"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COMXPL80_ADD() \
    MCFG_DEVICE_ADD(COMXPL80_TAG, COMXPL80, 0)


#define COMXPL80_INTERFACE(_name) \
	const comxpl80_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comxpl80_interface

struct comxpl80_interface
{
	devcb_write_line	m_out_txd_cb;
	devcb_write_line	m_out_clock_cb;
	devcb_write_line	m_out_keydown_cb;
};

// ======================> comxpl80_device

class comxpl80_device :  public device_t,
                         public comxpl80_interface
{
public:
    // construction/destruction
    comxpl80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE8_MEMBER( pa_w );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_WRITE8_MEMBER( pc_w );
	DECLARE_READ8_MEMBER( pd_r );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete();

private:
	// printer state
	UINT8 m_centronics_data;	// centronics data

	// PL-80 plotter state
	UINT16 m_font_addr;			// font ROM pack address latch
	UINT8 m_x_motor_phase;		// X motor phase
	UINT8 m_y_motor_phase;		// Y motor phase
	UINT8 m_z_motor_phase;		// Z motor phase
	UINT8 m_plotter_data;		// plotter data bus
	int m_plotter_ack;			// plotter acknowledge
	int m_plotter_online;		// online LED
};


// device type definition
extern const device_type COMXPL80;



#endif
