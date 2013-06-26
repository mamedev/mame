/**********************************************************************

    Fairchild DM9368 7-Segment Decoder/Driver/Latch emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    A1   1 |*    \_/     | 16  Vcc
                    A2   2 |             | 15  F
                   _LE   3 |             | 14  G
                  _RBO   4 |   DM9368    | 13  A
                  _RBI   5 |             | 12  B
                    A3   6 |             | 11  C
                    A0   7 |             | 10  D
                   GND   8 |_____________| 9   E

**********************************************************************/

#pragma once

#ifndef __DM9368__
#define __DM9368__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DM9368_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DM9368, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define DM9368_INTERFACE(name) \
	const dm9368_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> dm9368_interface

struct dm9368_interface
{
	int m_digit;

	devcb_read_line         m_in_rbi_cb;
	devcb_write_line        m_out_rbo_cb;
};



// ======================> dm9368_device

class dm9368_device :   public device_t,
						public dm9368_interface
{
public:
	// construction/destruction
	dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void a_w(UINT8 data);
	DECLARE_WRITE_LINE_MEMBER( rbi_w );
	DECLARE_READ_LINE_MEMBER( rbo_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	inline int get_rbi();
	inline void set_rbo(int state);

	devcb_resolved_read_line    m_in_rbi_func;
	devcb_resolved_write_line   m_out_rbo_func;

	int m_rbi;
	int m_rbo;
};


// device type definition
extern const device_type DM9368;



#endif
