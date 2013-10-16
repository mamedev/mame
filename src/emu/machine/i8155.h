// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8155/8156 - 2048-Bit Static MOS RAM with I/O Ports and Timer emulation
    8156 is the same as 8155, except that chip enable is active high instead of low

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   PC3   1 |*    \_/     | 40  Vcc
                   PC4   2 |             | 39  PC2
              TIMER IN   3 |             | 38  PC1
                 RESET   4 |             | 37  PC0
                   PC5   5 |             | 36  PB7
            _TIMER OUT   6 |             | 35  PB6
                 IO/_M   7 |             | 34  PB5
             CE or _CE   8 |             | 33  PB4
                   _RD   9 |             | 32  PB3
                   _WR  10 |    8155     | 31  PB2
                   ALE  11 |    8156     | 30  PB1
                   AD0  12 |             | 29  PB0
                   AD1  13 |             | 28  PA7
                   AD2  14 |             | 27  PA6
                   AD3  15 |             | 26  PA5
                   AD4  16 |             | 25  PA4
                   AD5  17 |             | 24  PA3
                   AD6  18 |             | 23  PA2
                   AD7  19 |             | 22  PA1
                   Vss  20 |_____________| 21  PA0

**********************************************************************/

#pragma once

#ifndef __I8155__
#define __I8155__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8155_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), I8155, _clock)  \
	MCFG_DEVICE_CONFIG(_config)


#define MCFG_I8156_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), I8156, _clock)  \
	MCFG_DEVICE_CONFIG(_config)


#define I8155_INTERFACE(name) \
	const i8155_interface (name) =


#define I8156_INTERFACE(name) \
	const i8155_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i8155_interface

struct i8155_interface
{
	devcb_read8             in_pa_cb;
	devcb_write8            out_pa_cb;

	devcb_read8             in_pb_cb;
	devcb_write8            out_pb_cb;

	devcb_read8             in_pc_cb;
	devcb_write8            out_pc_cb;

	// this gets called for each change of the TIMER OUT pin (pin 6)
	devcb_write_line        out_to_cb;
};



// ======================> i8155_device

class i8155_device :    public device_t,
						public device_memory_interface,
						public i8155_interface
{
public:
	// construction/destruction
	i8155_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	DECLARE_READ8_MEMBER( memory_r );
	DECLARE_WRITE8_MEMBER( memory_w );

	DECLARE_WRITE8_MEMBER( ale_w );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	inline UINT8 get_timer_mode();
	inline void timer_output();
	inline void pulse_timer_output();
	inline int get_port_mode(int port);
	inline UINT8 read_port(int port);
	inline void write_port(int port, UINT8 data);

	void register_w(int offset, UINT8 data);

private:
	devcb_resolved_read8        m_in_port_func[3];
	devcb_resolved_write8       m_out_port_func[3];
	devcb_resolved_write_line   m_out_to_func;

	// CPU interface
	int m_io_m;                 // I/O or memory select
	UINT8 m_ad;                 // address

	// registers
	UINT8 m_command;            // command register
	UINT8 m_status;             // status register
	UINT8 m_output[3];          // output latches

	// counter
	UINT16 m_count_length;      // count length register
	UINT16 m_counter;           // counter register
	int m_to;                   // timer output

	// timers
	emu_timer *m_timer;         // counter timer

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type I8155;
extern const device_type I8156;



#endif
