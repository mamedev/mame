/**********************************************************************

    Intel 8255(A) Programmable Peripheral Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   PA3   1 |*    \_/     | 40  PA4
                   PA2   2 |             | 39  PA5
                   PA1   3 |             | 38  PA6
                   PA0   4 |             | 37  PA7
                   _RD   5 |             | 36  WR
                   _CS   6 |             | 35  RESET
                   GND   7 |             | 34  D0
                    A1   8 |             | 33  D1
                    A0   9 |             | 32  D2
                   PC7  10 |    8255     | 31  D3
                   PC6  11 |    8255A    | 30  D4
                   PC5  12 |             | 29  D5
                   PC4  13 |             | 28  D6
                   PC0  14 |             | 27  D7
                   PC1  15 |             | 26  Vcc
                   PC2  16 |             | 25  PB7
                   PC3  17 |             | 24  PB6
                   PB0  18 |             | 23  PB5
                   PB1  19 |             | 22  PB4
                   PB2  20 |_____________| 21  PB3

**********************************************************************/

#pragma once

#ifndef __I8255__
#define __I8255__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8255_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8255, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_I8255A_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8255A, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define I8255_INTERFACE(name) \
	const i8255_interface (name) =


#define I8255A_INTERFACE(name) \
	const i8255_interface (name) =


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i8255_interface

struct i8255_interface
{
	devcb_read8			m_in_pa_func;
	devcb_write8		m_out_pa_func;
	devcb_read8			m_in_pb_func;
	devcb_write8		m_out_pb_func;
	devcb_read8			m_in_pc_func;
	devcb_write8		m_out_pc_func;
};


// ======================> i8255_device_config

class i8255_device_config : public device_config,
                             public i8255_interface
{
    friend class i8255_device;

    // construction/destruction
    i8255_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> i8255_device

class i8255_device :  public device_t
{
    friend class i8255_device_config;

    // construction/destruction
    i8255_device(running_machine &_machine, const i8255_device_config &_config);

public:
    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

    DECLARE_READ8_MEMBER( pa_r );
	UINT8 pa_r();
    
	DECLARE_READ8_MEMBER( pb_r );
	UINT8 pb_r();

	DECLARE_WRITE_LINE_MEMBER( pc2_w );
	DECLARE_WRITE_LINE_MEMBER( pc4_w );
	DECLARE_WRITE_LINE_MEMBER( pc6_w );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
	inline void check_interrupt(int port);
	inline void set_ibf(int port, int state);
	inline void set_obf(int port, int state);
	inline void set_inte(int port, int state);
	inline void set_inte1(int state);
	inline void set_inte2(int state);
	inline void set_intr(int port, int state);
	inline int group_mode(int group);
	inline int port_mode(int port);
	inline int port_c_lower_mode();
	inline int port_c_upper_mode();

	UINT8 read_mode0(int port);
	UINT8 read_mode1(int port);
	UINT8 read_mode2();
	UINT8 read_pc();
	void write_mode0(int port, UINT8 data);
	void write_mode1(int port, UINT8 data);
	void write_mode2(UINT8 data);
	void write_pc(UINT8 data);
	void output_pc();
	void set_mode(UINT8 data);
	void set_pc_bit(int bit, int state);
	
	devcb_resolved_read8		m_in_port_func[3];
	devcb_resolved_write8		m_out_port_func[3];

	UINT8 m_control;			// mode control word
	UINT8 m_output[3];			// output latch
	UINT8 m_input[3];			// input latch

	int m_ibf[2];				// input buffer full flag
	int m_obf[2];				// output buffer full flag, negative logic
	int m_inte[2];				// interrupt enable
	int m_inte1;				// interrupt enable
	int m_inte2;				// interrupt enable
	int m_intr[2];				// interrupt

    const i8255_device_config &m_config;
};


// device type definition
extern const device_type I8255;
extern const device_type I8255A;



#endif
