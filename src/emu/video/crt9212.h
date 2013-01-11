/**********************************************************************

    SMC CRT9212 Double Row Buffer (DRB) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  DIN2   1 |*    \_/     | 28  DIN3
                  DIN1   2 |             | 27  _WCLK
                  DIN0   3 |             | 26  _OE
                 DOUT7   4 |             | 25  WEN2
                 DOUT6   5 |             | 24  WEN1
                 DOUT5   6 |             | 23  GND
                 DOUT4   7 |   CRT9212   | 22  ROF
                   Vcc   8 |             | 21  WOF
                 DOUT3   9 |             | 20  REN
                 DOUT2  10 |             | 19  _CLRCNT
                 DOUT1  11 |             | 18  _TOG
                 DOUT0  12 |             | 17  _RCLK
                  DIN7  13 |             | 16  DIN4
                  DIN6  14 |_____________| 15  DIN5

**********************************************************************/

#pragma once

#ifndef __CRT9212__
#define __CRT9212__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

const int CRT9212_RAM_SIZE  = 135;



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CRT9212_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, CRT9212, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define CRT9212_INTERFACE(name) \
	const crt9212_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> crt9212_interface

struct crt9212_interface
{
	devcb_write_line        m_out_rof_cb;
	devcb_write_line        m_out_wof_cb;
	devcb_read_line         m_in_ren_cb;
	devcb_read_line         m_in_wen_cb;
	devcb_read_line         m_in_wen2_cb;
};



// ======================> crt9212_device

class crt9212_device :  public device_t,
						public crt9212_interface
{
public:
	// construction/destruction
	crt9212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( clrcnt_w );
	DECLARE_WRITE_LINE_MEMBER( tog_w );
	DECLARE_WRITE_LINE_MEMBER( rclk_w );
	DECLARE_WRITE_LINE_MEMBER( wclk_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	devcb_resolved_write_line   m_out_rof_func;
	devcb_resolved_write_line   m_out_wof_func;
	devcb_resolved_read_line    m_in_ren_func;
	devcb_resolved_read_line    m_in_wen_func;
	devcb_resolved_read_line    m_in_wen2_func;

	UINT8 m_ram[CRT9212_RAM_SIZE][2];

	UINT8 m_input;
	UINT8 m_output;

	int m_buffer;
	int m_rac;
	int m_wac;
	int m_tog;
	int m_clrcnt;
	int m_rclk;
	int m_wclk;
};


// device type definition
extern const device_type CRT9212;



#endif
