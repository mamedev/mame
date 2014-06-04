// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    QL Internal Mouse Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __QIMI__
#define __QIMI__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define QIMI_IO_BASE            0x1bf9c
#define QIMI_IO_LEN             0x22
#define QIMI_IO_END             (QIMI_IO_BASE + QIMI_IO_LEN )



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_QIMI_EXTINT_CALLBACK(_write) \
	devcb = &qimi_t::set_exting_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qimi_t

class qimi_t :  public device_t
{
public:
	// construction/destruction
	qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_exting_wr_callback(device_t &device, _Object object) { return downcast<qimi_t &>(device).m_write_extint.set_callback(object); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_write_line m_write_extint;

	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mouseb;

	UINT8   m_mouse_int;

	emu_timer *m_mouse_timer;

	UINT8 m_ql_mouse_x;
	UINT8 m_ql_mouse_y;
};


// device type definition
extern const device_type QIMI;



#endif
