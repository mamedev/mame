/**********************************************************************

    Intel 8212 8-Bit Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  _DS1   1 |*    \_/     | 24  Vcc
                    MD   2 |             | 23  _INT
                   DI1   3 |             | 22  DI8
                   DO1   4 |             | 21  DO8
                   DI2   5 |             | 20  DI7
                   DO2   6 |    8212     | 19  DO7
                   DI3   7 |             | 18  DI6
                   DO3   8 |             | 17  DO6
                   DI4   9 |             | 16  DI5
                   DO4  10 |             | 15  DO5
                   STB  11 |             | 14  _CLR
                   GND  12 |_____________| 13  DS2

**********************************************************************/

#pragma once

#ifndef __I8212__
#define __I8212__

#include "emu.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

enum
{
	I8212_MODE_INPUT = 0,
	I8212_MODE_OUTPUT
};



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I8212_ADD(_tag, _config) \
	MCFG_DEVICE_ADD((_tag), I8212, 0)   \
	MCFG_DEVICE_CONFIG(_config)

#define I8212_INTERFACE(name) \
	const i8212_interface (name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i8212_interface

struct i8212_interface
{
	devcb_write_line    m_out_int_cb;

	devcb_read8         m_in_di_cb;
	devcb_write8        m_out_do_cb;
};



// ======================> i8212_device

class i8212_device :    public device_t, public i8212_interface
{
public:
	// construction/destruction
	i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_WRITE_LINE_MEMBER( md_w );
	DECLARE_WRITE_LINE_MEMBER( stb_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_read8        m_in_di_func;
	devcb_resolved_write8       m_out_do_func;

	int m_md;                   // mode
	int m_stb;                  // strobe
	UINT8 m_data;               // data latch
};


// device type definition
extern const device_type I8212;



#endif
