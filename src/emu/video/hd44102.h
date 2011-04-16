/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __HD44102__
#define __HD44102__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_HD44102_ADD(_tag, _screen_tag, _sx, _sy) \
	MCFG_DEVICE_ADD(_tag, HD44102, 0) \
	hd44102_device_config::static_set_config(device, _screen_tag, _sx, _sy);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> hd44102_device_config

class hd44102_device_config :   public device_config
{
    friend class hd44102_device;

    // construction/destruction
    hd44102_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_config(device_config *device, const char *screen_tag, int sx, int sy);

private:
	const char *m_screen_tag;
	int m_sx;
	int m_sy;
};


// ======================> hd44102_device

class hd44102_device :	public device_t
{
    friend class hd44102_device_config;

    // construction/destruction
    hd44102_device(running_machine &_machine, const hd44102_device_config &_config);

public:
    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( cs2_w );

	void update_screen(bitmap_t *bitmap, const rectangle *cliprect);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
    DECLARE_READ8_MEMBER( status_r );
    DECLARE_WRITE8_MEMBER( control_w );

    DECLARE_READ8_MEMBER( data_r );
    DECLARE_WRITE8_MEMBER( data_w );

	inline void count_up_or_down();

	screen_device *m_screen;		// screen

	UINT8 m_ram[4][50];				// display memory

	UINT8 m_status;					// status register
	UINT8 m_output;					// output register

	int m_cs2;						// chip select
	int m_page;						// display start page
	int m_x;						// X address
	int m_y;						// Y address

	const hd44102_device_config &m_config;
};


// device type definition
extern const device_type HD44102;



#endif
