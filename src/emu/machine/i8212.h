// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8212 8-Bit Input/Output Port emulation

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

#define MCFG_I8212_IRQ_CALLBACK(_write) \
	devcb = &i8212_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8212_DI_CALLBACK(_read) \
	devcb = &i8212_device::set_di_rd_callback(*device, DEVCB_##_read);

#define MCFG_I8212_DO_CALLBACK(_write) \
	devcb = &i8212_device::set_do_wr_callback(*device, DEVCB_##_write);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i8212_device

class i8212_device :    public device_t
{
public:
	// construction/destruction
	i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<i8212_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_di_rd_callback(device_t &device, _Object object) { return downcast<i8212_device &>(device).m_read_di.set_callback(object); }
	template<class _Object> static devcb_base &set_do_wr_callback(device_t &device, _Object object) { return downcast<i8212_device &>(device).m_write_do.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( md_w );
	DECLARE_WRITE_LINE_MEMBER( stb_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	devcb_write_line   m_write_irq;
	devcb_read8        m_read_di;
	devcb_write8       m_write_do;

	int m_md;                   // mode
	int m_stb;                  // strobe
	UINT8 m_data;               // data latch
};


// device type definition
extern const device_type I8212;



#endif
