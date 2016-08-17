// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Fairchild DM9368 7-Segment Decoder/Driver/Latch emulation

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

#define MCFG_DM9368_RBO_CALLBACK(_write) \
	devcb = &dm9368_device::set_rbo_wr_callback(*device, DEVCB_##_read);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dm9368_device

class dm9368_device :   public device_t,
						public device_output_interface
{
public:
	// construction/destruction
	dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void a_w(UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( rbi_w ) { m_rbi = state; }
	DECLARE_READ_LINE_MEMBER( rbo_r ) { return m_rbo; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	devcb_write_line   m_write_rbo;

	int m_rbi;
	int m_rbo;

	static const UINT8 m_segment_data[];
};


// device type definition
extern const device_type DM9368;



#endif
