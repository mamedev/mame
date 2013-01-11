/**********************************************************************

    Intel 8355 - 16,384-Bit ROM with I/O emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  _CE1   1 |*    \_/     | 40  Vcc
                   CE2   2 |             | 39  PB7
                   CLK   3 |             | 38  PB6
                 RESET   4 |             | 37  PB5
                  N.C.   5 |             | 36  PB4
                 READY   6 |             | 35  PB3
                 IO/_M   7 |             | 34  PB2
                  _IOR   8 |             | 33  PB1
                   _RD   9 |             | 32  PB0
                  _IOW  10 |    8355     | 31  PA7
                   ALE  11 |    8355-2   | 30  PA6
                   AD0  12 |             | 29  PA5
                   AD1  13 |             | 28  PA4
                   AD2  14 |             | 27  PA3
                   AD3  15 |             | 26  PA2
                   AD4  16 |             | 25  PA1
                   AD5  17 |             | 24  PA0
                   AD6  18 |             | 23  A10
                   AD7  19 |             | 22  A9
                   Vss  20 |_____________| 21  A8

**********************************************************************/

#pragma once

#ifndef __I8355__
#define __I8355__

#include "emu.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************




///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I8355_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), I8355, _clock)  \
	MCFG_DEVICE_CONFIG(_config)

#define I8355_INTERFACE(name) \
	const i8355_interface (name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i8355_interface

struct i8355_interface
{
	devcb_read8             m_in_pa_cb;
	devcb_write8            m_out_pa_cb;

	devcb_read8             m_in_pb_cb;
	devcb_write8            m_out_pb_cb;
};



// ======================> i8355_device

class i8355_device :    public device_t,
						public device_memory_interface,
						public i8355_interface
{
public:
	// construction/destruction
	i8355_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	DECLARE_READ8_MEMBER( memory_r );
	DECLARE_WRITE8_MEMBER( memory_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	inline UINT8 read_port(int port);
	inline void write_port(int port, UINT8 data);

private:
	devcb_resolved_read8        m_in_port_func[2];
	devcb_resolved_write8       m_out_port_func[2];

	// registers
	UINT8 m_output[2];          // output latches
	UINT8 m_ddr[2];             // DDR latches

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type I8355;



#endif
